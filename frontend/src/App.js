import { analyze, compress, compressFileData, decompress } from './api.js';
import createTextEditor from './components/TextEditor.js';
import createFreqTable from './components/FreqTable.js';
import createTreeRenderer from './components/TreeRenderer.js';
import createFileControls from './components/FileControls.js';

export default function App() {
  const container = document.createElement('div');
  container.className = 'app-container';

  const header = document.createElement('h1');
  header.textContent = 'Huffman Coding Visualizer';
  container.appendChild(header);

  const status = document.createElement('div');
  status.className = 'status-message info';
  status.textContent = 'Enter text, or compress any file into a portable Huffman JSON package.';
  container.appendChild(status);

  const editor = createTextEditor(onAnalyze, onCompress);
  container.appendChild(editor);

  const fileControls = createFileControls(onLoadTextFile, onLoadCompressedFile, onFileError, onCompressFile);
  container.appendChild(fileControls);

  const statsPanel = createStatsPanel();
  container.appendChild(statsPanel);

  const outputPanel = createOutputPanel();
  container.appendChild(outputPanel);

  const results = document.createElement('div');
  results.className = 'results';
  container.appendChild(results);

  const freqTable = createFreqTable();
  results.appendChild(freqTable);

  const treeRenderer = createTreeRenderer();
  results.appendChild(treeRenderer);

  function setStatus(type, msg) {
    status.className = `status-message ${type}`;
    status.textContent = msg;
  }

  function setBusy(isBusy, label = 'Working...') {
    editor.setBusy(isBusy);
    if (isBusy) {
      status.className = 'status-message info';
      const spinner = document.createElement('span');
      spinner.className = 'loading';
      status.replaceChildren(spinner, document.createTextNode(label));
    }
  }

  function validateText(text) {
    if (text.length === 0) {
      setStatus('error', 'Please enter text or load a text file first.');
      return false;
    }
    return true;
  }

  async function onAnalyze(text) {
    if (!validateText(text)) return;
    setBusy(true, 'Analyzing frequencies and prefix codes...');
    const res = await analyze(text);
    setBusy(false);
    if (!res.success) {
      setStatus('error', res.error || 'Analyze failed.');
      return;
    }

    freqTable.update(res.frequencies, res.codes);
    treeRenderer.update(res.tree);
    statsPanel.update({
      textSize: res.text_size,
      uniqueChars: res.unique_chars
    });
    outputPanel.clear();
    setStatus('success', 'Analysis complete. The frequency table and Huffman tree are updated.');
  }

  async function onCompress(text) {
    if (!validateText(text)) return;
    setBusy(true, 'Compressing text and preparing the download...');
    const res = await compress(text);
    setBusy(false);
    if (!res.success) {
      setStatus('error', res.error || 'Compress failed.');
      return;
    }

    freqTable.update(res.frequencies, res.codes);
    treeRenderer.update(res.tree);
    statsPanel.update({
      originalSize: res.original_size,
      compressedSize: res.compressed_size,
      compressionRatio: res.compression_ratio,
      uniqueChars: Object.keys(res.frequencies || {}).length
    });
    const packageData = createCompressedPackage(res, {
      originalName: 'input.txt',
      mimeType: 'text/plain;charset=utf-8'
    });
    outputPanel.showCompressed(packageData);
    downloadJson(packageData, 'compressed-huffman.json');
    setStatus('success', 'Compression complete. A portable JSON file was downloaded and can be decompressed on any device with this app.');
  }

  async function onCompressFile(file) {
    setBusy(true, `Compressing ${file.name}...`);
    const res = await compressFileData(file.data_b64, file.name, file.type || 'application/octet-stream');
    setBusy(false);
    if (!res.success) {
      setStatus('error', res.error || 'File compression failed.');
      return;
    }

    freqTable.update(res.frequencies, res.codes);
    treeRenderer.update(res.tree);
    statsPanel.update({
      originalSize: res.original_size,
      compressedSize: res.compressed_size,
      compressionRatio: res.compression_ratio,
      uniqueChars: Object.keys(res.frequencies || {}).length
    });

    const packageData = createCompressedPackage(res, {
      originalName: file.name,
      mimeType: file.type || 'application/octet-stream'
    });
    const downloadName = `${stripExtension(file.name)}.huffman.json`;
    outputPanel.showCompressed(packageData);
    downloadJson(packageData, downloadName);
    setStatus('success', `${file.name} was compressed into ${downloadName}.`);
  }

  async function onLoadCompressedFile(payload, fileName) {
    setBusy(true, `Decompressing ${fileName}...`);
    const dataB64 = typeof payload === 'string' ? payload : payload.data_b64;
    const metadata = typeof payload === 'string' ? {} : payload.metadata || {};
    const res = await decompress(dataB64);
    setBusy(false);
    if (!res.success) {
      setStatus('error', res.error || 'Decompression failed.');
      return;
    }

    const decodedName = metadata.original_filename || metadata.original_name || 'decompressed-output.bin';
    const mimeType = metadata.mime_type || 'application/octet-stream';
    downloadBase64File(res.data_b64, decodedName, mimeType);

    if (res.text !== null && res.text !== undefined) {
      outputPanel.showText(res.text);
    } else {
      outputPanel.showMessage(`${decodedName} was restored and downloaded. Binary files are not displayed as text.`);
    }

    statsPanel.update({
      decodedSize: res.original_size
    });
    setStatus('success', `${fileName} was decompressed successfully and ${decodedName} was downloaded.`);
  }

  function onLoadTextFile(text) {
    editor.setValue(text);
    statsPanel.clear();
    outputPanel.clear();
    freqTable.clear();
    treeRenderer.clear();
    setStatus('info', 'Text file loaded. You can analyze or compress it now.');
  }

  function onFileError(message) {
    setStatus('error', message);
  }

  function downloadJson(data, filename) {
    const blob = new Blob([JSON.stringify(data, null, 2)], { type: 'application/json' });
    const url = URL.createObjectURL(blob);
    const a = document.createElement('a');
    a.href = url;
    a.download = filename;
    a.click();
    URL.revokeObjectURL(url);
  }

  return container;
}

