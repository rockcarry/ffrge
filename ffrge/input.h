#ifndef __RGE_INPUT_H__
#define __RGE_INPUT_H__

#ifdef __cplusplus
extern "C" {
#endif

enum {
    INPUT_EVENT_TIMER = 1,   // val1 - timer id
    INPUT_EVENT_KEY,         // val1 - keycode, val2 - key state
    INPUT_EVENT_MOUSE_MOVE,  // val1 - x, val2 - y, val3 - button state
    INPUT_EVENT_LBUTTON_DOWN,// val1 - x, val2 - y, val3 - button state
    INPUT_EVENT_LBUTTON_UP,  // val1 - x, val2 - y, val3 - button state
    INPUT_EVENT_MBUTTON_DOWN,// val1 - x, val2 - y, val3 - button state
    INPUT_EVENT_MBUTTON_UP,  // val1 - x, val2 - y, val3 - button state
    INPUT_EVENT_RBUTTON_DOWN,// val1 - x, val2 - y, val3 - button state
    INPUT_EVENT_RBUTTON_UP,  // val1 - x, val2 - y, val3 - button state
    INPUT_EVENT_MOUSE_WHELL, // val1 - wheel value
};

enum {
    INPUT_KEY_BACKSPACE = 0x08,
    INPUT_KEY_TAB       = 0x09,
    INPUT_KEY_CAPSLOCK  = 0x14,
    INPUT_KEY_SHIFT     = 0x10,
    INPUT_KEY_CTRL      = 0x11,
    INPUT_KEY_ALT       = 0x12,
    INPUT_KEY_ESC       = 0x1B,
    INPUT_KEY_SPACE     = 0x20,
    INPUT_KEY_ENTER     = 0x0D,
    INPUT_KEY_LEFT      = 0x25,
    INPUT_KEY_UP,
    INPUT_KEY_RIGHT,
    INPUT_KEY_DOWN,
    INPUT_KEY_WIN       = 0x5B,
    INPUT_KEY_MENU      = 0x5D,
    INPUT_KEY_NUM0      = 0x60,
    INPUT_KEY_NUM1,
    INPUT_KEY_NUM2,
    INPUT_KEY_NUM3,
    INPUT_KEY_NUM4,
    INPUT_KEY_NUM5,
    INPUT_KEY_NUM6,
    INPUT_KEY_NUM7,
    INPUT_KEY_NUM8,
    INPUT_KEY_NUM9,
    INPUT_KEY_F1        = 0x70,
    INPUT_KEY_F2,
    INPUT_KEY_F3,
    INPUT_KEY_F4,
    INPUT_KEY_F5,
    INPUT_KEY_F6,
    INPUT_KEY_F7,
    INPUT_KEY_F8,
    INPUT_KEY_F9,
    INPUT_KEY_F10,
    INPUT_KEY_F11,
    INPUT_KEY_F12,
    INPUT_KEY_NUMLOCK   = 0x90,
    INPUT_KEY_SEMI,
    INPUT_KEY_EQUAL,
    INPUT_KEY_COMMA,
    INPUT_KEY_MINUS,
    INPUT_KEY_PERIOD,
    INPUT_KEY_SLASH,
    INPUT_KEY_TILDE,
    INPUT_KEY_LBRACKET  = 0xDB,
    INPUT_KEY_BACKSLASH,
    INPUT_KEY_RBRACKET,
    INPUT_KEY_QUOT,
};

typedef void (*INPUTCALLBACK)(void *cbctxt, int type, int val1, int val2, int val3);
void input_init(INPUTCALLBACK callback, void *cbctxt); // should be called after WINDOW or SCREEN bitmap created
void input_exit(void);
void input_settimer(int id, int interval);
int  input_getkey  (int key);
void input_getmouse(int *x, int *y, int *btns);
void input_setmouse(int  x, int  y);

#ifdef __cplusplus
}
#endif

#endif
