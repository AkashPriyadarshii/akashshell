/*
 * akashshell v0.1 - a minimal C shell for Windows 11
 *
 * Copyright (c) 2026 Akash Priyadarshi
 * License: MIT (see end of file)
 *
 * Features
 *   - Built-ins: cd, exit
 *   - External commands via CreateProcess (routed through cmd /c)
 *   - Pipe chains: a | b | c
 *   - History persisted to %USERPROFILE%\.akashshell_history
 *
 * Build (GCC/MinGW):
 *     gcc -O2 -Wall shell.c -lshell32 -o akashshell.exe
 *
 * Build (MSVC):
 *     cl /O2 shell.c /link shell32.lib /out:akashshell.exe
 */

#define _CRT_SECURE_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <shlobj.h>

#define MAX_HIST  256
#define MAX_LINE  1024
#define MAX_SEGS   64

/* -------------------------------------------------------------------------
 * History
 * ------------------------------------------------------------------------- */

static char g_history[MAX_HIST][MAX_LINE];
static int  g_hist_count = 0;

static int history_path(char *buf)
{
    char home[MAX_PATH];
    if (!SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_PROFILE, NULL, 0, home)))
        return 0;
    _snprintf(buf, MAX_PATH, "%s\\.akashshell_history", home);
    buf[MAX_PATH - 1] = '\0';
    return 1;
}

static void load_history(void)
{
    char path[MAX_PATH];
    if (!history_path(path)) return;

    FILE *f = fopen(path, "r");
    if (!f) return;

    char buf[MAX_LINE];
    while (fgets(buf, MAX_LINE, f) && g_hist_count < MAX_HIST)
    {
        size_t len = strlen(buf);
        while (len > 0 && (buf[len-1] == '\n' || buf[len-1] == '\r'))
            buf[--len] = '\0';
        if (len > 0)
        {
            strncpy(g_history[g_hist_count], buf, MAX_LINE - 1);
            g_history[g_hist_count++][MAX_LINE - 1] = '\0';
        }
    }
    fclose(f);
}

static void add_to_history(const char *line)
{
    if (g_hist_count < MAX_HIST)
    {
        strncpy(g_history[g_hist_count], line, MAX_LINE - 1);
        g_history[g_hist_count++][MAX_LINE - 1] = '\0';
    }
}

static void save_history(void)
{
    char path[MAX_PATH];
    if (!history_path(path)) return;

    FILE *f = fopen(path, "w");
    if (!f) return;

    for (int i = 0; i < g_hist_count; i++)
        fprintf(f, "%s\n", g_history[i]);
    fclose(f);
}

/* -------------------------------------------------------------------------
 * Pipe-splitting
 * ------------------------------------------------------------------------- */

static int split_pipe(char *line, char *segs[], int max_segs)
{
    int n = 0;
    char *p = line;

    while (*p && n < max_segs)
    {
        /* skip leading whitespace */
        while (*p == ' ' || *p == '\t') p++;
        if (*p == '\0') break;

        segs[n++] = p;

        char *bar = strchr(p, '|');
        if (!bar) break;

        *bar = '\0';
        p = bar + 1;
    }
    return n;
}

static void rtrim(char *s)
{
    size_t n = strlen(s);
    while (n > 0 && (s[n-1] == ' ' || s[n-1] == '\t'))
        s[--n] = '\0';
}

/* -------------------------------------------------------------------------
 * Inheritable handle duplication helper
 *
 * CreatePipe by default creates both ends inheritable.  We need surgical
 * control: only ONE end should be inheritable for each child.
 *
 * Strategy:
 *   - Create the pipe with bInheritHandle = FALSE (neither end inheritable).
 *   - Duplicate only the end we want a specific child to inherit, producing
 *     a new inheritable handle.
 *   - Pass the duplicate to the child via STARTUPINFO.
 *   - Close the duplicate after CreateProcess returns (child owns the copy).
 * ------------------------------------------------------------------------- */

