/*
 * akashshell v0.1 GUI - Elite Hardware-Accelerated Modern Terminal
 * Copyright (c) 2026 Akash Priyadarshi
 * License: MIT
 */

#define INITGUID
#define COBJMACROS
#define WIN32_LEAN_AND_MEAN
#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>
#include <d2d1.h>
#include <dwrite.h>
#include <dwmapi.h>
#include <commctrl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <process.h>
#include <math.h>

#ifndef DWMWA_USE_IMMERSIVE_DARK_MODE
#define DWMWA_USE_IMMERSIVE_DARK_MODE 20
#endif
#ifndef DWMWA_SYSTEMBACKDROP_TYPE
#define DWMWA_SYSTEMBACKDROP_TYPE 38
#endif
#ifndef DWMWA_WINDOW_CORNER_PREFERENCE
#define DWMWA_WINDOW_CORNER_PREFERENCE 33
#endif

#define MAX_HIST 256
#define MAX_LINE 1024
#define MAX_SEGS 64

#define WM_USER_OUTPUT_UPDATE (WM_USER + 1)
#define WM_USER_EXEC_DONE     (WM_USER + 2)

/* -------------------------------------------------------------------------
 * Data Structures
 * ------------------------------------------------------------------------- */

typedef struct CommandBlock {
    WCHAR command[MAX_LINE];
    WCHAR *output;
    size_t output_len;
    size_t output_cap;
    BOOL is_running;
    struct CommandBlock *next;
} CommandBlock;

CommandBlock *g_head = NULL;
CommandBlock *g_tail = NULL;

char g_cwd[MAX_PATH];
char g_history[MAX_HIST][MAX_LINE];
int g_hist_count = 0;
int g_hist_index = -1;

/* -------------------------------------------------------------------------
 * Globals
 * ------------------------------------------------------------------------- */

HWND g_hWndMain = NULL;
HWND g_hInput = NULL;
WNDPROC g_wpOrigInputProc = NULL;

ID2D1Factory *g_pD2DFactory = NULL;
ID2D1HwndRenderTarget *g_pRT = NULL;
IDWriteFactory *g_pDWriteFactory = NULL;
IDWriteTextFormat *g_pTextFormatCmd = NULL;
IDWriteTextFormat *g_pTextFormatOut = NULL;

ID2D1SolidColorBrush *g_pBrushBg = NULL;
ID2D1SolidColorBrush *g_pBrushBlock = NULL;
ID2D1SolidColorBrush *g_pBrushCmdText = NULL;
ID2D1SolidColorBrush *g_pBrushOutText = NULL;

float g_scrollY = 0.0f;
float g_maxScrollY = 0.0f;
float g_inputBarHeight = 50.0f;

/* -------------------------------------------------------------------------
 * Helpers
 * ------------------------------------------------------------------------- */

void append_block(const WCHAR *cmd) {
    CommandBlock *b = (CommandBlock*)calloc(1, sizeof(CommandBlock));
    wcsncpy(b->command, cmd, MAX_LINE - 1);
    b->output_cap = 4096;
    b->output = (WCHAR*)calloc(b->output_cap, sizeof(WCHAR));
    b->output_len = 0;
    b->is_running = TRUE;
    
    if (!g_head) {
        g_head = g_tail = b;
    } else {
        g_tail->next = b;
        g_tail = b;
    }
}

void append_block_output(CommandBlock *b, const WCHAR *text, size_t len) {
    if (!b || !text || len == 0) return;
    if (b->output_len + len + 1 >= b->output_cap) {
        b->output_cap *= 2;
        if (b->output_cap < b->output_len + len + 1)
            b->output_cap = b->output_len + len + 1024;
        b->output = (WCHAR*)realloc(b->output, b->output_cap * sizeof(WCHAR));
    }
    wcsncpy(b->output + b->output_len, text, len);
    b->output_len += len;
    b->output[b->output_len] = 0;
}

static int history_path(char *buf) {
    char home[MAX_PATH];
    if (!GetEnvironmentVariableA("USERPROFILE", home, MAX_PATH)) return 0;
    _snprintf(buf, MAX_PATH, "%s\\.akashshell_history", home);
    buf[MAX_PATH - 1] = '\0';
    return 1;
}

static void add_to_history(const char *line) {
    if (g_hist_count < MAX_HIST) {
        strncpy(g_history[g_hist_count++], line, MAX_LINE - 1);
    }
}

