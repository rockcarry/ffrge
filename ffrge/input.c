#include <stdint.h>
#include <windows.h>
#include "input.h"

#ifndef WM_MOUSEWHEEL
#define WM_MOUSEWHEEL   0x020A
#endif

extern HWND g_ffrge_hwnd;
extern LRESULT (CALLBACK *g_ffrge_input_wndproc)(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

struct {
    INPUTCALLBACK callback;
    void         *cbctxt;
    uint32_t      keybtn_states[256 / 32];
} s_input_context = {0};

static void def_input_callback(void *cbctxt, int type, int val1, int val2, int val3) {}

static LRESULT CALLBACK input_wndproc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    int key = 0, down = 0;
    switch (uMsg) {
    case WM_TIMER  : s_input_context.callback(s_input_context.cbctxt, INPUT_EVENT_TIMER, (int)wParam, 0, 0); break;
    case WM_KEYUP  : case WM_SYSKEYUP  : s_input_context.callback(s_input_context.cbctxt, INPUT_EVENT_KEY, (int)wParam, 0, 0); key = (int)wParam; break;
    case WM_KEYDOWN: case WM_SYSKEYDOWN: s_input_context.callback(s_input_context.cbctxt, INPUT_EVENT_KEY, (int)wParam, 1, 0); key = (int)wParam; down = 1; break;
    case WM_MOUSEMOVE  : s_input_context.callback(s_input_context.cbctxt, INPUT_EVENT_MOUSE_MOVE  , (int)((lParam >> 0) & 0xFFFF), (int)((lParam >> 16) & 0xFFFF), (int)wParam); break;
    case WM_LBUTTONUP  : s_input_context.callback(s_input_context.cbctxt, INPUT_EVENT_LBUTTON_UP  , (int)((lParam >> 0) & 0xFFFF), (int)((lParam >> 16) & 0xFFFF), (int)wParam); break;
    case WM_LBUTTONDOWN: s_input_context.callback(s_input_context.cbctxt, INPUT_EVENT_LBUTTON_DOWN, (int)((lParam >> 0) & 0xFFFF), (int)((lParam >> 16) & 0xFFFF), (int)wParam); break;
    case WM_MBUTTONUP  : s_input_context.callback(s_input_context.cbctxt, INPUT_EVENT_MBUTTON_UP  , (int)((lParam >> 0) & 0xFFFF), (int)((lParam >> 16) & 0xFFFF), (int)wParam); break;
    case WM_MBUTTONDOWN: s_input_context.callback(s_input_context.cbctxt, INPUT_EVENT_MBUTTON_DOWN, (int)((lParam >> 0) & 0xFFFF), (int)((lParam >> 16) & 0xFFFF), (int)wParam); break;
    case WM_RBUTTONUP  : s_input_context.callback(s_input_context.cbctxt, INPUT_EVENT_RBUTTON_UP  , (int)((lParam >> 0) & 0xFFFF), (int)((lParam >> 16) & 0xFFFF), (int)wParam); break;
    case WM_RBUTTONDOWN: s_input_context.callback(s_input_context.cbctxt, INPUT_EVENT_RBUTTON_DOWN, (int)((lParam >> 0) & 0xFFFF), (int)((lParam >> 16) & 0xFFFF), (int)wParam); break;
    case WM_MOUSEWHEEL : s_input_context.callback(s_input_context.cbctxt, INPUT_EVENT_MOUSE_WHELL , (int)wParam, 0, 0); break;
    }
    if (key) {
        if (down) s_input_context.keybtn_states[key / 32] |= (1 << (key % 32));
        else      s_input_context.keybtn_states[key / 32] &=~(1 << (key % 32));
    }
    return 0;
}

void input_init(INPUTCALLBACK callback, void *cbctxt)
{
    s_input_context.callback = callback ? callback : def_input_callback;
    s_input_context.cbctxt   = callback ? cbctxt   : NULL;
    g_ffrge_input_wndproc    = input_wndproc;
}

void input_exit(void)
{
    s_input_context.callback = NULL;
    s_input_context.cbctxt   = NULL;
    g_ffrge_input_wndproc    = NULL;
}

void input_settimer(int id, int interval)
{
    if (g_ffrge_hwnd) {
        if (interval) SetTimer (g_ffrge_hwnd, id, interval, NULL);
        else          KillTimer(g_ffrge_hwnd, id);
    }
}

int input_getkey(int key)
{
    if (g_ffrge_hwnd == NULL || key <= 0 || key >= 256) return 0;
    return !!(s_input_context.keybtn_states[key / 32] & (1 << (key %32)));
}

void input_getmouse(int *x, int *y, int *btns)
{
    if (g_ffrge_hwnd) {
        POINT point;
        GetCursorPos  (&point);
        ScreenToClient(g_ffrge_hwnd, &point);
        if (x   ) *x = point.x;
        if (y   ) *y = point.y;
        if (btns) {
            *btns  = (GetAsyncKeyState(VK_LBUTTON) & 0x8000) ? (1 << 0): 0;
            *btns |= (GetAsyncKeyState(VK_RBUTTON) & 0x8000) ? (1 << 1): 0;
            *btns |= (GetAsyncKeyState(VK_MBUTTON) & 0x8000) ? (1 << 4): 0;
        }
    }
}

void input_setmouse(int x, int y)
{
    if (g_ffrge_hwnd) {
        POINT point = { x, y };
        ClientToScreen(g_ffrge_hwnd, &point);
        SetCursorPos(point.x, point.y);
    }
}
