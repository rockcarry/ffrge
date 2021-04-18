#include <windows.h>
#include <pthread.h>
#include "window.h"

#define RGE_WND_CLASS TEXT("RGEWndClass")
#define RGE_WND_NAME  TEXT("RGE")

static void _gdi_bmp_create (void *pb);
static void _gdi_bmp_destroy(void *pb, int flags);
static void _gdi_bmp_lock   (void *pb);
static void _gdi_bmp_unlock (void *pb, int flags);
static LRESULT CALLBACK RGE_GDI_WNDPROC(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

static HWND      s_gdi_hwnd  = NULL;
static HBITMAP   s_gdi_hbmp  = NULL;
static HDC       s_gdi_hdc   = NULL;
static pthread_t s_gdi_thread= (pthread_t)NULL;

BMP WINDOW = { 0, 0, 0, NULL, _gdi_bmp_create, _gdi_bmp_destroy, _gdi_bmp_lock, _gdi_bmp_unlock };

static void* _gdi_bmp_thread_proc(void *param)
{
    BMP        *pbmp = (BMP*)param;
    WNDCLASS    wc   = {0};
    RECT        rect = {0};
    MSG         msg  = {0};
    int         x, y, w, h;

    wc.lpfnWndProc   = RGE_GDI_WNDPROC;
    wc.hInstance     = GetModuleHandle(NULL);
    wc.hIcon         = LoadIcon  (NULL, IDI_APPLICATION);
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)GetStockObject(NULL_BRUSH);
    wc.lpszClassName = RGE_WND_CLASS;
    if (!RegisterClass(&wc)) return NULL;

    s_gdi_hwnd = CreateWindow(RGE_WND_CLASS, RGE_WND_NAME, WS_SYSMENU|WS_MINIMIZEBOX,
        CW_USEDEFAULT, CW_USEDEFAULT, pbmp->width, pbmp->height,
        NULL, NULL, wc.hInstance, NULL);
    if (!s_gdi_hwnd) goto done;

    GetClientRect(s_gdi_hwnd, &rect);
    w = pbmp->width  + (pbmp->width  - rect.right );
    h = pbmp->height + (pbmp->height - rect.bottom);
    x = (GetSystemMetrics(SM_CXSCREEN) - w) / 2;
    y = (GetSystemMetrics(SM_CYSCREEN) - h) / 2;
    x = x > 0 ? x : 0;
    y = y > 0 ? y : 0;

    MoveWindow  (s_gdi_hwnd, x, y, w, h, FALSE);
    ShowWindow  (s_gdi_hwnd, SW_SHOW);
    UpdateWindow(s_gdi_hwnd);

    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage (&msg);
    }

done:
    if (s_gdi_hwnd) { DestroyWindow(s_gdi_hwnd); s_gdi_hwnd = NULL; }
    return NULL;
}

static void _gdi_bmp_create(void *pb)
{
    BMP        *pbmp    = (BMP*)pb;
    BITMAP      bitmap  = {0};
    BITMAPINFO *bmpinfo = (BITMAPINFO*)&bitmap;

    bmpinfo->bmiHeader.biSize        =  sizeof(BITMAPINFOHEADER);
    bmpinfo->bmiHeader.biWidth       =  pbmp->width;
    bmpinfo->bmiHeader.biHeight      = -pbmp->height;
    bmpinfo->bmiHeader.biPlanes      =  1;
    bmpinfo->bmiHeader.biBitCount    =  32;
    bmpinfo->bmiHeader.biCompression =  BI_RGB;

    s_gdi_hdc  = CreateCompatibleDC(NULL);
    s_gdi_hbmp = CreateDIBSection(s_gdi_hdc, bmpinfo, DIB_RGB_COLORS, (void**)&(pbmp->pdata), NULL, 0);
    if (!s_gdi_hdc || !s_gdi_hbmp || !pbmp->pdata) goto failed;

    GetObject(s_gdi_hbmp, sizeof(BITMAP), &bitmap);
    SelectObject(s_gdi_hdc, s_gdi_hbmp);
    pbmp->stride = bitmap.bmWidthBytes;

    pthread_create(&s_gdi_thread, NULL, _gdi_bmp_thread_proc, pb);
    return;

failed:
    _gdi_bmp_destroy(pb, 0);
}

static void _gdi_bmp_destroy(void *pb, int flags)
{
    if (flags) PostMessage(s_gdi_hwnd, WM_CLOSE, 0, 0);
    if (s_gdi_thread) { pthread_join(s_gdi_thread, NULL); s_gdi_thread = (pthread_t)NULL; }
    if (s_gdi_hdc   ) { DeleteDC     (s_gdi_hdc ); s_gdi_hdc  = NULL; }
    if (s_gdi_hbmp  ) { DeleteObject (s_gdi_hbmp); s_gdi_hbmp = NULL; }
    ((BMP*)pb)->pdata = NULL;
}

static void _gdi_bmp_lock  (void *pb) {}
static void _gdi_bmp_unlock(void *pb, int flags)
{
    if (s_gdi_hwnd) {
        if (flags) {
            HDC hdc = GetDC(s_gdi_hwnd);
            BitBlt(hdc, 0, 0, ((BMP*)pb)->width, ((BMP*)pb)->height, s_gdi_hdc, 0, 0, SRCCOPY);
            ReleaseDC(s_gdi_hwnd ,hdc);
        } else InvalidateRect(s_gdi_hwnd, NULL, FALSE);
    }
}

static LRESULT CALLBACK RGE_GDI_WNDPROC(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    PAINTSTRUCT ps = {0};
    HDC        hdc = NULL;
    switch (uMsg) {
    case WM_PAINT:
        hdc = BeginPaint(hwnd, &ps);
        BitBlt(hdc, ps.rcPaint.left, ps.rcPaint.top,
            ps.rcPaint.right  - ps.rcPaint.left,
            ps.rcPaint.bottom - ps.rcPaint.top,
            s_gdi_hdc, ps.rcPaint.left, ps.rcPaint.top, SRCCOPY);
        EndPaint(hwnd, &ps);
        return 0;
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    default:
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
}

#ifdef _TEST_WINDOW_
int PASCAL WinMain(HINSTANCE hInst, HINSTANCE hPreInst, LPSTR lpszCmdLine, int nCmdShow)
{
    bitmap_load(&WINDOW, "test.bmp");
    bitmap_destroy(&WINDOW, 0);
    return 0;
}
#endif