static void load_history() {
    char path[MAX_PATH];
    if (!history_path(path)) return;
    FILE *f = fopen(path, "r");
    if (!f) return;
    char line[MAX_LINE];
    while (fgets(line, sizeof(line), f)) {
        size_t len = strlen(line);
        while (len > 0 && (line[len-1] == '\n' || line[len-1] == '\r')) line[--len] = '\0';
        if (len > 0) add_to_history(line);
    }
    fclose(f);
}

static void save_history() {
    char path[MAX_PATH];
    if (!history_path(path)) return;
    FILE *f = fopen(path, "w");
    if (!f) return;
    for (int i = 0; i < g_hist_count; i++) {
        fprintf(f, "%s\n", g_history[i]);
    }
    fclose(f);
}

/* -------------------------------------------------------------------------
 * Direct2D & DirectWrite
 * ------------------------------------------------------------------------- */

void CreateDeviceIndependentResources() {
    D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &IID_ID2D1Factory, NULL, (void**)&g_pD2DFactory);
    DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, &IID_IDWriteFactory, (IUnknown**)&g_pDWriteFactory);

    IDWriteFactory_CreateTextFormat(g_pDWriteFactory, L"Cascadia Code", NULL,
        DWRITE_FONT_WEIGHT_BOLD, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL,
        15.0f, L"en-us", &g_pTextFormatCmd);
        
    IDWriteFactory_CreateTextFormat(g_pDWriteFactory, L"Cascadia Code", NULL,
        DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL,
        14.0f, L"en-us", &g_pTextFormatOut);
        
    IDWriteTextFormat_SetWordWrapping(g_pTextFormatCmd, DWRITE_WORD_WRAPPING_WRAP);
    IDWriteTextFormat_SetWordWrapping(g_pTextFormatOut, DWRITE_WORD_WRAPPING_WRAP);
}

void CreateDeviceResources(HWND hWnd) {
    if (g_pRT) return;

    RECT rc;
    GetClientRect(hWnd, &rc);
    D2D1_SIZE_U size = { rc.right - rc.left, rc.bottom - rc.top };
    
    D2D1_RENDER_TARGET_PROPERTIES rtp = {
        D2D1_RENDER_TARGET_TYPE_DEFAULT,
        { DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED },
        0.0f, 0.0f,
        D2D1_RENDER_TARGET_USAGE_NONE,
        D2D1_FEATURE_LEVEL_DEFAULT
    };
    
    D2D1_HWND_RENDER_TARGET_PROPERTIES hrtp = {
        hWnd, size, D2D1_PRESENT_OPTIONS_NONE
    };

    ID2D1Factory_CreateHwndRenderTarget(g_pD2DFactory, &rtp, &hrtp, &g_pRT);
    
    D2D1_COLOR_F bg = {0.05f, 0.07f, 0.09f, 1.0f}; // #0D1117
    ID2D1HwndRenderTarget_CreateSolidColorBrush(g_pRT, &bg, NULL, &g_pBrushBg);
    
    D2D1_COLOR_F block = {0.09f, 0.11f, 0.13f, 1.0f}; // #161B22
    ID2D1HwndRenderTarget_CreateSolidColorBrush(g_pRT, &block, NULL, &g_pBrushBlock);
    
    D2D1_COLOR_F cmdTxt = {0.34f, 0.84f, 0.99f, 1.0f}; // Cyan
    ID2D1HwndRenderTarget_CreateSolidColorBrush(g_pRT, &cmdTxt, NULL, &g_pBrushCmdText);
    
    D2D1_COLOR_F outTxt = {0.80f, 0.80f, 0.80f, 1.0f}; // Light Gray
    ID2D1HwndRenderTarget_CreateSolidColorBrush(g_pRT, &outTxt, NULL, &g_pBrushOutText);
}

void DiscardDeviceResources() {
    if (g_pBrushBg) { ID2D1SolidColorBrush_Release(g_pBrushBg); g_pBrushBg = NULL; }
    if (g_pBrushBlock) { ID2D1SolidColorBrush_Release(g_pBrushBlock); g_pBrushBlock = NULL; }
    if (g_pBrushCmdText) { ID2D1SolidColorBrush_Release(g_pBrushCmdText); g_pBrushCmdText = NULL; }
    if (g_pBrushOutText) { ID2D1SolidColorBrush_Release(g_pBrushOutText); g_pBrushOutText = NULL; }
    if (g_pRT) { ID2D1HwndRenderTarget_Release(g_pRT); g_pRT = NULL; }
}

