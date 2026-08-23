# akashshell — C shell for Windows 11

This file holds the standing direction for the akashshell project. It is meant to be read by Claude Code (and by any human contributor) at the start of a session so that work stays aligned with the original intent. Think of it as the project's north star rather than a list of chores.

## The shape of version 0.1

The first release is a single C source file that implements a read-eval-print loop. It supports command piping, two built-in commands (`cd` and `exit`), and a persistent command history. That is the whole of it. Anything beyond that belongs to a later version and should not creep into 0.1.

## Where it goes next

Version 0.2 and later are expected to add arrow-key command recall, support for Linux and macOS, a small alias system, and environment variable expansion. These are noted here so that decisions made today do not quietly close the door on them tomorrow, but they are not part of the current work.

## How to build and run

The build is a one-liner because the program is a single file:

```bat
cl /O2 shell.c -o akashshell.exe
```

Run the resulting binary from any terminal. There is no installer and no configuration step in this version.

## Guiding principles

Keep it boring on purpose. The value of this project is that a person can understand all of it. Reach for the Windows API when you need to spawn a process or move bytes between handles, and reach for the C standard library for everything else. Do not pull in external dependencies to save a few lines; the few lines are not worth the opacity they buy.
