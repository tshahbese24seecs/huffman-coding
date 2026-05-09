const API_ROOT = '/api';

async function requestJson(path, payload) {
  let res;

  try {
    res = await fetch(`${API_ROOT}${path}`, {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify(payload)
    });
  } catch {
    return {
      success: false,
      error: 'Could not reach the Huffman API. Make sure the C++ server is running on port 8080.'
    };
  }

  const contentType = res.headers.get('content-type') || '';
  if (!contentType.includes('application/json')) {
    return {
      success: false,
      error: `Unexpected server response (${res.status}).`
    };
  }

  let data;
  try {
    data = await res.json();
  } catch {
    return {
      success: false,
      error: `Could not parse server response (${res.status}).`
    };
  }

  if (!res.ok && !data.error) {
    return {
      success: false,
      error: `Request failed with status ${res.status}.`
    };
  }

  return data;
}

export async function analyze(text) {
  return requestJson('/analyze', { text });
}

export async function compress(text) {
  return requestJson('/compress', { text });
}

export async function compressFileData(data_b64, filename, mime_type) {
  return requestJson('/compress', { data_b64, filename, mime_type });
}

export async function decompress(data_b64) {
  return requestJson('/decompress', { data_b64 });
}
