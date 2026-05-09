#include <iostream>
#include <fstream>
#include <string>
#include <filesystem>
#include <vector>
#include "../src/huffman.h"

using namespace std;
namespace fs = filesystem;

/**
 * @brief Automated test suite for Huffman Coding backend.
 *
 * Tests:
 *  1. Frequency calculation
 *  2. Huffman tree construction & prefix codes
 *  3. Tree serialization round-trip
 *  4. Tree → JSON output
 *  5. In-memory compress → decompress round-trip
 *  6. Base64 encode / decode round-trip
 *  7. File encode → decode round-trip
 *  8. Various string round-trips (edge cases)
 *  9. analyze() API
 */

static int tests_passed = 0;
static int tests_failed = 0;

static void ASSERT_EQ(const string &test_name, const string &expected, const string &actual)
{
    if (expected == actual)
    {
        cout << "  [PASS] " << test_name << "\n";
        tests_passed++;
    }
    else
    {
        cerr << "  [FAIL] " << test_name << "\n"
             << "         Expected: \"" << expected << "\"\n"
             << "         Got:      \"" << actual << "\"\n";
        tests_failed++;
    }
}

static void ASSERT_TRUE(const string &test_name, bool condition)
{
    if (condition)
    {
        cout << "  [PASS] " << test_name << "\n";
        tests_passed++;
    }
    else
    {
        cerr << "  [FAIL] " << test_name << "\n";
        tests_failed++;
    }
}

// ── Test 1: Frequency Calculation ───────────────────────────────────
void test_frequency()
{
    cout << "\n── Test: Frequency Calculation ──\n";

    auto freq = calculate_freq("aabbc");
    ASSERT_TRUE("freq('a') == 2", freq['a'] == 2);
    ASSERT_TRUE("freq('b') == 2", freq['b'] == 2);
    ASSERT_TRUE("freq('c') == 1", freq['c'] == 1);
    ASSERT_TRUE("3 unique chars",  freq.size() == 3);
}

// ── Test 2: Tree & Prefix Codes ─────────────────────────────────────
void test_tree_and_codes()
{
    cout << "\n── Test: Tree Construction & Prefix Codes ──\n";

    auto freq = calculate_freq("aaabbc");
    Node *root = generate_huffman_tree(freq);

    ASSERT_TRUE("Root is not null",         root != nullptr);
    ASSERT_TRUE("Root is internal node",    root->value == '\0');
    ASSERT_TRUE("Root freq == 6",           root->freq == 6);

    unordered_map<char, string> prefix;
    generate_prefix_codes(root, "", prefix);

    ASSERT_TRUE("3 prefix codes generated", prefix.size() == 3);
    for (auto &[ch, code] : prefix)
        ASSERT_TRUE(string("Non-empty code for '") + ch + "'", !code.empty());

    cout << "\n  Tree visualization:\n";
    print_tree(root, "    ");

    free_tree(root);
}

// ── Test 3: Serialization Round-Trip ────────────────────────────────
void test_serialization()
{
    cout << "\n── Test: Tree Serialization Round-Trip ──\n";

    auto freq = calculate_freq("huffman coding test");
    Node *root = generate_huffman_tree(freq);

    string tree_data;
    serialize_tree(root, tree_data);
    ASSERT_TRUE("Serialized tree is non-empty", !tree_data.empty());

    size_t pos = 0;
    Node *restored = deserialize_tree(tree_data, pos);
    ASSERT_TRUE("Deserialized tree root is not null", restored != nullptr);
    ASSERT_TRUE("Consumed all serialized data",       pos == tree_data.size());

    // Both trees must produce the same prefix codes
    unordered_map<char, string> codes_orig, codes_restored;
    generate_prefix_codes(root,     "", codes_orig);
    generate_prefix_codes(restored, "", codes_restored);

    ASSERT_TRUE("Same number of codes", codes_orig.size() == codes_restored.size());

    bool lengths_match = true;
    for (auto &[ch, code] : codes_orig)
    {
        if (codes_restored.find(ch) == codes_restored.end() ||
            codes_restored[ch].size() != code.size())
        {
            lengths_match = false;
            break;
        }
    }
    ASSERT_TRUE("Code lengths match after round-trip", lengths_match);

    free_tree(root);
    free_tree(restored);
}

// ── Test 4: Tree → JSON ────────────────────────────────────────────
void test_tree_json()
{
    cout << "\n── Test: Tree → JSON ──\n";

    auto freq = calculate_freq("abc");
    Node *root = generate_huffman_tree(freq);

    string json = tree_to_json(root);
    ASSERT_TRUE("JSON is non-empty", !json.empty());
    ASSERT_TRUE("JSON starts with {", json.front() == '{');
    ASSERT_TRUE("JSON ends with }",   json.back()  == '}');
    ASSERT_TRUE("JSON contains 'freq'", json.find("\"freq\"") != string::npos);
    ASSERT_TRUE("JSON contains 'value'", json.find("\"value\"") != string::npos);

    cout << "  JSON output: " << json << "\n";

    free_tree(root);
}

