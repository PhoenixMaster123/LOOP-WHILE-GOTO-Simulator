// A faithful TypeScript interpreter for the Theo-IDE concept languages
// (LOOP / WHILE / GOTO), ported from the libtheo reference implementation.
//
// Semantics matched to libtheo:
//  - Variables are non-negative integers, default 0.
//  - `x - n` is monus (clamped at 0); `x + n` is normal addition.
//  - LOOP x DO ... END evaluates x ONCE, runs the body that many times.
//  - WHILE x != 0 DO ... END.
//  - GOTO label / `label:` / IF x = n THEN GOTO label (labels are per-program).
//  - STOP halts the whole program.
//  - PROGRAM name IN a,b OUT c DO ... END, called by `RUN name WITH ..., ... END`.
//  - Only `id + int` and `id - int` arithmetic (the libtheo standard macros).

// ---------------------------------------------------------------------------
// Tokenizer
// ---------------------------------------------------------------------------

type TokType =
  | "id" | "int"
  | "assign" | "colon" | "semi" | "comma" | "eq" | "neq" | "plus" | "minus"
  | "star" | "slash" | "percent" | "caret" | "lt" | "gt" | "le" | "ge"
  | "program" | "in" | "out" | "do" | "end" | "loop" | "while"
  | "goto" | "if" | "then" | "stop" | "run" | "with"
  | "eof";

interface Token { type: TokType; value: string; line: number; }

const KEYWORDS: Record<string, TokType> = {
  program: "program", prog: "program",
  in: "in", out: "out", do: "do", end: "end",
  loop: "loop", while: "while", goto: "goto",
  if: "if", then: "then", stop: "stop", run: "run", with: "with",
};

export class TheoError extends Error {
  line: number;
  constructor(msg: string, line: number) {
    super(msg);
    this.line = line;
  }
}

function tokenize(src: string): Token[] {
  const toks: Token[] = [];
  let i = 0;
  let line = 1;
  const push = (type: TokType, value: string) => toks.push({ type, value, line });

  while (i < src.length) {
    const c = src[i];
    if (c === "\n") { line++; i++; continue; }
    if (c === " " || c === "\t" || c === "\r") { i++; continue; }
    // comments
    if (c === "/" && src[i + 1] === "/") {
      while (i < src.length && src[i] !== "\n") i++;
      continue;
    }
    // multi-char operators
    if (c === ":" && src[i + 1] === "=") { push("assign", ":="); i += 2; continue; }
    if (c === "!" && src[i + 1] === "=") { push("neq", "!="); i += 2; continue; }
    if (c === "<" && src[i + 1] === "=") { push("le", "<="); i += 2; continue; }
    if (c === ">" && src[i + 1] === "=") { push("ge", ">="); i += 2; continue; }
    switch (c) {
      case ":": push("colon", ":"); i++; continue;
      case ";": push("semi", ";"); i++; continue;
      case ",": push("comma", ","); i++; continue;
      case "=": push("eq", "="); i++; continue;
      case "+": push("plus", "+"); i++; continue;
      case "-": push("minus", "-"); i++; continue;
      case "*": push("star", "*"); i++; continue;
      case "/": push("slash", "/"); i++; continue;
      case "%": push("percent", "%"); i++; continue;
      case "^": push("caret", "^"); i++; continue;
      case "<": push("lt", "<"); i++; continue;
      case ">": push("gt", ">"); i++; continue;
    }
    // numbers
    if (c >= "0" && c <= "9") {
      let s = "";
      while (i < src.length && src[i] >= "0" && src[i] <= "9") s += src[i++];
      push("int", s);
      continue;
    }
    // identifiers / keywords
    if (/[A-Za-z_]/.test(c)) {
      let s = "";
      while (i < src.length && /[A-Za-z0-9_]/.test(src[i])) s += src[i++];
      const kw = KEYWORDS[s.toLowerCase()];
      push(kw ?? "id", s);
      continue;
    }
    throw new TheoError(`Unexpected character '${c}'`, line);
  }
  toks.push({ type: "eof", value: "", line });
  return toks;
}

