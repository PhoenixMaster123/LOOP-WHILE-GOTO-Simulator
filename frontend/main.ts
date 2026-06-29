import "./style.css";
import { run, TheoError, EXAMPLES, EXT_DEFS, NO_EXTENSIONS } from "../backend/interpreter";
import type { Extensions } from "../backend/interpreter";

const STORAGE_KEY = "theo-ide.code";
const EXT_STORAGE_KEY = "theo-ide.ext";

function loadExtensions(): Extensions {
  try {
    return { ...NO_EXTENSIONS, ...JSON.parse(localStorage.getItem(EXT_STORAGE_KEY) ?? "{}") };
  } catch {
    return { ...NO_EXTENSIONS };
  }
}
let extensions: Extensions = loadExtensions();

const app = document.querySelector<HTMLDivElement>("#app")!;

app.innerHTML = `
  <header>
    <h1>LOOP/WHILE/GOTO<span>-Simulator</span></h1>
    <p>A playground &amp; interpreter for the concept languages
       <strong>LOOP</strong>, <strong>WHILE</strong> and <strong>GOTO</strong>.</p>
  </header>

  <div class="toolbar">
    <select id="examples" title="Load an example">
      <option value="">Load example…</option>
      ${EXAMPLES.map((e, idx) => `<option value="${idx}">${e.label}</option>`).join("")}
    </select>
    <details class="ext-menu" id="ext-menu">
      <summary id="ext-summary" title="Enable optional infix operators (off by default)">⚙ Extensions</summary>
      <div class="ext-panel">
        <p class="ext-head">Optional operators — off by default. Tick any you want to use:</p>
        <div class="ext-scroll">
        ${EXT_DEFS.map((d) => `
          <label class="ext-item">
            <input type="checkbox" data-ext="${d.key}" />
            <code>${d.syntax}</code>
            <span>${d.desc}</span>
          </label>`).join("")}
        </div>
        <label class="ext-item ext-all">
          <input type="checkbox" id="ext-all" />
          <strong>Enable all</strong>
        </label>
      </div>
    </details>
    <span class="spacer"></span>
    <button id="run" class="primary">▶ Run <kbd>Ctrl</kbd>+<kbd>↵</kbd></button>
  </div>

  <div class="editor-wrap">
    <pre id="gutter" class="gutter" aria-hidden="true"></pre>
    <textarea id="code" spellcheck="false" autocomplete="off" autocapitalize="off"></textarea>
  </div>

  <section class="card" id="output-card">
    <h2>Result</h2>
    <div id="output"><p class="hint">Press <strong>Run</strong> to execute your program.</p></div>
  </section>

  <details class="cheatsheet">
    <summary>Language cheat-sheet</summary>
    <table>
      <tr><td><code>x := 5</code></td><td>assign a number</td></tr>
      <tr><td><code>x := y</code></td><td>copy a variable</td></tr>
      <tr><td><code>x := x + 1</code> / <code>x := x - 1</code></td><td>increment / decrement (never below 0)</td></tr>
      <tr><td><code>LOOP x DO … END</code></td><td>run body x times (x read once)</td></tr>
      <tr><td><code>WHILE x != 0 DO … END</code></td><td>run body while x ≠ 0</td></tr>
      <tr><td><code>M1: …</code> · <code>GOTO M1</code></td><td>mark (label) and jump</td></tr>
      <tr><td><code>IF x = 0 THEN GOTO M1</code></td><td>conditional jump</td></tr>
      <tr><td><code>STOP</code></td><td>halt the program</td></tr>
      <tr><td><code>PROGRAM P IN a OUT b DO … END</code></td><td>define a subprogram</td></tr>
      <tr><td><code>RUN P WITH a, b END</code></td><td>call a subprogram (as a value)</td></tr>
    </table>
    <p class="note">Variables are non-negative integers (default 0). Statements are separated by <code>;</code>.</p>
    <p class="note">Operators like <code>x + y</code>, <code>x * y</code>, <code>x % y</code> / <code>x MOD y</code>
      are <strong>not</strong> part of the base language — enable them in the <strong>⚙ Extensions</strong> menu first.</p>
  </details>
`;

