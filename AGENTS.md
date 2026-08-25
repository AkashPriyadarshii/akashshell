# AGENTS.md

## Project Overview & Exact Tech Stack
`akashshell` is a modern, elite-grade terminal shell for Windows 10/11. It replaces PowerShell and Bash by combining the structured data pipeline philosophy (passing objects instead of text) with the raw performance of Rust.
- **Tech Stack:** Rust (Stable), Cargo, Windows API (ConPTY).

## Environment & Tooling Commands
- **Build:** `cargo build`
- **Run:** `cargo run`
- **Format:** `cargo fmt`
- **Lint:** `cargo clippy -- -D warnings`

## Testing Instructions
- **Run Tests:** `cargo test`
- Agents must run `cargo clippy` and `cargo test` before committing.
- Add or update tests for the code you change, even if nobody asked. Fix any test or type errors until the whole suite is green.

## Project Structure & Architecture
- `akashshell-core/`: Contains the evaluation engine and the `Value` enum data model (Records, Lists, Strings). Inspired by `nushell`.
- `akashshell-parser/`: Handles the Lexer and AST generation.
- `akashshell-tui/`: Handles the non-blocking terminal rendering loop, Fish-like autosuggestions, and syntax highlighting. Inspired by `tuie`.
- `akashshell-pty/`: Handles Windows ConPTY integration for spawning legacy `.exe` files seamlessly. Inspired by `alacritty` and `rmux`.

## Local Coding Conventions & Patterns
- **Structured Data:** Always pass data between pipeline stages using the `Value` enum. Never fall back to raw string byte streams unless explicitly interacting with a legacy binary.
- **Zero Latency:** The TUI rendering thread must be strictly separated from the execution thread. The prompt must never block.
- **Multi-Agent Orchestration:** We follow the `grok-build` pattern. For complex tasks, the Orchestrator (Antigravity) should use `invoke_subagent` to spawn concurrent sub-agents into isolated Git worktrees to build components in parallel.

## Clear Operational Boundaries & Guardrails
- **No .NET/GC:** Do not introduce garbage-collected runtime dependencies.
- **Platform:** V0.1 is strictly for Windows 10/11. Do not spend time configuring macOS/Linux specific build features yet.
- **Security:** Do not execute destructive commands (like deleting all files) without explicit human override.