function createCompressedPackage(result, source) {
  return {
    format: 'huffman-coding-json',
    version: 1,
    encoding: 'base64',
    data_b64: result.data_b64,
    metadata: {
      original_filename: source.originalName,
      mime_type: source.mimeType,
      original_size: result.original_size,
      compressed_size: result.compressed_size,
      compression_ratio: result.compression_ratio,
      frequencies: result.frequencies,
      codes: result.codes
    }
  };
}

function downloadBase64File(dataB64, filename, mimeType) {
  const binary = Uint8Array.from(atob(dataB64), c => c.charCodeAt(0));
  const blob = new Blob([binary], { type: mimeType });
  const url = URL.createObjectURL(blob);
  const a = document.createElement('a');
  a.href = url;
  a.download = filename;
  a.click();
  URL.revokeObjectURL(url);
}

function stripExtension(name) {
  const idx = name.lastIndexOf('.');
  return idx > 0 ? name.slice(0, idx) : name;
}

function createStatsPanel() {
  const panel = document.createElement('section');
  panel.className = 'stats-panel';
  panel.hidden = true;

  const title = document.createElement('h2');
  title.textContent = 'Compression Stats';
  panel.appendChild(title);

  const grid = document.createElement('div');
  grid.className = 'stats-grid';
  panel.appendChild(grid);

  return Object.assign(panel, {
    update(stats) {
      const items = [];

      if (stats.textSize !== undefined) items.push(['Text size', `${stats.textSize} chars`]);
      if (stats.originalSize !== undefined) items.push(['Original size', `${stats.originalSize} bytes`]);
      if (stats.compressedSize !== undefined) items.push(['Compressed size', `${stats.compressedSize} bytes`]);
      if (stats.uniqueChars !== undefined) items.push(['Unique chars', stats.uniqueChars]);
      if (stats.decodedSize !== undefined) items.push(['Decoded size', `${stats.decodedSize} chars`]);
      if (stats.compressionRatio !== undefined) {
        const ratio = Number(stats.compressionRatio);
        const saved = Math.max(0, (1 - ratio) * 100);
        items.push(['Ratio', ratio.toFixed(3)]);
        items.push(['Space saved', `${saved.toFixed(1)}%`]);
      }

      grid.replaceChildren(...items.map(([label, value]) => {
        const card = document.createElement('div');
        card.className = 'stat-card';
        card.innerHTML = `<span>${label}</span><strong>${value}</strong>`;
        return card;
      }));
      panel.hidden = items.length === 0;
    },
    clear() {
      grid.replaceChildren();
      panel.hidden = true;
    }
  });
}

function createOutputPanel() {
  const panel = document.createElement('section');
  panel.className = 'output-panel';
  panel.hidden = true;

  const title = document.createElement('h2');
  title.textContent = 'Output';

  const actions = document.createElement('div');
  actions.className = 'output-actions';

  const copyBtn = document.createElement('button');
  copyBtn.className = 'secondary';
  copyBtn.textContent = 'Copy';

  const downloadBtn = document.createElement('button');
  downloadBtn.className = 'secondary';
  downloadBtn.textContent = 'Download Text';

  actions.append(copyBtn, downloadBtn);

  const header = document.createElement('div');
  header.className = 'panel-header';
  header.append(title, actions);
  panel.appendChild(header);

  const body = document.createElement('pre');
  body.className = 'output-body';
  panel.appendChild(body);

  let currentText = '';

  copyBtn.onclick = async () => {
    await navigator.clipboard.writeText(currentText);
    copyBtn.textContent = 'Copied';
    setTimeout(() => {
      copyBtn.textContent = 'Copy';
    }, 1200);
  };

  downloadBtn.onclick = () => {
    const blob = new Blob([currentText], { type: 'text/plain' });
    const url = URL.createObjectURL(blob);
    const a = document.createElement('a');
    a.href = url;
    a.download = 'decompressed.txt';
    a.click();
    URL.revokeObjectURL(url);
  };

  return Object.assign(panel, {
    showText(text) {
      currentText = text;
      title.textContent = 'Decompressed Text';
      body.textContent = text;
      actions.hidden = false;
      downloadBtn.hidden = false;
      panel.hidden = false;
    },
    showMessage(message) {
      currentText = message;
      title.textContent = 'Decompressed File';
      body.textContent = message;
      actions.hidden = false;
      downloadBtn.hidden = true;
      panel.hidden = false;
    },
    showCompressed(packageData) {
      currentText = JSON.stringify(packageData, null, 2);
      title.textContent = 'Portable Compressed Package';
      body.textContent = currentText;
      actions.hidden = false;
      downloadBtn.hidden = true;
      panel.hidden = false;
    },
    clear() {
      currentText = '';
      body.textContent = '';
      downloadBtn.hidden = false;
      actions.hidden = false;
      panel.hidden = true;
    }
  });
}
