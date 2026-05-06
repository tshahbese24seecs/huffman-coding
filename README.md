# Huffman Coding — Backend API

A C++17 backend that implements **Huffman Coding** compression/decompression, exposed as a REST API for frontend consumption.

## Architecture

```
Frontend (any)  ──HTTP──▶  C++ API Server  ──▶  Huffman Core Library
                            (cpp-httplib)         (pure C++ — no I/O)
```

The core library is completely decoupled from I/O — all functions accept and return structured data. The API server is a thin HTTP wrapper that serializes results to JSON.

## Project Structure

```
Project/
├── app/
│   └── main.cpp           # HTTP REST API server
├── src/
│   ├── node.h             # Huffman tree node definition
│   ├── huffman.h           # API header (result types + function declarations)
│   └── huffman.cpp         # Core implementation (in-memory, no side-effects)
├── test/
│   └── test.cpp            # Automated test suite (9 test groups)
├── data/
│   └── test.txt            # Sample input file
├── CMakeLists.txt          # Build system (auto-fetches dependencies)
└── README.md
```

## API Endpoints

All endpoints return JSON. CORS is enabled for all origins.

### `GET /api/health`

Health check.

```json
{ "status": "ok", "service": "huffman-coding-api" }
```

### `POST /api/analyze`

Analyze text without compressing. Returns frequency table, prefix codes, and the Huffman tree structure (for visualization).

**Request:**
```json
{ "text": "hello world" }
```

**Response:**
```json
{
  "success": true,
  "text_size": 11,
  "unique_chars": 8,
  "frequencies": { "h": 1, "e": 1, "l": 3, "o": 2, "SPACE": 1, "w": 1, "r": 1, "d": 1 },
  "codes": { "l": "00", "o": "01", "h": "100", ... },
  "tree": { "value": null, "freq": 11, "left": { ... }, "right": { ... } }
}
```

### `POST /api/compress`

Compress text and return the result as base64-encoded data with full metadata.

**Request:**
```json
{ "text": "hello world" }
```

**Response:**
```json
{
  "success": true,
  "original_size": 11,
  "compressed_size": 23,
  "compression_ratio": 0.476,
  "data_b64": "SFVGR...",
  "frequencies": { ... },
  "codes": { ... },
  "tree": { ... }
}
```

### `POST /api/decompress`

Decompress a base64-encoded payload back to original text.

**Request:**
```json
{ "data_b64": "SFVGR..." }
```

**Response:**
```json
{
  "success": true,
  "text": "hello world",
  "original_size": 11
}
```

## Build & Run

### Prerequisites

- **CMake** 3.14+
- **C++17 compiler** (g++, clang++, or MSVC)
- **Internet connection** on first build (CMake fetches dependencies automatically)

### Build

```bash
# Configure
cmake -B build -S .

# Build
cmake --build build

# Or in one line
cmake -B build -S . && cmake --build build
```

### Run the API Server

```bash
# Default port 8080
./build/huffman_server

# Custom port
./build/huffman_server 3000
```

### Run Tests

```bash
./build/huffman_tests
```

## Dependencies

All dependencies are fetched automatically via CMake `FetchContent` — no manual installation needed.

| Dependency | Purpose | Version |
|---|---|---|
| [cpp-httplib](https://github.com/yhirose/cpp-httplib) | HTTP server | v0.18.3 |
| [nlohmann/json](https://github.com/nlohmann/json) | JSON serialization | v3.11.3 |

## Core Library API

The Huffman library can also be used directly (without the HTTP server):

```cpp
#include "huffman.h"

// Analyze text
AnalysisResult analysis = analyze("hello world");

// Compress
CompressionResult comp = compress("hello world");
std::string b64 = base64_encode(comp.compressed_data);

// Decompress
auto raw = base64_decode(b64);
DecompressionResult decomp = decompress(raw);
std::string original = decomp.text;  // "hello world"
```
