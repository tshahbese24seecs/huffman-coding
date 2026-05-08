export default function createTextEditor(onAnalyze, onCompress, onDecompress) {
  const wrap = document.createElement('div');
  wrap.className = 'editor-wrap';

  const ta = document.createElement('textarea');
  ta.placeholder = 'Enter text to analyze or compress...';
  ta.rows = 8;
  wrap.appendChild(ta);

  const btns = document.createElement('div');
  btns.className = 'editor-buttons';

  const analyzeBtn = document.createElement('button');
  analyzeBtn.textContent = 'Analyze';
  analyzeBtn.onclick = () => onAnalyze(ta.value);
  btns.appendChild(analyzeBtn);

  const compressBtn = document.createElement('button');
  compressBtn.textContent = 'Compress & Download';
  compressBtn.onclick = () => onCompress(ta.value);
  btns.appendChild(compressBtn);

  const decompressBtn = document.createElement('button');
  decompressBtn.textContent = 'Decompress (paste b64)';
  decompressBtn.onclick = () => {
    const b64 = prompt('Paste base64 compressed data:');
    if (b64) onDecompress(b64);
  };
  btns.appendChild(decompressBtn);

  wrap.appendChild(btns);
  return wrap;
}
