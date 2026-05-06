#ifndef HUFFMAN_H
#define HUFFMAN_H

#include <string>
#include <vector>
#include <unordered_map>
#include <cstdint>
#include "node.h"

/**
 * @brief Core Huffman coding library.
 *
 * Designed as a pure backend library with no stdout/stderr side-effects.
 * All functions return structured results suitable for API consumption.
 *
 * Provides:
 *   - Frequency analysis
 *   - Huffman tree construction / destruction
 *   - Prefix-code generation
 *   - Tree serialization / deserialization
 *   - In-memory compression / decompression  (for API use)
 *   - File-based compression / decompression (for CLI use)
 *   - Tree → JSON serialization              (for frontend visualization)
 */

// ── Result types ────────────────────────────────────────────────────

/**
 * @brief Represents one entry in the prefix-code table.
 */
struct CodeEntry
{
    char character;
    int  frequency;
    std::string code;
};

/**
 * @brief Full result of an analysis or compression operation.
 *
 * Returned by `compress()` — contains everything a frontend needs:
 * frequencies, prefix codes, tree JSON, compressed bytes, and stats.
 */
struct CompressionResult
{
    bool success = false;
    std::string error;

    // Analysis data
    std::unordered_map<char, int>         frequencies;
    std::unordered_map<char, std::string> prefix_codes;
    std::string                           tree_json;

    // Compressed payload
    std::vector<unsigned char> compressed_data;

    // Stats
    size_t original_size   = 0;
    size_t compressed_size = 0;
    double compression_ratio = 0.0;
};

/**
 * @brief Result of a decompression operation.
 */
struct DecompressionResult
{
    bool success = false;
    std::string error;
    std::string text;
    size_t original_size = 0;
};

/**
 * @brief Result of a pure analysis (no compression).
 */
struct AnalysisResult
{
    bool success = false;
    std::string error;

    std::unordered_map<char, int>         frequencies;
    std::unordered_map<char, std::string> prefix_codes;
    std::string                           tree_json;
    size_t text_size = 0;
};

// ── Frequency analysis ──────────────────────────────────────────────
std::unordered_map<char, int> calculate_freq(const std::string &text);

// ── Tree operations ─────────────────────────────────────────────────
Node *generate_huffman_tree(const std::unordered_map<char, int> &freq);
void  print_tree(Node *root, const std::string &indent = "");
void  free_tree(Node *root);
std::string tree_to_json(Node *root);

// ── Prefix-code table ───────────────────────────────────────────────
void generate_prefix_codes(Node *root, const std::string &curr_code,
                           std::unordered_map<char, std::string> &prefix);

// ── Serialization ───────────────────────────────────────────────────
void  serialize_tree(Node *root, std::string &result);
Node *deserialize_tree(const std::string &data, size_t &pos);

// ── In-memory API ───────────────────────────────────────────────────
AnalysisResult      analyze(const std::string &text);
CompressionResult   compress(const std::string &text);
DecompressionResult decompress(const std::vector<unsigned char> &data);

// ── File-based API ──────────────────────────────────────────────────
bool encode_file(const std::string &input_path, const std::string &output_path);
bool decode_file(const std::string &input_path, const std::string &output_path);

// ── Utilities ───────────────────────────────────────────────────────
std::string              base64_encode(const std::vector<unsigned char> &data);
std::vector<unsigned char> base64_decode(const std::string &encoded);

#endif // HUFFMAN_H
