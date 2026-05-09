export default function createFileControls(onLoadTextFile, onLoadCompressedFile, onFileError, onCompressFile) {
  const wrap = document.createElement('div');
  wrap.className = 'file-controls';

  const textGroup = document.createElement('div');
  textGroup.className = 'file-group';

  const textLabel = document.createElement('label');
  textLabel.textContent = 'Load a text file';
  textGroup.appendChild(textLabel);

  const textInput = document.createElement('input');
  textInput.type = 'file';
  textInput.accept = '.txt,text/plain';
  textInput.onchange = (e) => {
    const f = e.target.files[0];
    if (!f) return;
    const reader = new FileReader();
    reader.onload = () => onLoadTextFile(String(reader.result));
    reader.readAsText(f);
  };
  textGroup.appendChild(textInput);

  const anyFileGroup = document.createElement('div');
  anyFileGroup.className = 'file-group';

  const anyFileLabel = document.createElement('label');
  anyFileLabel.textContent = 'Compress any file';
  anyFileGroup.appendChild(anyFileLabel);

  const anyFileInput = document.createElement('input');
  anyFileInput.type = 'file';
  anyFileInput.onchange = async (e) => {
    const f = e.target.files[0];
    if (!f) return;

    try {
      const buffer = await f.arrayBuffer();
      onCompressFile({
        name: f.name,
        type: f.type,
        size: f.size,
        data_b64: arrayBufferToBase64(buffer)
      });
    } catch (err) {
      onFileError(err.message || 'Could not read the selected file.');
    }
  };
  anyFileGroup.appendChild(anyFileInput);

  const huffGroup = document.createElement('div');
  huffGroup.className = 'file-group';

  const huffLabel = document.createElement('label');
  huffLabel.textContent = 'Decompress a compressed file';
  huffGroup.appendChild(huffLabel);

  const huffInput = document.createElement('input');
  huffInput.type = 'file';
  huffInput.accept = '.json,.huff,application/json,application/octet-stream';
  huffInput.onchange = async (e) => {
    const f = e.target.files[0];
    if (!f) return;

    try {
      if (f.name.toLowerCase().endsWith('.json')) {
        const text = await f.text();
        const parsed = JSON.parse(text);
        if (!parsed.data_b64 || parsed.format !== 'huffman-coding-json') {
          throw new Error('This JSON file is not a Huffman compressed package.');
        }
        onLoadCompressedFile(parsed, f.name);
        return;
      }

      const buffer = await f.arrayBuffer();
      onLoadCompressedFile(arrayBufferToBase64(buffer), f.name);
    } catch (err) {
      onFileError(err.message || 'Could not read the compressed file.');
    }
  };
  huffGroup.appendChild(huffInput);

  wrap.append(textGroup, anyFileGroup, huffGroup);

  return wrap;
}

function arrayBufferToBase64(buffer) {
  const bytes = new Uint8Array(buffer);
  let binary = '';
  const chunkSize = 8192;

  for (let i = 0; i < bytes.length; i += chunkSize) {
    const chunk = bytes.subarray(i, i + chunkSize);
    binary += String.fromCharCode(...chunk);
  }

  return btoa(binary);
}
