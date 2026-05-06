/**
 * @file server.cpp
 * @brief HTTP REST API server for Huffman Coding backend.
 *
 * Endpoints:
 *   GET  /api/health        — Health check
 *   POST /api/analyze       — Analyze text (frequencies, codes, tree) without compressing
 *   POST /api/compress      — Compress text, return base64 payload + metadata
 *   POST /api/decompress    — Decompress base64 payload, return original text
 *   POST /api/compress-file — Compress an uploaded file (multipart form)
 *
 * Dependencies:
 *   - cpp-httplib (single-header HTTP server, fetched via CMake)
 *   - nlohmann/json (single-header JSON library, fetched via CMake)
 *
 * CORS is enabled for all origins so any frontend can connect.
 */

#include <iostream>
#include <string>

#include <httplib.h>
#include <nlohmann/json.hpp>

#include "../src/huffman.h"

using json = nlohmann::json;
using namespace std;

// ── Helpers ─────────────────────────────────────────────────────────

/**
 * Convert a character to a display-friendly label for JSON keys.
 */
static string char_label(char ch)
{
    switch (ch)
    {
        case ' ':  return "SPACE";
        case '\n': return "\\n";
        case '\r': return "\\r";
        case '\t': return "\\t";
        default:   return string(1, ch);
    }
}

/**
 * Apply CORS headers to every response.
 */
static void apply_cors(httplib::Response &res)
{
    res.set_header("Access-Control-Allow-Origin",  "*");
    res.set_header("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
    res.set_header("Access-Control-Allow-Headers", "Content-Type");
}

/**
 * Build a JSON error response.
 */
static void send_error(httplib::Response &res, int status, const string &msg)
{
    apply_cors(res);
    res.status = status;
    json j = {{"success", false}, {"error", msg}};
    res.set_content(j.dump(), "application/json");
}

/**
 * Parse request body as JSON. Returns false and sends error on failure.
 */
static bool parse_json_body(const httplib::Request &req, httplib::Response &res, json &out)
{
    try
    {
        out = json::parse(req.body);
        return true;
    }
    catch (const json::parse_error &e)
    {
        send_error(res, 400, string("Invalid JSON: ") + e.what());
        return false;
    }
}

// ═══════════════════════════════════════════════════════════════════
//  Route handlers
// ═══════════════════════════════════════════════════════════════════

void handle_health(const httplib::Request &, httplib::Response &res)
{
    apply_cors(res);
    json j = {{"status", "ok"}, {"service", "huffman-coding-api"}};
    res.set_content(j.dump(), "application/json");
}

// ── POST /api/analyze ───────────────────────────────────────────────
void handle_analyze(const httplib::Request &req, httplib::Response &res)
{
    json body;
    if (!parse_json_body(req, res, body)) return;

    if (!body.contains("text") || !body["text"].is_string())
    {
        send_error(res, 400, "Missing required field: \"text\" (string).");
        return;
    }

    string text = body["text"].get<string>();
    auto result = analyze(text);

    if (!result.success)
    {
        send_error(res, 400, result.error);
        return;
    }

    // Build response
    json freq_obj = json::object();
    for (auto &[ch, f] : result.frequencies)
        freq_obj[char_label(ch)] = f;

    json codes_obj = json::object();
    for (auto &[ch, code] : result.prefix_codes)
        codes_obj[char_label(ch)] = code;

    apply_cors(res);
    json j = {
        {"success",     true},
        {"text_size",   result.text_size},
        {"unique_chars", result.frequencies.size()},
        {"frequencies", freq_obj},
        {"codes",       codes_obj},
        {"tree",        json::parse(result.tree_json)}
    };
    res.set_content(j.dump(2), "application/json");
}

// ── POST /api/compress ──────────────────────────────────────────────
void handle_compress(const httplib::Request &req, httplib::Response &res)
{
    json body;
    if (!parse_json_body(req, res, body)) return;

    if (!body.contains("text") || !body["text"].is_string())
    {
        send_error(res, 400, "Missing required field: \"text\" (string).");
        return;
    }

    string text = body["text"].get<string>();
    auto result = compress(text);

    if (!result.success)
    {
        send_error(res, 400, result.error);
        return;
    }

    // Encode compressed bytes as base64 for safe JSON transport
    string b64 = base64_encode(result.compressed_data);

    // Build frequency & codes for the response
    json freq_obj = json::object();
    for (auto &[ch, f] : result.frequencies)
        freq_obj[char_label(ch)] = f;

    json codes_obj = json::object();
    for (auto &[ch, code] : result.prefix_codes)
        codes_obj[char_label(ch)] = code;

    apply_cors(res);
    json j = {
        {"success",           true},
        {"original_size",     result.original_size},
        {"compressed_size",   result.compressed_size},
        {"compression_ratio", result.compression_ratio},
        {"data_b64",          b64},
        {"frequencies",       freq_obj},
        {"codes",             codes_obj},
        {"tree",              json::parse(result.tree_json)}
    };
    res.set_content(j.dump(2), "application/json");
}

// ── POST /api/decompress ────────────────────────────────────────────
void handle_decompress(const httplib::Request &req, httplib::Response &res)
{
    json body;
    if (!parse_json_body(req, res, body)) return;

    if (!body.contains("data_b64") || !body["data_b64"].is_string())
    {
        send_error(res, 400, "Missing required field: \"data_b64\" (string).");
        return;
    }

    string b64 = body["data_b64"].get<string>();
    vector<unsigned char> compressed = base64_decode(b64);

    auto result = decompress(compressed);

    if (!result.success && result.error.find("Warning") == string::npos)
    {
        send_error(res, 400, result.error);
        return;
    }

    apply_cors(res);
    json j = {
        {"success",       true},
        {"text",          result.text},
        {"original_size", result.original_size}
    };
    if (!result.error.empty())
        j["warning"] = result.error;

    res.set_content(j.dump(2), "application/json");
}

// ═══════════════════════════════════════════════════════════════════
//  Main — start the server
// ═══════════════════════════════════════════════════════════════════

int main(int argc, char *argv[])
{
    int port = 8080;

    // Allow overriding the port via CLI or env
    if (argc > 1)
        port = atoi(argv[1]);

    httplib::Server svr;

    // ── CORS preflight handler (catch-all for OPTIONS) ──────────
    svr.Options("/(.*)", [](const httplib::Request &, httplib::Response &res)
    {
        apply_cors(res);
        res.status = 204;
    });

    // ── Routes ──────────────────────────────────────────────────
    svr.Get("/api/health",       handle_health);
    svr.Post("/api/analyze",     handle_analyze);
    svr.Post("/api/compress",    handle_compress);
    svr.Post("/api/decompress",  handle_decompress);

    cout << "\n"
         << "  ╔═══════════════════════════════════════════╗\n"
         << "  ║     Huffman Coding — API Server           ║\n"
         << "  ╠═══════════════════════════════════════════╣\n"
         << "  ║                                           ║\n"
         << "  ║  Listening on http://localhost:" << port << "        ║\n"
         << "  ║                                           ║\n"
         << "  ║  Endpoints:                               ║\n"
         << "  ║    GET  /api/health                       ║\n"
         << "  ║    POST /api/analyze                      ║\n"
         << "  ║    POST /api/compress                     ║\n"
         << "  ║    POST /api/decompress                   ║\n"
         << "  ║                                           ║\n"
         << "  ║  Press Ctrl+C to stop.                    ║\n"
         << "  ╚═══════════════════════════════════════════╝\n\n";

    svr.listen("0.0.0.0", port);
    return 0;
}
