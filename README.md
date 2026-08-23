# akashshell ⚡

**A blazing-fast, minimalist, hardware-accelerated Windows 11 terminal emulator and shell written entirely in pure C.**

![Platform](https://img.shields.io/badge/Platform-Windows_11-0078D4?style=for-the-badge&logo=windows)
![Language](https://img.shields.io/badge/Language-C-A8B9CC?style=for-the-badge&logo=c)
![Rendering](https://img.shields.io/badge/Rendering-Direct2D_|_DirectWrite-FFB900?style=for-the-badge)
![Dependencies](https://img.shields.io/badge/Dependencies-Zero-success?style=for-the-badge)

`akashshell` is a ground-up reimagining of what a modern shell frontend can be on Windows. Abandoning bloated web frameworks (Electron) and heavy UI libraries, it uses **pure Win32 C, Direct2D, and DirectWrite** to deliver an elite-grade, zero-dependency terminal experience.

## ✨ Elite Features (v0.1)

*   🚀 **Hardware-Accelerated Rendering:** Bypasses GDI entirely. Uses `ID2D1HwndRenderTarget` for GPU-accelerated drawing at 60fps.
*   🖋️ **Sub-pixel Typography:** Integrates `IDWriteTextFormat` with *Cascadia Code* for flawless, anti-aliased font rendering identical to VS Code and Windows Terminal.
*   📦 **Warp-Style Block UI Paradigm:** Every command is visually isolated in a beautiful, rounded-corner data block. No more massive walls of scrolling text—each execution and its output are visually distinct.
*   ⚡ **Asynchronous Execution Engine:** Built-in multi-threading isolates the UI from the execution plane. Massive stdout streams are piped and captured asynchronously without dropping a single frame of UI performance.
*   🔗 **Native Command Piping:** Flawless Windows pipe handle inheritance (`a | b | c`) handled manually in C without relying on `system()`.
*   🌙 **Modern Windows 11 Integration:** Native DWM Immersive Dark Mode, Mica backdrops, and rounded window corners straight out of the box.

## 🛠️ Architecture

`akashshell` draws heavy architectural inspiration from modern titans like **Ghostty** and **Warp**:
*   **State Isolation:** Like Ghostty, the execution engine runs entirely separated from the OS UI controls.
*   **Data Structures:** Like Warp, commands are parsed into distinct `CommandBlock` linked-list nodes, allowing targeted UI rendering and precise memory isolation.

See `docs/PRD.md` and `docs/ARCHITECTURE.mmd` for an in-depth breakdown of the pipe architecture and rendering loop.

## 🚀 Building & Running

You need a C compiler that supports the Windows API (GCC via llvm-mingw or MSVC).

**Build the Elite Direct2D GUI (v0.1):**
```bat
gcc -O2 -Wall -Wextra gui.c -lshell32 -ldwmapi -lcomctl32 -lgdi32 -ld2d1 -ldwrite -lole32 -luuid -mwindows -o akashshell_gui.exe
```

**Build the Minimalist CLI (v0.1):**
```bat
gcc -O2 -Wall shell.c -lshell32 -o akashshell.exe
```

## 🧠 Why?
Built for Akash Priyadarshi as an extreme exercise in achieving maximum performance and modern UX within the absolute tightest constraints (one C file, zero 3rd-party dependencies). 

**Keywords:** `Windows 11`, `Terminal Emulator`, `Direct2D`, `Hardware-Accelerated`, `Win32 API`, `C Shell`, `Block UI`, `Warp Alternative`, `Zero Dependencies`, `Ghostty`.