// ---------------------------------------------------------------------------
// AST
// ---------------------------------------------------------------------------

type BinOp = "+" | "-" | "*" | "/" | "%" | "^" | "=" | "<" | ">" | "<=" | ">=" | "!=";

type ValueExpr =
  | { k: "const"; n: number }
  | { k: "var"; name: string }
  | { k: "add"; name: string; delta: number }
  | { k: "call"; func: string; args: ValueExpr[] }
  | { k: "binop"; op: BinOp; a: ValueExpr; b: ValueExpr; line: number };

// ---------------------------------------------------------------------------
// Optional language extensions (infix operators), off by default.
// ---------------------------------------------------------------------------

export interface Extensions {
  add: boolean; // x + y  (add two variables)
  sub: boolean; // x - y  (subtract; clamped at 0)
  mul: boolean; // x * y  (multiply)
  div: boolean; // x / y  (integer division)
  mod: boolean; // x % y  /  x MOD y  (modulo)
  pow: boolean; // x ^ y  (power)
  eq: boolean;  // x = y  -> 1 / 0
  lt: boolean;  // x < y  -> 1 / 0
  gt: boolean;  // x > y  -> 1 / 0
  le: boolean;  // x <= y -> 1 / 0
  ge: boolean;  // x >= y -> 1 / 0
  ne: boolean;  // x != y -> 1 / 0
}

export const NO_EXTENSIONS: Extensions = {
  add: false, sub: false, mul: false, div: false, mod: false,
  pow: false, eq: false, lt: false, gt: false, le: false, ge: false, ne: false,
};

export interface ExtDef { key: keyof Extensions; syntax: string; desc: string; }

export const EXT_DEFS: ExtDef[] = [
  { key: "add", syntax: "x + y", desc: "add two variables" },
  { key: "sub", syntax: "x − y", desc: "subtract (never below 0)" },
  { key: "mul", syntax: "x * y", desc: "multiply" },
  { key: "div", syntax: "x / y", desc: "integer division (b > 0)" },
  { key: "mod", syntax: "x % y  ·  x MOD y", desc: "remainder (b > 0)" },
  { key: "pow", syntax: "x ^ y", desc: "power / exponentiation" },
  { key: "eq", syntax: "x = y", desc: "equal → 1 or 0" },
  { key: "ne", syntax: "x != y", desc: "not equal → 1 or 0" },
  { key: "lt", syntax: "x < y", desc: "less than → 1 or 0" },
  { key: "gt", syntax: "x > y", desc: "greater than → 1 or 0" },
  { key: "le", syntax: "x <= y", desc: "less or equal → 1 or 0" },
  { key: "ge", syntax: "x >= y", desc: "greater or equal → 1 or 0" },
];

const OP_EXT: Record<BinOp, keyof Extensions> = {
  "+": "add", "-": "sub", "*": "mul", "/": "div", "%": "mod",
  "^": "pow", "=": "eq", "<": "lt", ">": "gt", "<=": "le", ">=": "ge", "!=": "ne",
};

type Stmt =
  | { k: "assign"; target: string; value: ValueExpr; line: number }
  | { k: "label"; name: string; line: number }
  | { k: "loop"; varName: string; body: Stmt[]; line: number }
  | { k: "while"; varName: string; body: Stmt[]; line: number }
  | { k: "goto"; label: string; line: number }
  | { k: "if"; varName: string; num: number; label: string; line: number }
  | { k: "stop"; line: number };

interface FuncDef {
  name: string;
  params: string[];
  outVar: string;
  body: Stmt[];
}

// ---------------------------------------------------------------------------
// Parser (recursive descent, mirrors libtheo's grammar)
// ---------------------------------------------------------------------------