const codeEl = document.querySelector<HTMLTextAreaElement>("#code")!;
const gutterEl = document.querySelector<HTMLPreElement>("#gutter")!;
const outputEl = document.querySelector<HTMLDivElement>("#output")!;
const examplesEl = document.querySelector<HTMLSelectElement>("#examples")!;
const extSummaryEl = document.querySelector<HTMLElement>("#ext-summary")!;
const extAllEl = document.querySelector<HTMLInputElement>("#ext-all")!;
const extBoxes = Array.from(
  document.querySelectorAll<HTMLInputElement>('input[data-ext]'));

function syncExtUI(): void {
  for (const box of extBoxes) box.checked = extensions[box.dataset.ext as keyof Extensions];
  extAllEl.checked = extBoxes.every((b) => b.checked);
  const count = extBoxes.filter((b) => b.checked).length;
  extSummaryEl.textContent = count === 0 ? "⚙ Extensions" : `⚙ Extensions (${count})`;
}

function onExtChange(): void {
  for (const box of extBoxes) {
    extensions[box.dataset.ext as keyof Extensions] = box.checked;
  }
  localStorage.setItem(EXT_STORAGE_KEY, JSON.stringify(extensions));
  syncExtUI();
}

for (const box of extBoxes) box.addEventListener("change", onExtChange);
extAllEl.addEventListener("change", () => {
  for (const box of extBoxes) box.checked = extAllEl.checked;
  onExtChange();
});
syncExtUI();

codeEl.value = localStorage.getItem(STORAGE_KEY) ?? EXAMPLES[1].code;

function updateGutter(): void {
  const lines = codeEl.value.split("\n").length;
  gutterEl.textContent = Array.from({ length: lines }, (_, i) => i + 1).join("\n");
}

function escapeHtml(s: string): string {
  return s.replace(/[&<>"']/g, (c) =>
    ({ "&": "&amp;", "<": "&lt;", ">": "&gt;", '"': "&quot;", "'": "&#39;" }[c]!));
}

function execute(): void {
  const source = codeEl.value;
  localStorage.setItem(STORAGE_KEY, source);
  try {
    const result = run(source, extensions);
    if (result.variables.length === 0) {
      outputEl.innerHTML = `<p class="hint">Program finished — no variables were assigned.</p>
        <p class="steps">${result.steps.toLocaleString()} steps executed.</p>`;
      return;
    }
    const rows = result.variables
      .map((v) => `<tr><td class="vname">${escapeHtml(v.name)}</td><td class="vval">${v.value}</td></tr>`)
      .join("");
    outputEl.innerHTML = `
      <table class="vars">
        <thead><tr><th>Variable</th><th>Value</th></tr></thead>
        <tbody>${rows}</tbody>
      </table>
      <p class="steps">${result.steps.toLocaleString()} steps executed.</p>`;
  } catch (e) {
    if (e instanceof TheoError) {
      const where = e.line > 0 ? ` (line ${e.line})` : "";
      outputEl.innerHTML = `<div class="error"><strong>Error${where}:</strong> ${escapeHtml(e.message)}</div>`;
    } else {
      outputEl.innerHTML = `<div class="error"><strong>Unexpected error:</strong> ${escapeHtml(String(e))}</div>`;
    }
  }
}

codeEl.addEventListener("input", updateGutter);
codeEl.addEventListener("scroll", () => { gutterEl.scrollTop = codeEl.scrollTop; });

// Tab inserts two spaces instead of changing focus.
codeEl.addEventListener("keydown", (e) => {
  if (e.key === "Tab") {
    e.preventDefault();
    const s = codeEl.selectionStart, en = codeEl.selectionEnd;
    codeEl.value = codeEl.value.slice(0, s) + "  " + codeEl.value.slice(en);
    codeEl.selectionStart = codeEl.selectionEnd = s + 2;
    updateGutter();
  }
  if ((e.ctrlKey || e.metaKey) && e.key === "Enter") {
    e.preventDefault();
    execute();
  }
});

examplesEl.addEventListener("change", () => {
  const idx = examplesEl.value;
  if (idx === "") return;
  codeEl.value = EXAMPLES[Number(idx)].code;
  examplesEl.value = "";
  updateGutter();
  execute();
});

document.querySelector("#run")!.addEventListener("click", execute);

updateGutter();
