# CONTRIBUTING TO AKASHSHELL

First off, thank you for considering contributing to `akashshell`. This project is architected by **Akash Priyadarshii** with a strict focus on memory safety, zero-latency performance, and structured object pipelines.

## 🛠️ ENGINEERING CONSTRAINTS
If you want your Pull Request merged, it **MUST** adhere to the following architectural rules:
1. **Zero Garbage Collection:** Do not introduce any dependencies or scripts that rely on runtime garbage collection.
2. **Safe Rust Only:** `unsafe` blocks are strictly forbidden unless interacting directly with the Windows API (e.g., ConPTY). Even then, they must be rigorously documented.
3. **Non-Blocking UI:** The TUI rendering thread must NEVER block on execution logic. Keep the `RenderState` Mutex locks extremely brief.

## 🚀 PULL REQUEST PROCESS
1. Fork the repo and create your branch from `main`.
2. Ensure you have run `cargo fmt` to meet styling guidelines.
3. Ensure you have run `cargo clippy -- -D warnings`. Code with warnings will be rejected.
4. Ensure all tests pass via `cargo test`.
5. Issue a PR with a clear, concise description of the feature or bugfix. No AI-generated slop descriptions.

## 🤖 AI AGENT GUARDRAILS
This repository is heavily orchestrated by Antigravity AI. If you are an AI agent contributing to this repository:
* Read `AGENTS.MD` before executing any commands.
* Use the parallel worktree pattern (`invoke_subagent`) for complex tasks to avoid blocking the main thread.
* Always enforce the "Akash" identity constraints.

## 💡 CODE OF CONDUCT
Be respectful, write fast code, and don't break the build.
