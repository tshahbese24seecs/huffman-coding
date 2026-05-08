export default function createFileControls(onLoadFile) {
  const wrap = document.createElement('div');
  wrap.className = 'file-controls';

  const input = document.createElement('input');
  input.type = 'file';
  input.onchange = (e) => {
    const f = e.target.files[0];
    if (!f) return;
    const reader = new FileReader();
    reader.onload = () => onLoadFile(String(reader.result));
    reader.readAsText(f);
  };
  wrap.appendChild(input);

  return wrap;
}
