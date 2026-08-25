# AKASHSHELL V0.1 — ARCHITECTURE

## 1. TECH STACK
*   **Language:** Rust (Stable)
*   **Toolchain:** Cargo / MSVC for Windows 10/11
*   **Build Environment:** Native Local Builds (`cargo build`). The development machine (this laptop) has Rust 1.96.0 installed and serves as the primary build host, bypassing the need for remote CI runners for dev cycles.

## 2. SYSTEM COMPONENTS

### 2.1 The Data Model (`nu-protocol` style)
Instead of string byte streams, pipelines pass structured data.
```rust
pub enum Value {
    Int(i64),
    String(String),
    Bool(bool),
    Record(HashMap<String, Value>),
    List(Vec<Value>),
}
```

### 2.2 Lexer & Parser
*   **Lexer:** Tokenizes the raw input string into shell primitives (commands, flags, strings, variables).
*   **Parser:** Constructs an Abstract Syntax Tree (AST). It understands piping `|` not as a byte stream, but as an object handoff.

### 2.3 Execution Engine (Evaluator)
*   Traverses the AST.
*   If a command is an internal `akashshell` built-in, it executes in-memory and yields a `Value`.
*   If a command is an external Windows binary (`.exe`), the engine spawns a subprocess.

### 2.4 Terminal & PTY Layer
*   **Renderer:** Uses a background thread to manage the TUI state, preventing long-running commands from freezing the prompt.
*   **ConPTY Integration:** Crucial for Windows. Wraps legacy EXEs in a Pseudo-Terminal so they output correctly formatted ANSI text to the `akashshell` renderer.

### 2.5 Extensibility (Wasm)
*   Plugin system designed around WebAssembly (e.g., using `wasmtime`).
*   Plugins communicate with the host shell via serialized `Value` objects over memory, enforcing strict safety and high speed.
