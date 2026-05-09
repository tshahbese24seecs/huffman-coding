export default function createTextEditor(onAnalyze, onCompress) {
  const wrap = document.createElement('div');
  wrap.className = 'editor-wrap';

  const label = document.createElement('label');
  label.textContent = 'Input text';
  label.htmlFor = 'source-text';
  wrap.appendChild(label);

  const ta = document.createElement('textarea');
  ta.id = 'source-text';
  ta.placeholder = 'Enter text to analyze or compress...';
  ta.rows = 8;
  wrap.appendChild(ta);

  const btns = document.createElement('div');
  btns.className = 'editor-buttons';

  const analyzeBtn = document.createElement('button');
  analyzeBtn.className = 'analyze';
  analyzeBtn.textContent = 'Analyze';
  analyzeBtn.onclick = () => onAnalyze(ta.value);
  btns.appendChild(analyzeBtn);

  const compressBtn = document.createElement('button');
  compressBtn.className = 'compress';
  compressBtn.textContent = 'Compress & Download';
  compressBtn.onclick = () => onCompress(ta.value);
  btns.appendChild(compressBtn);

  wrap.appendChild(btns);
  return Object.assign(wrap, {
    setValue(value) {
      ta.value = value;
      ta.focus();
    },
    setBusy(isBusy) {
      analyzeBtn.disabled = isBusy;
      compressBtn.disabled = isBusy;
    }
  });
}
