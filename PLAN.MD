# AKASHSHELL V0.1 — IMPLEMENTATION PLAN

## PHASE 1: SCAFFOLDING & FOUNDATION
- [x] Initialize Cargo workspace (`akashshell-core`, `akashshell-tui`, `akashshell-parser`).
- [x] Verify local Cargo compilation (Rust 1.96.0 installed on local laptop).
- [x] Define the core `Value` enum for structured data.

## PHASE 2: LEXER & PARSER
- [x] Implement the tokenizer (splitting strings, recognizing pipelines).
- [x] Build the AST structure.
- [x] Create basic internal built-ins (`echo`, `ls`, `cd`).

## PHASE 3: EVALUATION & PIPELINES
- [x] Implement the Evaluator to walk the AST.
- [x] Enable object passing through pipes (`|`).
- [x] Implement robust error handling (spans and clear terminal error messages).

## PHASE 4: WINDOWS PTY & EXTERNAL COMMANDS
- [x] Integrate Windows `ConPTY` to intercept and render external EXE outputs.
- [x] Ensure `Ctrl+C` and terminal resizing work correctly with external processes.

## PHASE 5: ELITE TUI
- [x] Implement the non-blocking event loop.
- [x] Add Fish-style history-based auto-suggestions.
- [x] Add real-time syntax highlighting for commands.

## PHASE 6: RELEASE v0.1
- [x] Compile release binary.
- [x] Final manual testing on Windows 10/11.
- [x] Cut v0.1 GitHub Release.
