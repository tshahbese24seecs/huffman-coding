#include "huffman.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <queue>
#include <vector>
#include <cstdint>
#include <algorithm>

using namespace std;

static bool is_leaf(Node *node)
{
    return node && node->left == nullptr && node->right == nullptr;
}

// ═══════════════════════════════════════════════════════════════════
//  Frequency analysis
// ═══════════════════════════════════════════════════════════════════

unordered_map<char, int> calculate_freq(const string &text)
{
    unordered_map<char, int> frequency;
    for (char ch : text)
        frequency[ch]++;
    return frequency;
}

// ═══════════════════════════════════════════════════════════════════
//  Huffman tree construction
// ═══════════════════════════════════════════════════════════════════

Node *generate_huffman_tree(const unordered_map<char, int> &freq)
{
    if (freq.empty())
        return nullptr;

    priority_queue<Node *, vector<Node *>, compare_nodes> min_heap;

    for (auto &[ch, f] : freq)
        min_heap.push(new Node(ch, f));

    // Edge case: only one unique character
    if (min_heap.size() == 1)
    {
        Node *only = min_heap.top();
        min_heap.pop();
        return new Node('\0', only->freq, only, nullptr);
    }

    while (min_heap.size() > 1)
    {
        Node *left  = min_heap.top(); min_heap.pop();
        Node *right = min_heap.top(); min_heap.pop();

        Node *parent = new Node('\0', left->freq + right->freq, left, right);
        min_heap.push(parent);
    }

    return min_heap.top();
}

// ═══════════════════════════════════════════════════════════════════
//  Tree visualization (console — kept for debugging/tests)
// ═══════════════════════════════════════════════════════════════════

void print_tree(Node *root, const string &indent)
{
    if (root == nullptr)
        return;

    if (is_leaf(root))
    {
        cout << indent << "+-- '";
        if (root->value == ' ')       cout << "SPACE";
        else if (root->value == '\0') cout << "\\0";
        else if (root->value == '\n') cout << "\\n";
        else if (root->value == '\t') cout << "\\t";
        else if (root->value == '\r') cout << "\\r";
        else                          cout << root->value;
        cout << "': " << root->freq << "\n";
        return;
    }

    cout << indent << "|-- [Internal]: " << root->freq << "\n";
    print_tree(root->left,  indent + "|   ");
    print_tree(root->right, indent + "    ");
}

// ═══════════════════════════════════════════════════════════════════
//  Tree → JSON  (for frontend rendering)
//
//  Output format (nested):
//  {
//    "value": null | "a",
//    "freq": 5,
//    "left":  { ... } | null,
//    "right": { ... } | null
//  }
// ═══════════════════════════════════════════════════════════════════

static string escape_json_char(char ch)
{
    switch (ch)
    {
        case '"':  return "\\\"";
        case '\\': return "\\\\";
        case '\n': return "\\n";
        case '\r': return "\\r";
        case '\t': return "\\t";
        case '\b': return "\\b";
        case '\f': return "\\f";
        default:
            if (static_cast<unsigned char>(ch) < 0x20 ||
                static_cast<unsigned char>(ch) >= 0x7F)
            {
                char buf[8];
                snprintf(buf, sizeof(buf), "\\u%04x", static_cast<unsigned char>(ch));
                return string(buf);
            }
            return string(1, ch);
    }
}

string tree_to_json(Node *root)
{
    if (!root)
        return "null";

    string json = "{";

    // value
    if (is_leaf(root))
        json += "\"value\":\"" + escape_json_char(root->value) + "\",";
    else
        json += "\"value\":null,";

    // freq
    json += "\"freq\":" + to_string(root->freq) + ",";

    // children
    json += "\"left\":" + tree_to_json(root->left) + ",";
    json += "\"right\":" + tree_to_json(root->right);

    json += "}";
    return json;
}

// ═══════════════════════════════════════════════════════════════════
//  Free tree memory
// ═══════════════════════════════════════════════════════════════════

void free_tree(Node *root)
{
    if (!root) return;
    free_tree(root->left);
    free_tree(root->right);
    delete root;
}

// ═══════════════════════════════════════════════════════════════════
//  Prefix-code generation
// ═══════════════════════════════════════════════════════════════════

