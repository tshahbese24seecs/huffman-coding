export default function createFreqTable() {
  const wrap = document.createElement('div');
  wrap.className = 'freq-wrap';

  const title = document.createElement('h2');
  title.textContent = 'Frequencies & Codes';
  wrap.appendChild(title);

  const empty = document.createElement('p');
  empty.className = 'empty-state';
  empty.textContent = 'Analyze or compress text to see the code table.';
  wrap.appendChild(empty);

  const table = document.createElement('table');
  table.className = 'freq-table';
  table.hidden = true;
  table.innerHTML = `
    <thead>
      <tr>
        <th>Character</th>
        <th>Frequency</th>
        <th>Code</th>
      </tr>
    </thead>
    <tbody></tbody>
  `;
  wrap.appendChild(table);

  const tbody = table.querySelector('tbody');

  return Object.assign(wrap, {
    update(freqObj, codesObj) {
      tbody.replaceChildren();

      const rows = Object.keys(freqObj || {}).sort((a, b) => {
        const byFreq = freqObj[b] - freqObj[a];
        return byFreq || a.localeCompare(b);
      });

      empty.hidden = rows.length > 0;
      table.hidden = rows.length === 0;

      for (const key of rows) {
        const row = document.createElement('tr');
        const charCell = document.createElement('td');
        const char = document.createElement('span');
        char.className = 'char-col';
        char.textContent = displayKey(key);
        charCell.appendChild(char);

        const freqCell = document.createElement('td');
        freqCell.textContent = String(freqObj[key]);

        const codeCell = document.createElement('td');
        const code = document.createElement('span');
        code.className = 'code-col';
        code.textContent = codesObj?.[key] ?? '';
        codeCell.appendChild(code);

        row.append(charCell, freqCell, codeCell);
        tbody.appendChild(row);
      }
    },
    clear() {
      tbody.replaceChildren();
      empty.hidden = false;
      table.hidden = true;
    }
  });
}

function displayKey(key) {
  if (key === 'SPACE') return 'space';
  if (key === '\\n') return 'newline';
  if (key === '\\t') return 'tab';
  if (key === '\\r') return 'return';
  return key;
}
