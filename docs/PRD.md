# Product Requirements Document (PRD) - akashshell v0.1

## Vision
To build the most performant, visually stunning, single-file Windows shell and terminal emulator possible using pure C and zero third-party dependencies.

## Target Audience
- Single-user optimization (Akash Priyadarshi).
- Windows 11 power users seeking ultra-low latency and modern UX without the bloat of web-based terminals.

## Core Requirements (v0.1 Complete)
- **R1 - Hardware-Accelerated GUI:** Implemented via Direct2D (`ID2D1HwndRenderTarget`) and DirectWrite. No GDI, no standard Win32 `EDIT` controls.
- **R2 - Block-Based UI Paradigm:** Output is structured as a linked list of visual "blocks" (Command + Output), moving away from the legacy infinite-scroll text buffer.
- **R3 - Async Pipeline Engine:** Commands are dispatched to a background worker thread. Stdout is captured via anonymous pipes and pushed to the UI thread via `PostMessage` without blocking the 60fps render loop.
- **R4 - Pipe Chains:** `a | b | c` works flawlessly via manual `DuplicateHandle` and `CreatePipe` routing.
- **R5 - Built-in Commands:** `cd` (changes context in engine) and `exit` (flushes state and terminates).
- **R6 - Command History:** Memory-based history array, navigated via Up/Down arrow keys in the GUI, flushed to `%USERPROFILE%\.akashshell_history` on exit.
- **R7 - DWM Integration:** Windows 11 Dark Mode (20), Mica Backdrop (38), and Rounded Corners (33) applied via `DwmSetWindowAttribute`.

## Future Scope (v0.2+)
- Environment variable expansion (`$VAR`).
- Syntax highlighting within the input bar.
- Configurable settings panel (fonts, colors, padding).
