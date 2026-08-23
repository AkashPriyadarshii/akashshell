# Project Rules — akashshell

These are the rules that govern how akashshell gets built. They exist so that the project stays small, readable, and true to its teaching purpose. When a rule here and a habit from another project disagree, the rule here wins.

## Scope discipline

Version 0.1 is one C file, compiled with Microsoft's `cl` toolchain, and targeted at Windows 11 alone. Resist the urge to widen that. A feature that is not in the plan for 0.1 is a feature for 0.2, and pretending otherwise is how small projects turn into things nobody can finish.

## Dependencies

The project depends on nothing beyond the Windows API header `<windows.h>` and the C standard I/O header `<stdio.h>`. There is no third-party library, no build system, and no package manager in play. If a task seems to need a library, the first move is to ask whether the task actually belongs in this version at all.

## Built-ins versus external commands

The shell handles two commands internally: `cd`, which changes the working directory, and `exit`, which ends the session. Every other command is treated as an external program and launched through `CreateProcess`. This split matters because `cd` has to affect the shell's own state; you cannot change a directory by spawning a child process, since the change would die with that child. Keep that distinction clear in the code.

## Pipes

When the user writes `a | b`, the shell wires the standard output of `a` into the standard input of `b` using a manual chain of Windows pipe handles. The shell creates one pipe between each adjacent pair of commands and hands the right ends to `CreateProcess`. This is the one genuinely fiddly part of the program, and it is worth getting right once rather than approximating with `system`.

## History

Command history is collected during the session with a simple loop that reads each line, and it is written to the file `%USERPROFILE%\.akashshell_history` when the shell exits. The history lives in memory while the program runs and is flushed to disk at the end. There is no incremental append during the session in 0.1; that is fine for a single-user personal shell.

## What this rules file is not

It is not a spec. The product requirements document in `docs/` carries the what; this file carries the how-we-work guardrails. Keep the two separate so that each stays short.
