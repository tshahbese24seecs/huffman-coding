const BASE_URL = import.meta.env.PROD
  ? ''
  : 'http://localhost:8080';

async function apiFetch(path, options = {}) {
  const res = await fetch(`${BASE_URL}${path}`, {
    headers: { 'Content-Type': 'application/json' },
    ...options,
  });
  return res.json();
}

export async function analyze(text) {
  return apiFetch('/api/analyze', { method: 'POST', body: JSON.stringify({ text }) });
}

export async function compress(text) {
  return apiFetch('/api/compress', { method: 'POST', body: JSON.stringify({ text }) });
}

export async function compressFileData(data_b64, filename, mime_type) {
  return apiFetch('/api/compress', { method: 'POST', body: JSON.stringify({ data_b64, filename, mime_type }) });
}

export async function decompress(data_b64) {
  return apiFetch('/api/decompress', { method: 'POST', body: JSON.stringify({ data_b64 }) });
}