static HANDLE make_inheritable(HANDLE h)
{
    HANDLE dup = INVALID_HANDLE_VALUE;
    if (!DuplicateHandle(GetCurrentProcess(), h,
                         GetCurrentProcess(), &dup,
                         0, TRUE,               /* bInheritHandle = TRUE */
                         DUPLICATE_SAME_ACCESS))
        return INVALID_HANDLE_VALUE;
    return dup;
}

/* -------------------------------------------------------------------------
 * Single-stage spawner
 *
 * Wraps `cmdline` with  cmd.exe /c  so Windows builtins work.
 * stdin_h / stdout_h are already-inheritable handles that the child will use.
 * ------------------------------------------------------------------------- */

static BOOL spawn_stage(const char *cmdline,
                        HANDLE stdin_h, HANDLE stdout_h,
                        HANDLE *proc_out)
{
    char comspec[MAX_PATH];
    if (!GetEnvironmentVariableA("ComSpec", comspec, MAX_PATH))
        strncpy(comspec, "C:\\Windows\\System32\\cmd.exe", MAX_PATH - 1);

    char full[MAX_LINE + MAX_PATH + 10];
    _snprintf(full, sizeof(full), "\"%s\" /c %s", comspec, cmdline);
    full[sizeof(full) - 1] = '\0';

    STARTUPINFOA si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    si.cb         = sizeof(si);
    si.dwFlags    = STARTF_USESTDHANDLES;
    si.hStdInput  = stdin_h;
    si.hStdOutput = stdout_h;
    si.hStdError  = GetStdHandle(STD_ERROR_HANDLE);

    /* bInheritHandles = TRUE so the handles in si are inherited. */
    BOOL ok = CreateProcessA(NULL, full, NULL, NULL,
                             TRUE,   /* bInheritHandles */
                             0, NULL, NULL, &si, &pi);
    if (!ok)
    {
        fprintf(stderr, "akashshell: %s: command not found\n", cmdline);
        return FALSE;
    }

    *proc_out = pi.hProcess;
    CloseHandle(pi.hThread);
    return TRUE;
}

/* -------------------------------------------------------------------------
 * Executor — handles pipes + builtins
 * ------------------------------------------------------------------------- */

