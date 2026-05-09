cmake_minimum_required(VERSION 3.14)
project(HuffmanCoding LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ═══════════════════════════════════════════════════════════════════
#  Fetch external dependencies
# ═══════════════════════════════════════════════════════════════════

include(FetchContent)

# cpp-httplib — single-header HTTP server
FetchContent_Declare(
    httplib
    GIT_REPOSITORY https://github.com/yhirose/cpp-httplib.git
    GIT_TAG        v0.18.3
)

# nlohmann/json — single-header JSON library
FetchContent_Declare(
    nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG        v3.11.3
)

FetchContent_MakeAvailable(httplib nlohmann_json)

# ═══════════════════════════════════════════════════════════════════
#  Huffman core library (static)
# ═══════════════════════════════════════════════════════════════════

add_library(huffman_core STATIC
    src/huffman.cpp
    src/huffman.h
    src/node.h
)

target_include_directories(huffman_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)

# ═══════════════════════════════════════════════════════════════════
#  API Server executable
# ═══════════════════════════════════════════════════════════════════

add_executable(huffman_server app/main.cpp)

target_link_libraries(huffman_server PRIVATE
    huffman_core
    httplib::httplib
    nlohmann_json::nlohmann_json
)

# ═══════════════════════════════════════════════════════════════════
#  Test executable
# ═══════════════════════════════════════════════════════════════════

add_executable(huffman_tests test/test.cpp)

target_link_libraries(huffman_tests PRIVATE huffman_core)

# Windows: link ws2_32 for httplib sockets
if(WIN32)
    target_link_libraries(huffman_server PRIVATE ws2_32)
endif()
