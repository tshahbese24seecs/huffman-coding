FROM gcc:13 AS builder
RUN apt-get update && apt-get install -y cmake git && rm -rf /var/lib/apt/lists/*
WORKDIR /app
COPY . .
RUN cmake -B build -S . -DCMAKE_BUILD_TYPE=Release \
 && cmake --build build --target huffman_server --config Release -j$(nproc)

FROM gcc:13
WORKDIR /app
COPY --from=builder /app/build/huffman_server ./huffman_server
COPY --from=builder /app/frontend/dist ./frontend/dist
EXPOSE 8080
CMD ["./huffman_server", "8080"]