float DrawTextLayout(IDWriteTextFormat *fmt, const WCHAR *text, size_t len, float x, float y, float width, ID2D1SolidColorBrush *brush) {
    if (!text || len == 0) return 0.0f;
    IDWriteTextLayout *pLayout = NULL;
    IDWriteFactory_CreateTextLayout(g_pDWriteFactory, text, (UINT32)len, fmt, width, 10000.0f, &pLayout);
    
    DWRITE_TEXT_METRICS tm;
    IDWriteTextLayout_GetMetrics(pLayout, &tm);
    
    D2D1_POINT_2F pt = { x, y };
    ID2D1HwndRenderTarget_DrawTextLayout(g_pRT, pt, pLayout, (ID2D1Brush*)brush, D2D1_DRAW_TEXT_OPTIONS_NONE);
    
    float height = tm.height;
    IDWriteTextLayout_Release(pLayout);
    return height;
}

void OnPaint(HWND hWnd) {
    CreateDeviceResources(hWnd);
    
    ID2D1HwndRenderTarget_BeginDraw(g_pRT);
    D2D1_COLOR_F clearColor = {0.05f, 0.07f, 0.09f, 1.0f};
    ID2D1HwndRenderTarget_Clear(g_pRT, &clearColor);
    
    RECT rc;
    GetClientRect(hWnd, &rc);
    float width = (float)(rc.right - rc.left);
    float height = (float)(rc.bottom - rc.top);
    
    float padding = 20.0f;
    float blockWidth = width - padding * 2.0f;
    float y = padding - g_scrollY;
    
    for (CommandBlock *b = g_head; b; b = b->next) {
        // Measure heights first
        IDWriteTextLayout *pCmdLayout = NULL;
        IDWriteFactory_CreateTextLayout(g_pDWriteFactory, b->command, (UINT32)wcslen(b->command), g_pTextFormatCmd, blockWidth - 30.0f, 10000.0f, &pCmdLayout);
        DWRITE_TEXT_METRICS ctm;
        IDWriteTextLayout_GetMetrics(pCmdLayout, &ctm);
        IDWriteTextLayout_Release(pCmdLayout);
        
        float outHeight = 0.0f;
        if (b->output_len > 0) {
            IDWriteTextLayout *pOutLayout = NULL;
            IDWriteFactory_CreateTextLayout(g_pDWriteFactory, b->output, (UINT32)b->output_len, g_pTextFormatOut, blockWidth - 30.0f, 10000.0f, &pOutLayout);
            DWRITE_TEXT_METRICS otm;
            IDWriteTextLayout_GetMetrics(pOutLayout, &otm);
            IDWriteTextLayout_Release(pOutLayout);
            outHeight = otm.height;
        }
        
        float blockHeight = ctm.height + (outHeight > 0 ? outHeight + 15.0f : 0.0f) + 30.0f;
        
        // Draw Rounded Rect
        D2D1_ROUNDED_RECT rr = { {padding, y, padding + blockWidth, y + blockHeight}, 8.0f, 8.0f };
        ID2D1HwndRenderTarget_FillRoundedRectangle(g_pRT, &rr, (ID2D1Brush*)g_pBrushBlock);
        
        // Draw Text
        DrawTextLayout(g_pTextFormatCmd, b->command, wcslen(b->command), padding + 15.0f, y + 15.0f, blockWidth - 30.0f, g_pBrushCmdText);
        if (b->output_len > 0) {
            DrawTextLayout(g_pTextFormatOut, b->output, b->output_len, padding + 15.0f, y + ctm.height + 25.0f, blockWidth - 30.0f, g_pBrushOutText);
        }
        
        y += blockHeight + padding;
    }
    
    // Bottom input bar styling
    float inputY = height - g_inputBarHeight;
    D2D1_RECT_F inputRect = { 0, inputY, width, height };
    ID2D1HwndRenderTarget_FillRectangle(g_pRT, &inputRect, (ID2D1Brush*)g_pBrushBlock);
    
    WCHAR cwdW[MAX_PATH];
    MultiByteToWideChar(CP_UTF8, 0, g_cwd, -1, cwdW, MAX_PATH);
    DrawTextLayout(g_pTextFormatCmd, cwdW, wcslen(cwdW), padding, inputY + 15.0f, width - padding*2, g_pBrushOutText);

    g_maxScrollY = y + g_scrollY - (height - g_inputBarHeight);
    if (g_maxScrollY < 0) g_maxScrollY = 0;

    HRESULT hr = ID2D1HwndRenderTarget_EndDraw(g_pRT, NULL, NULL);
    if (hr == D2DERR_RECREATE_TARGET) DiscardDeviceResources();
}

