Huffman Frontend (vanilla JS + Vite)

Quick start:

1. Install dependencies

```bash
cd frontend
npm install
```

2. Run dev server (Vite)

```bash
npm run dev
```

Vite config proxies `/api` to `http://localhost:8080` by default. Run the C++ backend (`./build/huffman_server`) and open the frontend dev URL.

Build for production:

```bash
npm run build
```

Serve `dist/` with any static server or integrate with your production stack.