class Parser {
  toks: Token[];
  pos = 0;
  ext: Extensions;
  constructor(toks: Token[], ext: Extensions) { this.toks = toks; this.ext = ext; }

  peek(): Token { return this.toks[this.pos]; }
  next(): Token { return this.toks[this.pos++]; }
  at(t: TokType): boolean { return this.peek().type === t; }

  expect(t: TokType): Token {
    const tok = this.peek();
    if (tok.type !== t) {
      throw new TheoError(`Expected '${t}' but found '${tok.value || tok.type}'`, tok.line);
    }
    return this.next();
  }

  parseProgram(): { funcs: FuncDef[]; main: Stmt[] } {
    const funcs: FuncDef[] = [];
    while (this.at("program")) funcs.push(this.parseFuncDef());
    const main = this.parseStatements();
    if (!this.at("eof")) {
      const tok = this.peek();
      throw new TheoError(`Unexpected '${tok.value || tok.type}' after program`, tok.line);
    }
    return { funcs, main };
  }

  parseFuncDef(): FuncDef {
    this.expect("program");
    const name = this.expect("id").value;
    const params: string[] = [];
    let outVar = "x0";
    if (this.at("in")) {
      this.next();
      params.push(this.expect("id").value);
      while (this.at("comma")) { this.next(); params.push(this.expect("id").value); }
      if (this.at("out")) { this.next(); outVar = this.expect("id").value; }
    }
    this.expect("do");
    const body = this.parseStatements();
    this.expect("end");
    return { name, params, outVar, body };
  }

  // P -> statement (; statement)*  with labels (`id :`) not requiring a separator
  parseStatements(): Stmt[] {
    const stmts: Stmt[] = [];
    for (;;) {
      const t = this.peek().type;
      if (t === "end" || t === "eof" || t === "program") break;

      const isLabel = this.parseStatementInto(stmts);
      if (isLabel) continue; // label binds to the following statement, no ';'
      if (this.at("semi")) { this.next(); continue; }
      break;
    }
    return stmts;
  }

  // returns true if the statement parsed was a label
  parseStatementInto(stmts: Stmt[]): boolean {
    const tok = this.peek();
    const line = tok.line;
    switch (tok.type) {
      case "id": {
        const name = this.next().value;
        if (this.at("assign")) {
          this.next();
          stmts.push({ k: "assign", target: name, value: this.parseValue(), line });
          return false;
        }
        if (this.at("colon")) {
          this.next();
          stmts.push({ k: "label", name, line });
          return true;
        }
        throw new TheoError(`Expected ':=' or ':' after '${name}'`, line);
      }
      case "loop": {
        this.next();
        const varName = this.expect("id").value;
        this.expect("do");
        const body = this.parseStatements();
        this.expect("end");
        stmts.push({ k: "loop", varName, body, line });
        return false;
      }
      case "while": {
        this.next();
        const varName = this.expect("id").value;
        this.expect("neq");
        const zero = this.expect("int").value;
        if (zero !== "0") throw new TheoError("WHILE condition must be 'x != 0'", line);
        this.expect("do");
        const body = this.parseStatements();
        this.expect("end");
        stmts.push({ k: "while", varName, body, line });
        return false;
      }
      case "goto": {
        this.next();
        const label = this.expect("id").value;
        stmts.push({ k: "goto", label, line });
        return false;
      }
      case "if": {
        this.next();
        const varName = this.expect("id").value;
        this.expect("eq");
        const num = parseInt(this.expect("int").value, 10);
        this.expect("then");
        this.expect("goto");
        const label = this.expect("id").value;
        stmts.push({ k: "if", varName, num, label, line });
        return false;
      }
      case "stop": {
        this.next();
        stmts.push({ k: "stop", line });
        return false;
      }
      default:
        throw new TheoError(
          `Expected a statement but found '${tok.value || tok.type}'`, line);
    }
  }

  // VALUE -> additive expression.
  // Base Theo only allows `var ± number`; richer forms (var±var, * / %) need
  // the corresponding extension to be enabled.
  parseValue(): ValueExpr {
    return this.parseCompare();
  }

