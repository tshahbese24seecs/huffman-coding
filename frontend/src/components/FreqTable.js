export default function createFreqTable() {
  const wrap = document.createElement('div');
  wrap.className = 'freq-wrap';

  const title = document.createElement('h2');
  title.textContent = 'Frequencies & Codes';
  wrap.appendChild(title);

  const pre = document.createElement('pre');
  pre.textContent = 'No data yet.';
  wrap.appendChild(pre);

  return Object.assign(wrap, {
    update(freqObj, codesObj) {
      const freqLines = [];
      for (const key of Object.keys(freqObj))
        freqLines.push(`${key}: ${freqObj[key]} (${codesObj[key] ?? ''})`);
      pre.textContent = freqLines.join('\n') || 'No data yet.';
    }
  });
}
