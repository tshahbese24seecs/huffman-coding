const API_ROOT = '/api';

export async function analyze(text) {
  const res = await fetch(`${API_ROOT}/analyze`, {
    method: 'POST',
    headers: { 'Content-Type': 'application/json' },
    body: JSON.stringify({ text })
  });
  return res.json();
}

export async function compress(text) {
  const res = await fetch(`${API_ROOT}/compress`, {
    method: 'POST',
    headers: { 'Content-Type': 'application/json' },
    body: JSON.stringify({ text })
  });
  return res.json();
}

export async function decompress(data_b64) {
  const res = await fetch(`${API_ROOT}/decompress`, {
    method: 'POST',
    headers: { 'Content-Type': 'application/json' },
    body: JSON.stringify({ data_b64 })
  });
  return res.json();
}