static int execute(char *line)
{
    /* --- built-in: exit -------------------------------------------------- */
    {
        char tmp[MAX_LINE];
        strncpy(tmp, line, MAX_LINE - 1); tmp[MAX_LINE-1] = '\0';
        char *tok = strtok(tmp, " \t");
        if (tok && strcmp(tok, "exit") == 0) return 1;
    }

    /* --- built-in: cd ---------------------------------------------------- */
    {
        char tmp[MAX_LINE];
        strncpy(tmp, line, MAX_LINE - 1); tmp[MAX_LINE-1] = '\0';
        char *tok = strtok(tmp, " \t");
        if (tok && strcmp(tok, "cd") == 0)
        {
            char *dir = strtok(NULL, " \t");
            if (!dir || !*dir)
            {
                char home[MAX_PATH];
                if (SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_PROFILE,
                                               NULL, 0, home)))
                    dir = home;
                else
                    dir = ".";
            }
            if (!SetCurrentDirectoryA(dir))
                fprintf(stderr,
                    "akashshell: cd: %s: No such file or directory\n", dir);
            return 0;
        }
    }

    /* --- pipe chain ------------------------------------------------------ */

    char *segs[MAX_SEGS];
    int nseg = split_pipe(line, segs, MAX_SEGS);
    if (nseg == 0) return 0;

    /*
     * Pipe handle strategy (the correct Windows way):
     *
     * For each adjacent pair (i, i+1) we create a pipe whose handles are
     * NOT inheritable by default.  Then we duplicate only the write end
     * for child i (making it inheritable) and only the read end for child
     * i+1 (making it inheritable).  After each CreateProcess we immediately
     * close the duplicated handle — the child retains its copy.
     *
     * This ensures no child accidentally inherits a pipe end it should not
     * hold, which is the root cause of "findstr hangs forever" and
     * "nonexistent pipe" errors.
     */

    /* Pre-create all pipes so we can set up each stage cleanly. */
    HANDLE pipe_rd[MAX_SEGS]; /* pipe_rd[i] = read end between stage i and i+1 */
    HANDLE pipe_wr[MAX_SEGS]; /* pipe_wr[i] = write end between stage i and i+1 */

    int npipes = nseg - 1;
    for (int p = 0; p < npipes; p++)
    {
        SECURITY_ATTRIBUTES sa;
        sa.nLength              = sizeof(sa);
        sa.lpSecurityDescriptor = NULL;
        sa.bInheritHandle       = FALSE; /* NOT inheritable by default */

        if (!CreatePipe(&pipe_rd[p], &pipe_wr[p], &sa, 0))
        {
            fprintf(stderr, "akashshell: CreatePipe failed at stage %d\n", p);
            /* Clean up pipes already created. */
            for (int q = 0; q < p; q++)
            {
                CloseHandle(pipe_rd[q]);
                CloseHandle(pipe_wr[q]);
            }
            return 0;
        }
    }

    HANDLE procs[MAX_SEGS];
    int    nproc = 0;

    for (int i = 0; i < nseg; i++)
    {
        rtrim(segs[i]);

        /* Determine stdin handle for this stage. */
        HANDLE child_stdin_inh;
        if (i == 0)
        {
            /* First stage reads from shell's real stdin. */
            child_stdin_inh = make_inheritable(GetStdHandle(STD_INPUT_HANDLE));
        }
        else
        {
            /* Subsequent stages read from the previous pipe's read end. */
            child_stdin_inh = make_inheritable(pipe_rd[i-1]);
        }

        /* Determine stdout handle for this stage. */
        HANDLE child_stdout_inh;
        if (i == nseg - 1)
        {
            /* Last stage writes to shell's real stdout. */
            child_stdout_inh = make_inheritable(GetStdHandle(STD_OUTPUT_HANDLE));
        }
        else
        {
            /* Earlier stages write into the next pipe's write end. */
            child_stdout_inh = make_inheritable(pipe_wr[i]);
        }

        HANDLE proc;
        BOOL ok = spawn_stage(segs[i], child_stdin_inh, child_stdout_inh, &proc);

        /* Close the inheritable duplicates immediately — child holds its own
         * copies via handle inheritance. */
        CloseHandle(child_stdin_inh);
        CloseHandle(child_stdout_inh);

        if (ok) procs[nproc++] = proc;
    }

    /* Close all pipe handles we own — children hold the inherited copies.
     * We must close these BEFORE waiting, otherwise the read end stays open
     * in our process and the next-stage child never sees EOF. */
    for (int p = 0; p < npipes; p++)
    {
        CloseHandle(pipe_rd[p]);
        CloseHandle(pipe_wr[p]);
    }

    /* Wait for all children. */
    for (int h = 0; h < nproc; h++)
    {
        WaitForSingleObject(procs[h], INFINITE);
        CloseHandle(procs[h]);
    }

    return 0;
}

/* -------------------------------------------------------------------------
 * REPL
 * ------------------------------------------------------------------------- */

int main(void)
{
    load_history();
    printf("akashshell v0.1  (type 'exit' to quit)\n");

    char line[MAX_LINE];

    for (;;)
    {
        char cwd[MAX_PATH];
        if (GetCurrentDirectoryA(MAX_PATH, cwd))
            printf("%s> ", cwd);
        else
            printf("akashshell> ");
        fflush(stdout);

        if (!fgets(line, sizeof(line), stdin))
            break;

        size_t len = strlen(line);
        while (len > 0 && (line[len-1] == '\n' || line[len-1] == '\r'))
            line[--len] = '\0';

        if (len == 0) continue;

        add_to_history(line);

        if (execute(line) != 0)
            break;
    }

    save_history();
    printf("\nBye.\n");
    return 0;
}

/*
 * MIT License
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