/* -------------------------------------------------------------------------
 * Execution Engine
 * ------------------------------------------------------------------------- */

static HANDLE make_inheritable(HANDLE h) {
    HANDLE dup = INVALID_HANDLE_VALUE;
    DuplicateHandle(GetCurrentProcess(), h, GetCurrentProcess(), &dup, 0, TRUE, DUPLICATE_SAME_ACCESS);
    return dup;
}

typedef struct {
    char line[MAX_LINE];
    CommandBlock *block;
} ExecParams;

DWORD WINAPI ExecThread(LPVOID lpParam) {
    ExecParams *params = (ExecParams *)lpParam;
    char *line = params->line;
    CommandBlock *block = params->block;

    // Handle Builtins
    char tmp[MAX_LINE];
    strncpy(tmp, line, MAX_LINE - 1);
    char *tok = strtok(tmp, " \t");
    
    if (tok && strcmp(tok, "exit") == 0) {
        PostMessage(g_hWndMain, WM_CLOSE, 0, 0);
        free(params);
        return 0;
    }
    if (tok && strcmp(tok, "cd") == 0) {
        char *dir = strtok(NULL, " \t");
        if (!dir || !*dir) {
            char home[MAX_PATH];
            if (GetEnvironmentVariableA("USERPROFILE", home, MAX_PATH)) dir = home;
        }
        if (SetCurrentDirectoryA(dir)) {
            GetCurrentDirectoryA(MAX_PATH, g_cwd);
        } else {
            WCHAR err[] = L"akashshell: cd: directory not found\r\n";
            append_block_output(block, err, wcslen(err));
        }
        block->is_running = FALSE;
        PostMessage(g_hWndMain, WM_USER_EXEC_DONE, 0, 0);
        free(params);
        return 0;
    }

    // Pipe Execution
    char *segs[MAX_SEGS];
    int n = 0; char *p = line;
    while (*p && n < MAX_SEGS) {
        while (*p == ' ' || *p == '\t') p++;
        if (!*p) break;
        segs[n++] = p;
        char *bar = strchr(p, '|');
        if (!bar) break;
        *bar = '\0'; p = bar + 1;
    }
    if (n == 0) { free(params); return 0; }

    HANDLE pipe_rd[MAX_SEGS], pipe_wr[MAX_SEGS];
    for (int i = 0; i < n - 1; i++) {
        SECURITY_ATTRIBUTES sa = { sizeof(sa), NULL, FALSE };
        CreatePipe(&pipe_rd[i], &pipe_wr[i], &sa, 0);
    }

    HANDLE master_rd, master_wr;
    SECURITY_ATTRIBUTES sa_master = { sizeof(sa_master), NULL, FALSE };
    CreatePipe(&master_rd, &master_wr, &sa_master, 0);

    HANDLE procs[MAX_SEGS];
    int nproc = 0;
    
    char comspec[MAX_PATH];
    if (!GetEnvironmentVariableA("ComSpec", comspec, MAX_PATH))
        strcpy(comspec, "C:\\Windows\\System32\\cmd.exe");

    for (int i = 0; i < n; i++) {
        // Trim
        size_t len = strlen(segs[i]);
        while(len>0 && (segs[i][len-1]==' ' || segs[i][len-1]=='\t')) segs[i][--len]='\0';

        HANDLE cIn = (i == 0) ? make_inheritable(GetStdHandle(STD_INPUT_HANDLE)) : make_inheritable(pipe_rd[i-1]);
        HANDLE cOut = (i == n - 1) ? make_inheritable(master_wr) : make_inheritable(pipe_wr[i]);

        char full[MAX_LINE + MAX_PATH + 10];
        _snprintf(full, sizeof(full), "\"%s\" /c %s", comspec, segs[i]);

        STARTUPINFOA si = {0}; si.cb = sizeof(si);
        si.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
        si.hStdInput = cIn; si.hStdOutput = cOut; si.hStdError = cOut;
        si.wShowWindow = SW_HIDE;

        PROCESS_INFORMATION pi;
        if (CreateProcessA(NULL, full, NULL, NULL, TRUE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi)) {
            procs[nproc++] = pi.hProcess;
            CloseHandle(pi.hThread);
        }
        CloseHandle(cIn); CloseHandle(cOut);
    }

    for (int i = 0; i < n - 1; i++) { CloseHandle(pipe_rd[i]); CloseHandle(pipe_wr[i]); }
    CloseHandle(master_wr);

    char buf[1024]; DWORD read;
    while (ReadFile(master_rd, buf, sizeof(buf) - 1, &read, NULL) && read > 0) {
        WCHAR wbuf[1024];
        int wlen = MultiByteToWideChar(CP_UTF8, 0, buf, read, wbuf, 1024);
        append_block_output(block, wbuf, wlen);
        PostMessage(g_hWndMain, WM_USER_OUTPUT_UPDATE, 0, 0);
    }
    CloseHandle(master_rd);

    for (int h = 0; h < nproc; h++) {
        WaitForSingleObject(procs[h], INFINITE);
        CloseHandle(procs[h]);
    }

    block->is_running = FALSE;
    PostMessage(g_hWndMain, WM_USER_EXEC_DONE, 0, 0);
    free(params);
    return 0;
}