  // Lowest precedence: comparisons (= < >), returning 1 / 0.
  private parseCompare(): ValueExpr {
    let left = this.parseAdd();
    for (;;) {
      const tok = this.peek();
      let op: BinOp | null = null;
      if (tok.type === "eq") op = "=";
      else if (tok.type === "neq") op = "!=";
      else if (tok.type === "lt") op = "<";
      else if (tok.type === "gt") op = ">";
      else if (tok.type === "le") op = "<=";
      else if (tok.type === "ge") op = ">=";
      if (op === null) break;
      const line = tok.line;
      this.next();
      const right = this.parseAdd();
      left = this.combine(op, left, right, line);
    }
    return left;
  }

  private parseAdd(): ValueExpr {
    let left = this.parseMul();
    while (this.at("plus") || this.at("minus")) {
      const line = this.peek().line;
      const op: BinOp = this.next().type === "plus" ? "+" : "-";
      const right = this.parseMul();
      left = this.combine(op, left, right, line);
    }
    return left;
  }

  private parseMul(): ValueExpr {
    let left = this.parsePow();
    for (;;) {
      const tok = this.peek();
      let op: BinOp | null = null;
      if (tok.type === "star") op = "*";
      else if (tok.type === "slash") op = "/";
      else if (tok.type === "percent") op = "%";
      else if (tok.type === "id") {
        const lw = tok.value.toLowerCase();
        if (lw === "mod") op = "%";
        else if (lw === "div") op = "/";
      }
      if (op === null) break;
      const line = tok.line;
      this.next();
      const right = this.parsePow();
      left = this.combine(op, left, right, line);
    }
    return left;
  }

  // Highest precedence: power (right-associative).
  private parsePow(): ValueExpr {
    const left = this.parseFactor();
    if (this.at("caret")) {
      const line = this.peek().line;
      this.next();
      const right = this.parsePow();
      return this.combine("^", left, right, line);
    }
    return left;
  }

  private parseFactor(): ValueExpr {
    const tok = this.peek();
    if (tok.type === "run") {
      this.next();
      const func = this.expect("id").value;
      this.expect("with");
      const args: ValueExpr[] = [];
      if (!this.at("end")) {
        args.push(this.parseValue());
        while (this.at("comma")) { this.next(); args.push(this.parseValue()); }
      }
      this.expect("end");
      return { k: "call", func, args };
    }
    if (tok.type === "int") {
      this.next();
      return { k: "const", n: parseInt(tok.value, 10) };
    }
    if (tok.type === "id") {
      this.next();
      return { k: "var", name: tok.value };
    }
    throw new TheoError(
      `Expected a value (variable, number, or RUN ... END) ` +
      `but found '${tok.value || tok.type}'`, tok.line);
  }

  // Fold a binary operation, keeping base Theo (`var ± number`) always legal
  // and gating everything else behind the matching extension.
  private combine(op: BinOp, left: ValueExpr, right: ValueExpr, line: number): ValueExpr {
    if ((op === "+" || op === "-") && left.k === "var" && right.k === "const") {
      return { k: "add", name: left.name, delta: op === "+" ? right.n : -right.n };
    }
    if (!this.ext[OP_EXT[op]]) {
      const def = EXT_DEFS.find((d) => d.key === OP_EXT[op])!;
      throw new TheoError(
        `'${op}' between two values needs the "${def.syntax}" extension, ` +
        `which is off by default. Enable it via the ⚙ Extensions menu.`, line);
    }
    return { k: "binop", op, a: left, b: right, line };
  }
}

// ---------------------------------------------------------------------------
// Compiler: each function body -> flat instruction list (so GOTO works freely)
// ---------------------------------------------------------------------------