void generate_prefix_codes(Node *root, const string &curr_code,
                           unordered_map<char, string> &prefix)
{
    if (!root)
        return;

    if (is_leaf(root))
    {
        prefix[root->value] = curr_code.empty() ? "0" : curr_code;
        return;
    }

    generate_prefix_codes(root->left,  curr_code + "0", prefix);
    generate_prefix_codes(root->right, curr_code + "1", prefix);
}

// ═══════════════════════════════════════════════════════════════════
//  Tree serialization  (binary-safe)
//
//  Format (written as raw bytes):
//    Leaf     →  '1' followed by the char byte
//    Internal →  '0' followed by left-subtree then right-subtree
// ═══════════════════════════════════════════════════════════════════

void serialize_tree(Node *root, string &result)
{
    if (!root)
        return;

    if (is_leaf(root))
    {
        result += '1';
        result += root->value;
    }
    else
    {
        result += '0';
        serialize_tree(root->left, result);
        serialize_tree(root->right, result);
    }
}

Node *deserialize_tree(const string &data, size_t &pos)
{
    if (pos >= data.size())
        return nullptr;

    char marker = data[pos++];

    if (marker == '1')
    {
        char ch = data[pos++];
        return new Node(ch, 0);
    }
    else
    {
        Node *left  = deserialize_tree(data, pos);
        Node *right = deserialize_tree(data, pos);
        return new Node('\0', 0, left, right);
    }
}

// ═══════════════════════════════════════════════════════════════════
//  Base64 encode / decode
// ═══════════════════════════════════════════════════════════════════