/* -------------------------------------------------------------------------
 * Window Procs
 * ------------------------------------------------------------------------- */

LRESULT CALLBACK InputProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    if (uMsg == WM_KEYDOWN) {
        if (wParam == VK_RETURN) {
            char line[MAX_LINE];
            GetWindowTextA(hWnd, line, MAX_LINE);
            SetWindowTextA(hWnd, "");
            
            size_t len = strlen(line);
            while(len>0 && (line[len-1]=='\n' || line[len-1]=='\r')) line[--len]='\0';
            if (len == 0) return 0;

            add_to_history(line);
            g_hist_index = g_hist_count;

            WCHAR wcmd[MAX_LINE];
            MultiByteToWideChar(CP_UTF8, 0, line, -1, wcmd, MAX_LINE);
            append_block(wcmd);

            ExecParams *p = malloc(sizeof(ExecParams));
            strncpy(p->line, line, MAX_LINE);
            p->block = g_tail;
            CreateThread(NULL, 0, ExecThread, p, 0, NULL);
            
            InvalidateRect(g_hWndMain, NULL, FALSE);
            return 0;
        } else if (wParam == VK_UP) {
            if (g_hist_index > 0) {
                g_hist_index--;
                SetWindowTextA(hWnd, g_history[g_hist_index]);
                SendMessage(hWnd, EM_SETSEL, strlen(g_history[g_hist_index]), strlen(g_history[g_hist_index]));
            }
            return 0;
        } else if (wParam == VK_DOWN) {
            if (g_hist_index < g_hist_count - 1) {
                g_hist_index++;
                SetWindowTextA(hWnd, g_history[g_hist_index]);
                SendMessage(hWnd, EM_SETSEL, strlen(g_history[g_hist_index]), strlen(g_history[g_hist_index]));
            } else {
                g_hist_index = g_hist_count;
                SetWindowTextA(hWnd, "");
            }
            return 0;
        }
    }
    return CallWindowProc(g_wpOrigInputProc, hWnd, uMsg, wParam, lParam);
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_CREATE: {
        BOOL val = TRUE;
        DwmSetWindowAttribute(hWnd, DWMWA_USE_IMMERSIVE_DARK_MODE, &val, sizeof(val));
        int mica = 2, round = 2;
        DwmSetWindowAttribute(hWnd, DWMWA_SYSTEMBACKDROP_TYPE, &mica, sizeof(mica));
        DwmSetWindowAttribute(hWnd, DWMWA_WINDOW_CORNER_PREFERENCE, &round, sizeof(round));

        CreateDeviceIndependentResources();
        
        load_history();
        GetCurrentDirectoryA(MAX_PATH, g_cwd);
        
        g_hInput = CreateWindowExA(0, "EDIT", "",
            WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL,
            0, 0, 0, 0, hWnd, NULL, NULL, NULL);
            
        HFONT hFont = CreateFontA(20, 0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET,
            0, 0, CLEARTYPE_QUALITY, FIXED_PITCH, "Cascadia Code");
        SendMessage(g_hInput, WM_SETFONT, (WPARAM)hFont, TRUE);
        
        g_wpOrigInputProc = (WNDPROC)SetWindowLongPtr(g_hInput, GWLP_WNDPROC, (LONG_PTR)InputProc);
        SetFocus(g_hInput);
        
        WCHAR welcome[] = L"akashshell v0.1 - Hardware Accelerated Block UI";
        append_block(welcome);
        WCHAR out[] = L"Ready.";
        append_block_output(g_tail, out, wcslen(out));
        g_tail->is_running = FALSE;
        break;
    }
    case WM_PAINT: {
        PAINTSTRUCT ps;
        BeginPaint(hWnd, &ps);
        OnPaint(hWnd);
        EndPaint(hWnd, &ps);
        break;
    }
    case WM_SIZE: {
        if (g_pRT) {
            D2D1_SIZE_U size = { LOWORD(lParam), HIWORD(lParam) };
            ID2D1HwndRenderTarget_Resize(g_pRT, &size);
        }
        
        // Measure CWD text width to offset input box
        WCHAR cwdW[MAX_PATH];
        MultiByteToWideChar(CP_UTF8, 0, g_cwd, -1, cwdW, MAX_PATH);
        
        IDWriteTextLayout *pLayout = NULL;
        IDWriteFactory_CreateTextLayout(g_pDWriteFactory, cwdW, wcslen(cwdW), g_pTextFormatCmd, 1000.0f, 100.0f, &pLayout);
        DWRITE_TEXT_METRICS tm;
        IDWriteTextLayout_GetMetrics(pLayout, &tm);
        IDWriteTextLayout_Release(pLayout);
        
        float inputOffset = 20.0f + tm.width + 10.0f;
        MoveWindow(g_hInput, (int)inputOffset, HIWORD(lParam) - 35, LOWORD(lParam) - (int)inputOffset - 20, 25, TRUE);
        
        InvalidateRect(hWnd, NULL, FALSE);
        break;
    }
    case WM_CTLCOLOREDIT: {
        HDC hdc = (HDC)wParam;
        SetTextColor(hdc, RGB(204, 204, 204));
        SetBkColor(hdc, RGB(22, 27, 34));
        static HBRUSH hbr = NULL;
        if(!hbr) hbr = CreateSolidBrush(RGB(22, 27, 34));
        return (LRESULT)hbr;
    }
    case WM_MOUSEWHEEL: {
        int zDelta = GET_WHEEL_DELTA_WPARAM(wParam);
        g_scrollY -= (float)zDelta * 0.5f;
        if (g_scrollY < 0) g_scrollY = 0;
        if (g_scrollY > g_maxScrollY) g_scrollY = g_maxScrollY;
        InvalidateRect(hWnd, NULL, FALSE);
        break;
    }
    case WM_USER_OUTPUT_UPDATE:
    case WM_USER_EXEC_DONE:
        g_scrollY = g_maxScrollY; // Auto-scroll
        InvalidateRect(hWnd, NULL, FALSE);
        break;
    case WM_DESTROY:
        save_history();
        DiscardDeviceResources();
        if(g_pTextFormatCmd) IDWriteTextFormat_Release(g_pTextFormatCmd);
        if(g_pTextFormatOut) IDWriteTextFormat_Release(g_pTextFormatOut);
        if(g_pDWriteFactory) IDWriteFactory_Release(g_pDWriteFactory);
        if(g_pD2DFactory) ID2D1Factory_Release(g_pD2DFactory);
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

int APIENTRY WinMain(HINSTANCE hInst, HINSTANCE hPrev, LPSTR cmd, int show) {
    (void)hPrev; (void)cmd;
    
    WNDCLASSEXA wc = {0};
    wc.cbSize = sizeof(wc);
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInst;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.lpszClassName = "AkashShellElite";
    RegisterClassExA(&wc);

    g_hWndMain = CreateWindowExA(0, wc.lpszClassName, "akashshell (Hardware Accelerated)", 
        WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 1024, 768, NULL, NULL, hInst, NULL);
        
    ShowWindow(g_hWndMain, show);
    UpdateWindow(g_hWndMain);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return (int)msg.wParam;
}