type Instr =
  | { k: "assign"; target: string; value: ValueExpr; line: number }
  | { k: "loopInit"; counter: number; varName: string; line: number }
  | { k: "jmpCounterZero"; counter: number; target: number }
  | { k: "dec"; counter: number }
  | { k: "jmpVarZero"; varName: string; target: number }
  | { k: "jmpVarEq"; varName: string; num: number; target: number }
  | { k: "jmp"; target: number }
  | { k: "halt" };

interface CompiledFunc {
  params: string[];
  outVar: string;
  code: Instr[];
  counters: number;
}

class FuncCompiler {
  code: Instr[] = [];
  labelPos = new Map<string, number>();
  pending: { index: number; label: string; line: number }[] = [];
  counters = 0;

  compile(stmts: Stmt[]): void {
    this.compileBlock(stmts);
    // resolve named jumps (GOTO / IF)
    for (const p of this.pending) {
      const tgt = this.labelPos.get(p.label);
      if (tgt === undefined) {
        throw new TheoError(`Unknown label '${p.label}'`, p.line);
      }
      const instr = this.code[p.index];
      if (instr.k === "jmp" || instr.k === "jmpVarEq") instr.target = tgt;
    }
  }

  compileBlock(stmts: Stmt[]): void {
    for (const s of stmts) {
      switch (s.k) {
        case "label": {
          if (this.labelPos.has(s.name)) {
            throw new TheoError(`Label '${s.name}' is defined more than once`, s.line);
          }
          this.labelPos.set(s.name, this.code.length);
          break;
        }
        case "assign":
          this.code.push({ k: "assign", target: s.target, value: s.value, line: s.line });
          break;
        case "stop":
          this.code.push({ k: "halt" });
          break;
        case "goto":
          this.pending.push({ index: this.code.length, label: s.label, line: s.line });
          this.code.push({ k: "jmp", target: -1 });
          break;
        case "if":
          this.pending.push({ index: this.code.length, label: s.label, line: s.line });
          this.code.push({ k: "jmpVarEq", varName: s.varName, num: s.num, target: -1 });
          break;
        case "loop": {
          const counter = this.counters++;
          this.code.push({ k: "loopInit", counter, varName: s.varName, line: s.line });
          const start = this.code.length;
          const jz: Instr = { k: "jmpCounterZero", counter, target: -1 };
          this.code.push(jz);
          this.compileBlock(s.body);
          this.code.push({ k: "dec", counter });
          this.code.push({ k: "jmp", target: start });
          jz.target = this.code.length;
          break;
        }
        case "while": {
          const start = this.code.length;
          const jz: Instr = { k: "jmpVarZero", varName: s.varName, target: -1 };
          this.code.push(jz);
          this.compileBlock(s.body);
          this.code.push({ k: "jmp", target: start });
          jz.target = this.code.length;
          break;
        }
      }
    }
  }
}

// ---------------------------------------------------------------------------
// Virtual machine
// ---------------------------------------------------------------------------

const STEP_LIMIT = 20_000_000;

class HaltSignal {}

export interface RunResult {
  variables: { name: string; value: number }[];
  steps: number;
}

class Machine {
  funcs: Map<string, CompiledFunc>;
  steps = 0;
  constructor(funcs: Map<string, CompiledFunc>) { this.funcs = funcs; }

  private tick(): void {
    if (++this.steps > STEP_LIMIT) {
      throw new TheoError(
        `Execution stopped after ${STEP_LIMIT.toLocaleString()} steps ` +
        `(possible infinite loop).`, 0);
    }
  }

