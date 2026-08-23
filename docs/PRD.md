# Product Requirements Document (PRD) - akashshell v0.2 (Production Grade)

## Vision
The absolute "max best" Windows 11 shell and terminal emulator. Built by a vibecoder to compress a 6-month elite engineering roadmap into immediate production-grade execution. 
**Strict Constraint:** 100% Pure C and Windows API. No MVP. No web frameworks. No Rust.

## Target Audience
Akash Priyadarshi — demanding zero-latency, elite UX, and extreme bare-metal performance.

## Core Requirements (v0.2)
- **R1 - Custom Direct2D Node Engine:** Rip out all OS-level UI controls. Build a lightweight flex-style layout engine in C to calculate bounding boxes, animations, and hover states natively at 60fps.
- **R2 - Bespoke Text Editor:** Implement a gap-buffer text editing structure in C. Render text manually via `IDWriteTextLayout`. Support syntax highlighting, multi-cursor, and sub-pixel caret rendering.
- **R3 - Object Pipeline (PowerShell Killer):** Integrate a single-header JSON parser. Commands output structured JSON, parsed in C, and rendered as beautiful interactive data grids/tables.
- **R5 - Embedded Database:** Drop `sqlite3.c` into the codebase. Log all commands, execution times, and outputs for instant semantic search.
- **R6 - Embedded Scripting:** Drop `lua.c` into the codebase. Users write Lua scripts to automate OS tasks and format CLI outputs.
