export default function createTreeRenderer() {
  const wrap = document.createElement('div');
  wrap.className = 'tree-wrap';

  const title = document.createElement('h2');
  title.textContent = 'Huffman Tree (JSON)';
  wrap.appendChild(title);

  const pre = document.createElement('pre');
  pre.textContent = 'No tree yet.';
  wrap.appendChild(pre);

  return Object.assign(wrap, {
    update(tree) {
      // The backend returns `tree` as an object; if it's a string parse it
      const obj = (typeof tree === 'string') ? JSON.parse(tree) : tree;
      pre.textContent = JSON.stringify(obj, null, 2);
    }
  });
}