  evalValue(v: ValueExpr, env: Map<string, number>): number {
    switch (v.k) {
      case "const": return v.n;
      case "var": return env.get(v.name) ?? 0;
      case "add": return Math.max(0, (env.get(v.name) ?? 0) + v.delta);
      case "call": {
        const fn = this.funcs.get(v.func);
        if (!fn) throw new TheoError(`Unknown program '${v.func}'`, 0);
        if (fn.params.length !== v.args.length) {
          throw new TheoError(
            `Program '${v.func}' expects ${fn.params.length} argument(s) ` +
            `but got ${v.args.length}`, 0);
        }
        const args = v.args.map((a) => this.evalValue(a, env));
        return this.runFunc(fn, args);
      }
      case "binop": {
        const a = this.evalValue(v.a, env);
        const b = this.evalValue(v.b, env);
        switch (v.op) {
          case "+": return a + b;
          case "-": return Math.max(0, a - b); // monus, like base Theo
          case "*": return a * b;
          case "/":
            if (b === 0) throw new TheoError("Division by zero", v.line);
            return Math.floor(a / b);
          case "%":
            if (b === 0) throw new TheoError("Modulo by zero", v.line);
            return a % b;
          case "^": return Math.pow(a, b);
          case "=": return a === b ? 1 : 0;
          case "!=": return a !== b ? 1 : 0;
          case "<": return a < b ? 1 : 0;
          case ">": return a > b ? 1 : 0;
          case "<=": return a <= b ? 1 : 0;
          case ">=": return a >= b ? 1 : 0;
        }
      }
    }
  }

  // returns the value of the function's OUT variable
  runFunc(fn: CompiledFunc, args: number[]): number {
    const env = new Map<string, number>();
    fn.params.forEach((p, idx) => env.set(p, Math.max(0, args[idx])));
    this.execute(fn, env);
    return env.get(fn.outVar) ?? 0;
  }

  execute(fn: CompiledFunc, env: Map<string, number>): void {
    const counters = new Array<number>(fn.counters).fill(0);
    const code = fn.code;
    let ip = 0;
    while (ip < code.length) {
      this.tick();
      const i = code[ip];
      switch (i.k) {
        case "assign":
          env.set(i.target, this.evalValue(i.value, env));
          ip++;
          break;
        case "loopInit":
          counters[i.counter] = env.get(i.varName) ?? 0;
          ip++;
          break;
        case "jmpCounterZero":
          ip = counters[i.counter] === 0 ? i.target : ip + 1;
          break;
        case "dec":
          counters[i.counter] = Math.max(0, counters[i.counter] - 1);
          ip++;
          break;
        case "jmpVarZero":
          ip = (env.get(i.varName) ?? 0) === 0 ? i.target : ip + 1;
          break;
        case "jmpVarEq":
          ip = (env.get(i.varName) ?? 0) === i.num ? i.target : ip + 1;
          break;
        case "jmp":
          ip = i.target;
          break;
        case "halt":
          throw new HaltSignal();
      }
    }
  }
}

// ---------------------------------------------------------------------------
// Public entry point
// ---------------------------------------------------------------------------

export function run(source: string, ext: Extensions = NO_EXTENSIONS): RunResult {
  const toks = tokenize(source);
  const parser = new Parser(toks, ext);
  const { funcs, main } = parser.parseProgram();

  const compiled = new Map<string, CompiledFunc>();
  for (const f of funcs) {
    if (compiled.has(f.name)) {
      throw new TheoError(`Program '${f.name}' is defined more than once`, 0);
    }
    const fc = new FuncCompiler();
    fc.compile(f.body);
    compiled.set(f.name, { params: f.params, outVar: f.outVar, code: fc.code, counters: fc.counters });
  }

  const mainCompiler = new FuncCompiler();
  mainCompiler.compile(main);
  const mainFunc: CompiledFunc = {
    params: [], outVar: "", code: mainCompiler.code, counters: mainCompiler.counters,
  };

  const machine = new Machine(compiled);
  const env = new Map<string, number>();
  try {
    machine.execute(mainFunc, env);
  } catch (e) {
    if (!(e instanceof HaltSignal)) throw e; // STOP ends the program cleanly
  }

  const variables = [...env.entries()]
    .map(([name, value]) => ({ name, value }))
    .sort((a, b) => a.name.localeCompare(b.name));

  return { variables, steps: machine.steps };
}

export interface Example { label: string; lang: string; code: string; }

