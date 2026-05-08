import { analyze, compress, decompress } from './api.js';
import createTextEditor from './components/TextEditor.js';
import createFreqTable from './components/FreqTable.js';
import createTreeRenderer from './components/TreeRenderer.js';
import createFileControls from './components/FileControls.js';

export default function App() {
  const container = document.createElement('div');
  container.className = 'app-container';

  const header = document.createElement('h1');
  header.textContent = 'Huffman Coding — UI';
  container.appendChild(header);

  const editor = createTextEditor(onAnalyze, onCompress, onDecompress);
  container.appendChild(editor);

  const fileControls = createFileControls(onLoadFile);
  container.appendChild(fileControls);

  const results = document.createElement('div');
  results.className = 'results';
  container.appendChild(results);

  const freqTable = createFreqTable();
  results.appendChild(freqTable);

  const treeRenderer = createTreeRenderer();
  results.appendChild(treeRenderer);

  function showError(msg) {
    alert(msg);
  }

  async function onAnalyze(text) {
    const res = await analyze(text);
    if (!res.success) return showError(res.error || 'Analyze failed');
    freqTable.update(res.frequencies, res.codes);
    treeRenderer.update(res.tree);
  }

  async function onCompress(text) {
    const res = await compress(text);
    if (!res.success) return showError(res.error || 'Compress failed');
    freqTable.update(res.frequencies, res.codes);
    treeRenderer.update(res.tree);
    // download compressed
    const binary = Uint8Array.from(atob(res.data_b64), c => c.charCodeAt(0));
    const blob = new Blob([binary], { type: 'application/octet-stream' });
    const url = URL.createObjectURL(blob);
    const a = document.createElement('a');
    a.href = url; a.download = 'out.huff'; a.click(); URL.revokeObjectURL(url);
  }

  async function onDecompress(b64) {
    const res = await decompress(b64);
    if (!res.success) return showError(res.error || 'Decompress failed');
    alert('Decompressed text:\n' + res.text);
  }

  function onLoadFile(text) {
    // populate editor textarea
    const ta = container.querySelector('textarea');
    ta.value = text;
  }

  return container;
}
