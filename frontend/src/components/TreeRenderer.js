export default function createTreeRenderer() {
  const wrap = document.createElement('div');
  wrap.className = 'tree-wrap';

  const title = document.createElement('h2');
  title.textContent = 'Huffman Tree';
  wrap.appendChild(title);

  const empty = document.createElement('p');
  empty.className = 'empty-state';
  empty.textContent = 'The generated tree will appear here.';
  wrap.appendChild(empty);

  const scroller = document.createElement('div');
  scroller.className = 'tree-scroller';
  scroller.hidden = true;

  const treeContainer = document.createElement('div');
  treeContainer.className = 'tree-container';
  scroller.appendChild(treeContainer);
  wrap.appendChild(scroller);

  return Object.assign(wrap, {
    update(tree) {
      const obj = (typeof tree === 'string') ? JSON.parse(tree) : tree;
      treeContainer.replaceChildren();

      if (!obj) {
        empty.hidden = false;
        scroller.hidden = true;
        return;
      }

      treeContainer.appendChild(renderNode(obj));
      empty.hidden = true;
      scroller.hidden = false;
    },
    clear() {
      treeContainer.replaceChildren();
      empty.hidden = false;
      scroller.hidden = true;
    }
  });
}

function renderNode(node) {
  const branch = document.createElement('div');
  branch.className = 'tree-branch';

  const nodeEl = document.createElement('div');
  const isLeaf = node.value !== null && node.value !== undefined;
  nodeEl.className = `tree-node ${isLeaf ? 'leaf' : 'internal'}`;

  const label = document.createElement('span');
  label.className = 'node-label';
  label.textContent = isLeaf ? displayValue(node.value) : 'internal';

  const freq = document.createElement('span');
  freq.className = 'node-freq';
  freq.textContent = String(node.freq);

  nodeEl.append(label, freq);
  branch.appendChild(nodeEl);

  const children = [node.left, node.right].filter(Boolean);
  if (children.length > 0) {
    const childWrap = document.createElement('div');
    childWrap.className = 'tree-children';
    children.forEach((child) => childWrap.appendChild(renderNode(child)));
    branch.appendChild(childWrap);
  }

  return branch;
}

function displayValue(value) {
  if (value === ' ') return 'space';
  if (value === '\n') return 'newline';
  if (value === '\t') return 'tab';
  if (value === '\r') return 'return';
  return value;
}