export const EXAMPLES: Example[] = [
  {
    label: "LOOP — addition (x_0 = x_1 + x_2)",
    lang: "LOOP",
    code: `// LOOP: addition  x_0 = x_1 + x_2
// Convention: x_1, x_2, ... are the inputs and x_0 holds the result.
// Only 'var + 1' is allowed, so we add x_2 by counting up x_2 times.
x_1 := 5;
x_2 := 3;
x_0 := x_1;
LOOP x_2 DO
  x_0 := x_0 + 1;
END`,
  },
  {
    label: "LOOP — multiplication (x_0 = x_1 * x_2)",
    lang: "LOOP",
    code: `// LOOP: multiplication  x_0 = x_1 * x_2
// Add x_1 to the result x_2 times (a LOOP nested inside a LOOP).
x_1 := 4;
x_2 := 3;
x_0 := 0;
LOOP x_2 DO
  LOOP x_1 DO
    x_0 := x_0 + 1;
  END;
END`,
  },
  {
    label: "LOOP — factorial (x_0 = x_1!)",
    lang: "LOOP",
    code: `// LOOP: factorial  x_0 = x_1!   (x_1 is the input n)
// Round k multiplies the result x_0 by the counter x_2, then x_2 grows.
// x_3 is a helper (Hilfsvariable) holding x_2 * x_0.
x_0 := 1;
x_1 := 4;
x_2 := 1;
LOOP x_1 DO
  x_3 := 0;
  LOOP x_2 DO
    LOOP x_0 DO
      x_3 := x_3 + 1;
    END;
  END;
  x_0 := x_3 + 0;
  x_2 := x_2 + 1;
END`,
  },
  {
    label: "WHILE — power of two (x_0 = 2 ^ x_1)",
    lang: "WHILE",
    code: `// WHILE: power of two  x_0 = 2 ^ x_1
// Double the result until the counter x_1 reaches 0. Doubling x_0 means
// adding 2 for every unit of x_0, collected in the helper x_2.
x_1 := 3;
x_0 := 1;
WHILE x_1 != 0 DO
  x_2 := 0;
  LOOP x_0 DO
    x_2 := x_2 + 1;
    x_2 := x_2 + 1;
  END;
  x_0 := x_2 + 0;
  x_1 := x_1 - 1;
END`,
  },
  {
    label: "GOTO — addition (x_0 = x_1 + x_2)",
    lang: "GOTO",
    code: `// GOTO: addition  x_0 = x_1 + x_2  (pure GOTO, no LOOP/WHILE)
// Marks M1, M2, ... are jump targets; IF .. THEN GOTO exits the loop.
x_1 := 5;
x_2 := 3;
x_0 := x_1;
M1: IF x_2 = 0 THEN GOTO M2;
x_0 := x_0 + 1;
x_2 := x_2 - 1;
GOTO M1;
M2: STOP`,
  },
  {
    label: "GOTO — multiplication (x_0 = x_1 * x_2)",
    lang: "GOTO",
    code: `// GOTO: multiplication  x_0 = x_1 * x_2
// Add x_1 to the result, count x_2 down, and loop back to mark M1.
x_1 := 4;
x_2 := 3;
x_0 := 0;
M1: IF x_2 = 0 THEN GOTO M2;
LOOP x_1 DO
  x_0 := x_0 + 1
END;
x_2 := x_2 - 1;
GOTO M1;
M2: STOP`,
  },
  {
    label: "PROGRAM — subprogram call (RUN)",
    lang: "RUN",
    code: `// Subprogram with IN/OUT, called via RUN ... WITH ... END.
// Inside a PROGRAM, x_0/x_1 are local, so they don't clash with the caller.
PROGRAM Double IN x_1 OUT x_0 DO
  x_0 := x_1;
  LOOP x_1 DO
    x_0 := x_0 + 1
  END
END

x_1 := 7;
x_0 := RUN Double WITH x_1 END`,
  },
];