// ── Test 5: In-Memory Compress/Decompress ──────────────────────────
void test_memory_roundtrip()
{
    cout << "\n── Test: In-Memory Compress → Decompress ──\n";

    string text = "hello huffman world! this is a compression test.";

    auto comp = compress(text);
    ASSERT_TRUE("Compression succeeded",     comp.success);
    ASSERT_TRUE("Compressed data non-empty", !comp.compressed_data.empty());
    ASSERT_TRUE("Original size matches",     comp.original_size == text.size());
    ASSERT_TRUE("Has frequencies",           !comp.frequencies.empty());
    ASSERT_TRUE("Has prefix codes",          !comp.prefix_codes.empty());
    ASSERT_TRUE("Has tree JSON",             !comp.tree_json.empty());

    cout << "  Original: " << comp.original_size << " bytes\n"
         << "  Compressed: " << comp.compressed_size << " bytes\n"
         << "  Ratio: " << comp.compression_ratio << "\n";

    auto decomp = decompress(comp.compressed_data);
    ASSERT_TRUE("Decompression succeeded", decomp.success);
    ASSERT_EQ("Round-trip text matches", text, decomp.text);
}

// ── Test 6: Base64 Round-Trip ──────────────────────────────────────
void test_base64()
{
    cout << "\n── Test: Base64 Encode → Decode ──\n";

    // Test with known values
    vector<unsigned char> data = {'H', 'e', 'l', 'l', 'o'};
    string encoded = base64_encode(data);
    ASSERT_EQ("Base64 of 'Hello'", "SGVsbG8=", encoded);

    vector<unsigned char> decoded = base64_decode(encoded);
    ASSERT_TRUE("Decoded size matches", decoded.size() == data.size());

    bool match = true;
    for (size_t i = 0; i < data.size(); i++)
        if (data[i] != decoded[i]) { match = false; break; }
    ASSERT_TRUE("Decoded bytes match original", match);

    // Test with compression output
    auto comp = compress("base64 test data");
    string b64 = base64_encode(comp.compressed_data);
    vector<unsigned char> roundtrip = base64_decode(b64);
    ASSERT_TRUE("B64 round-trip preserves size", roundtrip.size() == comp.compressed_data.size());

    auto decomp = decompress(roundtrip);
    ASSERT_EQ("B64 transport round-trip", "base64 test data", decomp.text);
}

// ── Test 7: File Encode/Decode ──────────────────────────────────────
void test_file_roundtrip()
{
    cout << "\n── Test: File Encode → Decode Round-Trip ──\n";

    fs::path data_dir  = fs::weakly_canonical(fs::path("../data"));
    fs::path input     = data_dir / "test.txt";
    fs::path compressed = data_dir / "test.huff";
    fs::path decoded   = data_dir / "test.decoded.txt";

    if (!fs::exists(input))
    {
        cerr << "  [SKIP] " << input << " does not exist.\n";
        return;
    }

    ifstream fin(input, ios::binary);
    string original((istreambuf_iterator<char>(fin)), istreambuf_iterator<char>());
    fin.close();

    bool enc = encode_file(input.string(), compressed.string());
    ASSERT_TRUE("Encode succeeded", enc);
    ASSERT_TRUE("Compressed file exists", fs::exists(compressed));

    bool dec = decode_file(compressed.string(), decoded.string());
    ASSERT_TRUE("Decode succeeded", dec);

    ifstream dfin(decoded, ios::binary);
    string result((istreambuf_iterator<char>(dfin)), istreambuf_iterator<char>());
    dfin.close();

    ASSERT_EQ("Decoded matches original", original, result);

    fs::remove(compressed);
    fs::remove(decoded);
}

// ── Test 8: Various String Round-Trips ──────────────────────────────
void test_string_roundtrips()
{
    cout << "\n── Test: Various String Round-Trips ──\n";

    vector<string> test_cases = {
        "hello world",
        "aaaaaaa",
        "abcdefghijklmnopqrstuvwxyz",
        "The quick brown fox jumps over the lazy dog",
        "111222333",
        string(1, 'Z'),
        "  spaces  everywhere  ",
        "Line1\nLine2\nLine3",
        string("A\0B\0C", 5),
    };

    for (size_t i = 0; i < test_cases.size(); i++)
    {
        string label = "Round-trip #" + to_string(i + 1);
        auto comp = compress(test_cases[i]);

        if (!comp.success)
        {
            cerr << "  [FAIL] " << label << " — compress failed: " << comp.error << "\n";
            tests_failed++;
            continue;
        }

        auto decomp = decompress(comp.compressed_data);

        ASSERT_EQ(label + " (\"" + test_cases[i].substr(0, 30) + "...\")",
                  test_cases[i], decomp.text);
    }
}

// ── Test 9: analyze() API ───────────────────────────────────────────
void test_analyze()
{
    cout << "\n── Test: analyze() API ──\n";

    auto r = analyze("aabbcc");
    ASSERT_TRUE("Analysis succeeded", r.success);
    ASSERT_TRUE("Text size == 6",     r.text_size == 6);
    ASSERT_TRUE("3 unique chars",     r.frequencies.size() == 3);
    ASSERT_TRUE("3 prefix codes",     r.prefix_codes.size() == 3);
    ASSERT_TRUE("Tree JSON non-empty", !r.tree_json.empty());

    // Error case
    auto empty = analyze("");
    ASSERT_TRUE("Empty text fails",   !empty.success);
}

// ═══════════════════════════════════════════════════════════════════
int main()
{
    cout << "╔═══════════════════════════════════════════╗\n"
         << "║     Huffman Coding — Test Suite           ║\n"
         << "╚═══════════════════════════════════════════╝\n";

    test_frequency();
    test_tree_and_codes();
    test_serialization();
    test_tree_json();
    test_memory_roundtrip();
    test_base64();
    test_file_roundtrip();
    test_string_roundtrips();
    test_analyze();

    cout << "\n══════════════════════════════════════════════\n"
         << "  Results: " << tests_passed << " passed, "
         << tests_failed << " failed\n"
         << "══════════════════════════════════════════════\n";

    return tests_failed > 0 ? 1 : 0;
}
