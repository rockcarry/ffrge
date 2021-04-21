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
    int           mousex, mousey;
} s_input_context = {0};

static LRESULT CALLBACK input_wndproc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    int key = 0, down = 0;
    if (s_input_context.callback == NULL) return -1;
    switch (uMsg) {
    case WM_TIMER:
        s_input_context.callback(s_input_context.cbctxt, INPUT_TYPE_TIMER, (int)wParam, 0);
        break;
    case WM_KEYUP  : case WM_SYSKEYUP  :
        s_input_context.callback(s_input_context.cbctxt, INPUT_TYPE_KEY, (int)wParam, 0);
        key = (int)wParam;
        break;
    case WM_KEYDOWN: case WM_SYSKEYDOWN:
        s_input_context.callback(s_input_context.cbctxt, INPUT_TYPE_KEY, (int)wParam, 1);
        key = (int)wParam; down = 1;
        break;
    case WM_LBUTTONUP:
        s_input_context.callback(s_input_context.cbctxt, INPUT_TYPE_KEY, INPUT_KEY_LBUTTON, 0);
        key = INPUT_KEY_LBUTTON;
        break;
    case WM_LBUTTONDOWN:
        s_input_context.callback(s_input_context.cbctxt, INPUT_TYPE_KEY, INPUT_KEY_LBUTTON, 1);
        key = INPUT_KEY_LBUTTON; down = 1;
        break;
    case WM_RBUTTONUP:
        s_input_context.callback(s_input_context.cbctxt, INPUT_TYPE_KEY, INPUT_KEY_RBUTTON, 0);
        key = INPUT_KEY_RBUTTON;
        break;
    case WM_RBUTTONDOWN:
        s_input_context.callback(s_input_context.cbctxt, INPUT_TYPE_KEY, INPUT_KEY_RBUTTON, 1);
        key = INPUT_KEY_RBUTTON; down = 1;
        break;
    case WM_MBUTTONUP:
        s_input_context.callback(s_input_context.cbctxt, INPUT_TYPE_KEY, INPUT_KEY_MBUTTON, 0);
        key = INPUT_KEY_MBUTTON;
        break;
    case WM_MBUTTONDOWN:
        s_input_context.callback(s_input_context.cbctxt, INPUT_TYPE_KEY, INPUT_KEY_MBUTTON, 1);
        key = INPUT_KEY_MBUTTON; down = 1;
        break;
    case WM_MOUSEMOVE:
        s_input_context.callback(s_input_context.cbctxt, INPUT_TYPE_MOUSE, (int)((lParam >> 0) & 0xFFFF), (int)((lParam >> 16) & 0xFFFF));
        break;
    case WM_MOUSEWHEEL:
        s_input_context.callback(s_input_context.cbctxt, INPUT_TYPE_WHEEL, (int)wParam, 0);
        break;
    }
    if (key) {
        if (down) s_input_context.keybtn_states[key / 32] |= (1 << (key % 32));
        else      s_input_context.keybtn_states[key / 32] &=~(1 << (key % 32));
    }
    return 0;
}

void input_init(INPUTCALLBACK callback, void *cbctxt)
{
    s_input_context.callback = callback;
    s_input_context.cbctxt   = callback ? cbctxt        : NULL;
    g_ffrge_input_wndproc    = callback ? input_wndproc : NULL;
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
    if (g_ffrge_hwnd == NULL) return -1;
    if (key >= 1 && key <= 255) {
        return !!(s_input_context.keybtn_states[key / 32] & (1 << (key %32)));
    } else return 0;
}

void input_getmouse(int *x, int *y)
{
    if (g_ffrge_hwnd) {
        POINT point;
        GetCursorPos  (&point);
        ScreenToClient(g_ffrge_hwnd, &point);
        if (x) *x = point.x;
        if (y) *y = point.y;
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