static const char B64_TABLE[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

string base64_encode(const vector<unsigned char> &data)
{
    string out;
    out.reserve(((data.size() + 2) / 3) * 4);

    for (size_t i = 0; i < data.size(); i += 3)
    {
        uint32_t octet_a = data[i];
        uint32_t octet_b = (i + 1 < data.size()) ? data[i + 1] : 0;
        uint32_t octet_c = (i + 2 < data.size()) ? data[i + 2] : 0;

        uint32_t triple = (octet_a << 16) | (octet_b << 8) | octet_c;

        out += B64_TABLE[(triple >> 18) & 0x3F];
        out += B64_TABLE[(triple >> 12) & 0x3F];
        out += (i + 1 < data.size()) ? B64_TABLE[(triple >> 6) & 0x3F] : '=';
        out += (i + 2 < data.size()) ? B64_TABLE[triple & 0x3F] : '=';
    }
    return out;
}

vector<unsigned char> base64_decode(const string &encoded)
{
    // Build reverse lookup
    static int table[256] = {};
    static bool table_built = false;
    if (!table_built)
    {
        fill(begin(table), end(table), -1);
        for (int i = 0; i < 64; i++)
            table[static_cast<unsigned char>(B64_TABLE[i])] = i;
        table_built = true;
    }

    vector<unsigned char> out;
    out.reserve(encoded.size() * 3 / 4);

    uint32_t buf = 0;
    int bits_collected = 0;

    for (char c : encoded)
    {
        if (c == '=' || c == '\n' || c == '\r')
            continue;
        int val = table[static_cast<unsigned char>(c)];
        if (val == -1)
            continue;

        buf = (buf << 6) | val;
        bits_collected += 6;

        if (bits_collected >= 8)
        {
            bits_collected -= 8;
            out.push_back(static_cast<unsigned char>((buf >> bits_collected) & 0xFF));
        }
    }
    return out;
}

// ═══════════════════════════════════════════════════════════════════
//  Bit-level I/O helpers  (internal)
// ═══════════════════════════════════════════════════════════════════

struct MemBitWriter
{
    vector<unsigned char> &out;
    unsigned char buffer = 0;
    int count = 0;

    explicit MemBitWriter(vector<unsigned char> &dest) : out(dest) {}

    void write_bit(int bit)
    {
        buffer = (buffer << 1) | (bit & 1);
        if (++count == 8)
        {
            out.push_back(buffer);
            buffer = 0;
            count = 0;
        }
    }

    void write_string(const string &bits)
    {
        for (char c : bits)
            write_bit(c - '0');
    }

    int flush()
    {
        int padding = 0;
        if (count > 0)
        {
            padding = 8 - count;
            buffer <<= padding;
            out.push_back(buffer);
            buffer = 0;
            count = 0;
        }
        return padding;
    }
};

struct FileBitWriter
{
    ofstream &out;
    unsigned char buffer = 0;
    int count = 0;

    explicit FileBitWriter(ofstream &os) : out(os) {}

    void write_bit(int bit)
    {
        buffer = (buffer << 1) | (bit & 1);
        if (++count == 8)
        {
            out.put(static_cast<char>(buffer));
            buffer = 0;
            count = 0;
        }
    }

    void write_string(const string &bits)
    {
        for (char c : bits)
            write_bit(c - '0');
    }

    int flush()
    {
        int padding = 0;
        if (count > 0)
        {
            padding = 8 - count;
            buffer <<= padding;
            out.put(static_cast<char>(buffer));
            buffer = 0;
            count = 0;
        }
        return padding;
    }
};

// ═══════════════════════════════════════════════════════════════════
//  Little-endian helpers  (for binary header)
// ═══════════════════════════════════════════════════════════════════

static void push_u16(vector<unsigned char> &v, uint16_t val)
{
    v.push_back(static_cast<unsigned char>(val & 0xFF));
    v.push_back(static_cast<unsigned char>((val >> 8) & 0xFF));
}

static void push_u32(vector<unsigned char> &v, uint32_t val)
{
    v.push_back(static_cast<unsigned char>(val & 0xFF));
    v.push_back(static_cast<unsigned char>((val >> 8)  & 0xFF));
    v.push_back(static_cast<unsigned char>((val >> 16) & 0xFF));
    v.push_back(static_cast<unsigned char>((val >> 24) & 0xFF));
}

static uint16_t pop_u16(const unsigned char *p)
{
    return static_cast<uint16_t>(p[0]) | (static_cast<uint16_t>(p[1]) << 8);
}

static uint32_t pop_u32(const unsigned char *p)
{
    return static_cast<uint32_t>(p[0])
         | (static_cast<uint32_t>(p[1]) << 8)
         | (static_cast<uint32_t>(p[2]) << 16)
         | (static_cast<uint32_t>(p[3]) << 24);
}

// ═══════════════════════════════════════════════════════════════════
//  analyze()  — frequency + codes + tree JSON, no compression
// ═══════════════════════════════════════════════════════════════════

AnalysisResult analyze(const string &text)
{
    AnalysisResult r;

    if (text.empty())
    {
        r.error = "Input text is empty.";
        return r;
    }

    r.text_size    = text.size();
    r.frequencies  = calculate_freq(text);

    Node *root = generate_huffman_tree(r.frequencies);
    generate_prefix_codes(root, "", r.prefix_codes);
    r.tree_json = tree_to_json(root);

    free_tree(root);

    r.success = true;
    return r;
}

// ═══════════════════════════════════════════════════════════════════
//  compress()  — in-memory, returns bytes + metadata
//
//  Compressed byte layout (same as file format):
//  ┌──────────────────────────────────────────────────────────┐
//  │  [4 bytes] magic number   "HUFF"                        │
//  │  [4 bytes] original text size  (uint32_t LE)            │
//  │  [1 byte ] padding bits in last encoded byte            │
//  │  [2 bytes] serialized tree length  (uint16_t LE)        │
//  │  [N bytes] serialized tree data                         │
//  │  [M bytes] encoded bit-stream                           │
//  └──────────────────────────────────────────────────────────┘
// ═══════════════════════════════════════════════════════════════════

CompressionResult compress(const string &text)
{
    CompressionResult r;

    if (text.empty())
    {
        r.error = "Input text is empty.";
        return r;
    }

    // ── Build tree & codes ──────────────────────────────────────
    r.frequencies  = calculate_freq(text);
    r.original_size = text.size();

    Node *root = generate_huffman_tree(r.frequencies);
    generate_prefix_codes(root, "", r.prefix_codes);
    r.tree_json = tree_to_json(root);

    // ── Serialize tree ──────────────────────────────────────────
    string tree_data;
    serialize_tree(root, tree_data);
    free_tree(root);

    // ── Encode text bits ────────────────────────────────────────
    string encoded_bits;
    encoded_bits.reserve(text.size() * 4);
    for (char ch : text)
        encoded_bits += r.prefix_codes[ch];

    // ── Assemble compressed buffer ──────────────────────────────
    vector<unsigned char> &out = r.compressed_data;
    out.reserve(11 + tree_data.size() + encoded_bits.size() / 8 + 1);

    // Magic
    out.push_back('H'); out.push_back('U');
    out.push_back('F'); out.push_back('F');

    // Original size
    push_u32(out, static_cast<uint32_t>(text.size()));

    // Padding placeholder (index 8)
    size_t padding_idx = out.size();
    out.push_back(0);

    // Tree
    push_u16(out, static_cast<uint16_t>(tree_data.size()));
    for (char c : tree_data)
        out.push_back(static_cast<unsigned char>(c));

    // Bit-stream
    MemBitWriter bw(out);
    bw.write_string(encoded_bits);
    int padding = bw.flush();

    // Write actual padding
    out[padding_idx] = static_cast<unsigned char>(padding);

    r.compressed_size = out.size();
    r.compression_ratio = (r.original_size > 0)
        ? static_cast<double>(r.compressed_size) / r.original_size
        : 0.0;
    r.success = true;
    return r;
}

// ═══════════════════════════════════════════════════════════════════
//  decompress()  — in-memory, returns original text
// ═══════════════════════════════════════════════════════════════════

DecompressionResult decompress(const vector<unsigned char> &data)
{
    DecompressionResult r;

    // ── Validate minimum header size ────────────────────────────
    if (data.size() < 11)
    {
        r.error = "Data too short to contain a valid header.";
        return r;
    }

    // ── Magic ───────────────────────────────────────────────────
    if (data[0] != 'H' || data[1] != 'U' || data[2] != 'F' || data[3] != 'F')
    {
        r.error = "Invalid format (expected HUFF magic).";
        return r;
    }

    // ── Header ──────────────────────────────────────────────────
    uint32_t original_size = pop_u32(&data[4]);
    uint8_t  padding       = data[8];
    uint16_t tree_len      = pop_u16(&data[9]);

    size_t header_end = 11 + tree_len;
    if (data.size() < header_end)
    {
        r.error = "Data truncated — tree data incomplete.";
        return r;
    }

    // ── Tree ────────────────────────────────────────────────────
    string tree_data(data.begin() + 11, data.begin() + header_end);
    size_t pos = 0;
    Node *root = deserialize_tree(tree_data, pos);

    if (!root)
    {
        r.error = "Failed to deserialize Huffman tree.";
        return r;
    }

    // ── Decode bit-stream ───────────────────────────────────────
    size_t encoded_start = header_end;
    size_t encoded_len   = data.size() - encoded_start;
    size_t total_bits    = encoded_len * 8 - padding;

    string decoded;
    decoded.reserve(original_size);

    size_t bit_index = 0;
    Node *current = root;

    while (decoded.size() < original_size && bit_index < total_bits)
    {
        size_t byte_idx = encoded_start + bit_index / 8;
        int    bit_pos  = 7 - (bit_index % 8);
        int    bit      = (data[byte_idx] >> bit_pos) & 1;
        bit_index++;

        current = (bit == 0) ? current->left : current->right;

        if (is_leaf(current))
        {
            decoded += current->value;
            current = root;
        }
    }

    free_tree(root);

    r.text = decoded;
    r.original_size = decoded.size();
    r.success = true;

    if (decoded.size() != original_size)
        r.error = "Warning: decoded size mismatch.";

    return r;
}

// ═══════════════════════════════════════════════════════════════════
//  File-based encode / decode  (thin wrappers over in-memory API)
// ═══════════════════════════════════════════════════════════════════

bool encode_file(const string &input_path, const string &output_path)
{
    ifstream fin(input_path, ios::binary);
    if (!fin.is_open())
        return false;

    string text((istreambuf_iterator<char>(fin)), istreambuf_iterator<char>());
    fin.close();

    auto result = compress(text);
    if (!result.success)
        return false;

    ofstream fout(output_path, ios::binary);
    if (!fout.is_open())
        return false;

    fout.write(reinterpret_cast<const char *>(result.compressed_data.data()),
               result.compressed_data.size());
    fout.close();
    return true;
}

bool decode_file(const string &input_path, const string &output_path)
{
    ifstream fin(input_path, ios::binary);
    if (!fin.is_open())
        return false;

    vector<unsigned char> data((istreambuf_iterator<char>(fin)),
                                istreambuf_iterator<char>());
    fin.close();

    auto result = decompress(data);
    if (!result.success && result.error.find("Warning") == string::npos)
        return false;

    ofstream fout(output_path, ios::binary);
    if (!fout.is_open())
        return false;

    fout.write(result.text.data(), result.text.size());
    fout.close();
    return true;
}
