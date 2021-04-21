#include <windows.h>
#include <pthread.h>
#include "screen.h"

#define DDSCL_FULLSCREEN       0x00000001
#define DDSCL_EXCLUSIVE        0x00000010
#define	DDSD_CAPS		       0x00000001
#define DDSCAPS_PRIMARYSURFACE 0x00000200
#define DDLOCK_WAIT            0x00000001

#ifndef WM_MOUSEWHEEL
#define WM_MOUSEWHEEL          0x020A
#endif

typedef struct {
    DWORD dwColorSpaceLowValue;  /* low boundary of color space that is to
                                  * be treated as Color Key, inclusive */
    DWORD dwColorSpaceHighValue; /* high boundary of color space that is
                                  * to be treated as Color Key, inclusive */
} DDCOLORKEY,*LPDDCOLORKEY;

typedef struct {
    DWORD        dwSize;                 /* 0: size of structure */
    DWORD        dwFlags;                /* 4: pixel format flags */
    DWORD        dwFourCC;               /* 8: (FOURCC code) */
    union {
        DWORD    dwRGBBitCount;          /* C: how many bits per pixel */
        DWORD    dwYUVBitCount;          /* C: how many bits per pixel */
        DWORD    dwZBufferBitDepth;      /* C: how many bits for z buffers */
        DWORD    dwAlphaBitDepth;        /* C: how many bits for alpha channels*/
        DWORD    dwLuminanceBitCount;
        DWORD    dwBumpBitCount;
        DWORD    dwPrivateFormatBitCount;
    } DUMMYUNIONNAME1;
    union {
        DWORD    dwRBitMask;             /* 10: mask for red bit*/
        DWORD    dwYBitMask;             /* 10: mask for Y bits*/
        DWORD    dwStencilBitDepth;
        DWORD    dwLuminanceBitMask;
        DWORD    dwBumpDuBitMask;
        DWORD    dwOperations;
    } DUMMYUNIONNAME2;
    union {
        DWORD    dwGBitMask;             /* 14: mask for green bits*/
        DWORD    dwUBitMask;             /* 14: mask for U bits*/
        DWORD    dwZBitMask;
        DWORD    dwBumpDvBitMask;
        struct {
            WORD wFlipMSTypes;
            WORD wBltMSTypes;
        } MultiSampleCaps;
    } DUMMYUNIONNAME3;
    union {
        DWORD    dwBBitMask;             /* 18: mask for blue bits*/
        DWORD    dwVBitMask;             /* 18: mask for V bits*/
        DWORD    dwStencilBitMask;
        DWORD    dwBumpLuminanceBitMask;
    } DUMMYUNIONNAME4;
    union {
        DWORD    dwRGBAlphaBitMask;      /* 1C: mask for alpha channel */
        DWORD    dwYUVAlphaBitMask;      /* 1C: mask for alpha channel */
        DWORD    dwLuminanceAlphaBitMask;
        DWORD    dwRGBZBitMask;          /* 1C: mask for Z channel */
        DWORD    dwYUVZBitMask;          /* 1C: mask for Z channel */
    } DUMMYUNIONNAME5;
                                         /* 20: next structure */
} DDPIXELFORMAT,*LPDDPIXELFORMAT;

typedef struct {
    DWORD   dwCaps;    /* capabilities of surface wanted */
} DDSCAPS, *LPDDSCAPS;

typedef struct {
    DWORD           dwSize;             /* 0:  size of the DDSURFACEDESC structure */
    DWORD           dwFlags;            /* 4:  determines what fields are valid */
    DWORD           dwHeight;           /* 8:  height of surface to be created */
    DWORD           dwWidth;            /* C:  width of input surface */
    LONG            lPitch;             /* 10: distance to start of next line (return value only) */
    DWORD           dwBackBufferCount;  /* 14: number of back buffers requested */
    union {
        DWORD       dwMipMapCount;      /* 18: number of mip-map levels requested */
        DWORD       dwZBufferBitDepth;  /* 18: depth of Z buffer requested */
        DWORD       dwRefreshRate;      /* 18: refresh rate (used when display mode is described) */
    } DUMMYUNIONNAME2;
    DWORD           dwAlphaBitDepth;    /* 1C: depth of alpha buffer requested */
    DWORD           dwReserved;         /* 20: reserved */
    LPVOID          lpSurface;          /* 24: pointer to the associated surface memory */
    DDCOLORKEY      ddckCKDestOverlay;  /* 28: CK for dest overlay use */
    DDCOLORKEY      ddckCKDestBlt;      /* 30: CK for destination blt use */
    DDCOLORKEY      ddckCKSrcOverlay;   /* 38: CK for source overlay use */
    DDCOLORKEY      ddckCKSrcBlt;       /* 40: CK for source blt use */
    DDPIXELFORMAT   ddpfPixelFormat;    /* 48: pixel format description of the surface */
    DDSCAPS         ddsCaps;            /* 68: direct draw surface caps */
} DDSURFACEDESC,*LPDDSURFACEDESC;

typedef struct {
    HRESULT (WINAPI *QueryInterface)(void *this, IID *riid, void** ppvObject);
    ULONG   (WINAPI *AddRef )(void *this);
    ULONG   (WINAPI *Release)(void *this);

    DWORD    dwReserved1[22];
    HRESULT (WINAPI *Lock)(void *this, LPRECT lpDestRect, LPDDSURFACEDESC lpDDSurfaceDesc, DWORD dwFlags, HANDLE hEvent);
    DWORD    dwReserved2[6];
    HRESULT (WINAPI *Unlock)(void *this, LPVOID lpSurfaceData);
} IDirectDrawSurfaceVtbl;

typedef struct {
    IDirectDrawSurfaceVtbl *pVtbl;
} IDirectDrawSurface, *LPDIRECTDRAWSURFACE;

typedef struct {
    HRESULT (WINAPI *QueryInterface)(void *this, IID *riid, void** ppvObject);
    ULONG   (WINAPI *AddRef )(void *this);
    ULONG   (WINAPI *Release)(void *this);

    HRESULT (WINAPI *Compact               )(void *this);
    HRESULT (WINAPI *CreateClipper         )(void *this, DWORD dwFlags, void *lplpDDClipper, IUnknown *pUnkOuter);
    HRESULT (WINAPI *CreatePalette         )(void *this, DWORD dwFlags, void *lpColorTable, void *lplpDDPalette, IUnknown *pUnkOuter);
    HRESULT (WINAPI *CreateSurface         )(void *this, LPDDSURFACEDESC lpDDSurfaceDesc, LPDIRECTDRAWSURFACE *lplpDDSurface, IUnknown *pUnkOuter);
    HRESULT (WINAPI *DuplicateSurface      )(void *this, LPDIRECTDRAWSURFACE lpDDSurface, LPDIRECTDRAWSURFACE *lplpDupDDSurface);
    HRESULT (WINAPI *EnumDisplayModes      )(void *this, DWORD dwFlags, LPDDSURFACEDESC lpDDSurfaceDesc, LPVOID lpContext, void *lpEnumModesCallback);
    HRESULT (WINAPI *EnumSurfaces          )(void *this, DWORD dwFlags, LPDDSURFACEDESC lpDDSD, LPVOID lpContext, void *lpEnumSurfacesCallback);
    HRESULT (WINAPI *FlipToGDISurface      )(void *this);
    DWORD    dwReserved1;
    HRESULT (WINAPI *GetDisplayMode        )(void *this, LPDDSURFACEDESC lpDDSurfaceDesc);
    HRESULT (WINAPI *GetFourCCCodes        )(void *this, LPDWORD lpNumCodes, LPDWORD lpCodes);
    HRESULT (WINAPI *GetGDISurface         )(void *this, LPDIRECTDRAWSURFACE *lplpGDIDDSurface);
    HRESULT (WINAPI *GetMonitorFrequency   )(void *this, LPDWORD lpdwFrequency);
    HRESULT (WINAPI *GetScanLine           )(void *this, LPDWORD lpdwScanLine );
    HRESULT (WINAPI *GetVerticalBlankStatus)(void *this, BOOL *lpbIsInVB);
    HRESULT (WINAPI *Initialize            )(void *this, GUID *lpGUID);
    HRESULT (WINAPI *RestoreDisplayMode    )(void *this);
    HRESULT (WINAPI *SetCooperativeLevel   )(void *this, HWND hWnd, DWORD dwFlags);
    HRESULT (WINAPI *SetDisplayMode        )(void *this, DWORD dwWidth, DWORD dwHeight, DWORD dwBPP);
    HRESULT (WINAPI *WaitForVerticalBlank  )(void *this, DWORD dwFlags, HANDLE hEvent);
} IDirectDrawVtbl;

typedef struct {
    IDirectDrawVtbl *pVtbl;
} IDirectDraw;

typedef void*(WINAPI *PFN_DirectDrawCreate)(void *guid, void **lpddraw, void *iunknown);

#define RGE_WND_CLASS TEXT("RGEWndClass")
#define RGE_WND_NAME  TEXT("RGE")

static void _ddraw_bmp_create (void *pb);
static void _ddraw_bmp_destroy(void *pb, int flags);
static void _ddraw_bmp_lock   (void *pb);
static void _ddraw_bmp_unlock (void *pb, int flags);
static LRESULT CALLBACK RGE_DDRAW_WNDPROC(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
LRESULT (CALLBACK *g_ffrge_input_wndproc)(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

       HWND                g_ffrge_hwnd  = NULL;
static HMODULE             s_hDllDDraw   = NULL;
static IDirectDraw        *s_lpDirectDraw= NULL;
static IDirectDrawSurface *s_lpDDSPrimary= NULL;
static pthread_t           s_ddraw_thread= (pthread_t)NULL;
static BOOL                s_ddraw_inited= FALSE;

BMP SCREEN = { 0, 0, 0, NULL, _ddraw_bmp_create, _ddraw_bmp_destroy, _ddraw_bmp_lock, _ddraw_bmp_unlock };

static void* _ddraw_bmp_thread_proc(void *param)
{
    BMP        *pbmp = (BMP*)param;
    WNDCLASS    wc   = {0};
    MSG         msg  = {0};
    PFN_DirectDrawCreate create;
    DDSURFACEDESC ddsd = {0};

    wc.lpfnWndProc   = RGE_DDRAW_WNDPROC;
    wc.hInstance     = GetModuleHandle(NULL);
    wc.hIcon         = LoadIcon  (NULL, IDI_APPLICATION);
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)GetStockObject(NULL_BRUSH);
    wc.lpszClassName = RGE_WND_CLASS;
    if (!RegisterClass(&wc)) return NULL;

    g_ffrge_hwnd = CreateWindow(RGE_WND_CLASS, RGE_WND_NAME, WS_POPUP,
        CW_USEDEFAULT, CW_USEDEFAULT, pbmp->width, pbmp->height,
        NULL, NULL, wc.hInstance, NULL);
    if (!g_ffrge_hwnd) goto done;

    ShowWindow  (g_ffrge_hwnd, SW_SHOW);
    UpdateWindow(g_ffrge_hwnd);

    s_hDllDDraw = LoadLibrary(TEXT("ddraw.dll"));
    if (!s_hDllDDraw) goto done;
    create = (PFN_DirectDrawCreate)GetProcAddress(s_hDllDDraw, "DirectDrawCreate");
    if (!create) goto done;
    create(NULL, &s_lpDirectDraw, NULL);
    if (s_lpDirectDraw == NULL) goto done;
    s_lpDirectDraw->pVtbl->SetCooperativeLevel(s_lpDirectDraw, g_ffrge_hwnd, DDSCL_FULLSCREEN|DDSCL_EXCLUSIVE);
    s_lpDirectDraw->pVtbl->SetDisplayMode(s_lpDirectDraw, pbmp->width, pbmp->height, 32);

    ddsd.dwSize  = sizeof(ddsd);
    ddsd.dwFlags = DDSD_CAPS;
    ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;
    s_lpDirectDraw->pVtbl->CreateSurface(s_lpDirectDraw, &ddsd, &s_lpDDSPrimary, NULL);
    if (s_lpDDSPrimary == NULL) goto done;
    pbmp->width  = ddsd.dwWidth;
    pbmp->height = ddsd.dwHeight;

    s_ddraw_inited = TRUE;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage (&msg);
    }

done:
    if (s_lpDDSPrimary) { s_lpDDSPrimary->pVtbl->Release(s_lpDDSPrimary); s_lpDDSPrimary = NULL; }
    if (s_lpDirectDraw) { s_lpDirectDraw->pVtbl->Release(s_lpDirectDraw); s_lpDirectDraw = NULL; }
    if (g_ffrge_hwnd  ) { DestroyWindow(g_ffrge_hwnd); g_ffrge_hwnd = NULL; }
    if (s_hDllDDraw   ) { FreeLibrary(s_hDllDDraw);    s_hDllDDraw  = NULL; }
    s_ddraw_inited = TRUE;
    return NULL;
}

static void _ddraw_bmp_create(void *pb)
{
    int i;
    if (g_ffrge_hwnd) return;
    s_ddraw_inited = FALSE;
    pthread_create(&s_ddraw_thread, NULL, _ddraw_bmp_thread_proc, pb);
    for (i = 0; i < 10 && !s_ddraw_inited; i++) Sleep(100);
}

static void _ddraw_bmp_destroy(void *pb, int flags)
{
    if (flags) PostMessage(g_ffrge_hwnd, WM_CLOSE, 0, 0);
    if (s_ddraw_thread) { pthread_join(s_ddraw_thread, NULL); s_ddraw_thread = (pthread_t)NULL; }
    g_ffrge_hwnd = NULL;
}

static void _ddraw_bmp_lock(void *pb)
{
    BMP          *pbmp = (BMP*)pb;
    DDSURFACEDESC ddsd = { sizeof(ddsd) };
    if (s_lpDDSPrimary) s_lpDDSPrimary->pVtbl->Lock(s_lpDDSPrimary, NULL, &ddsd, DDLOCK_WAIT, NULL);
    pbmp->width  = ddsd.dwWidth;
    pbmp->height = ddsd.dwHeight;
    pbmp->pdata  = (BYTE*)ddsd.lpSurface;
    pbmp->stride = ddsd.lPitch;
}

static void _ddraw_bmp_unlock(void *pb, int flags)
{
    if (s_lpDDSPrimary) s_lpDDSPrimary->pVtbl->Unlock(s_lpDDSPrimary, NULL);
    ((BMP*)pb)->pdata = NULL;
}

static LRESULT CALLBACK RGE_DDRAW_WNDPROC(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg) {
    case WM_TIMER: case WM_KEYUP: case WM_KEYDOWN: case WM_SYSKEYUP: case WM_SYSKEYDOWN: case WM_MOUSEMOVE: case WM_MOUSEWHEEL:
    case WM_LBUTTONUP: case WM_LBUTTONDOWN: case WM_RBUTTONUP: case WM_RBUTTONDOWN: case WM_MBUTTONUP: case WM_MBUTTONDOWN:
        if (g_ffrge_input_wndproc) {
            return g_ffrge_input_wndproc(hwnd, uMsg, wParam, lParam);
        }
        break;
    }
    switch (uMsg) {
    case WM_KEYDOWN:
        if ((int)wParam == VK_ESCAPE) {
            PostQuitMessage(0);
            return 0;
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

static void _gdi_bmp_create (void *pb);
static void _gdi_bmp_destroy(void *pb, int flags);
static void _gdi_bmp_lock   (void *pb);
static void _gdi_bmp_unlock (void *pb, int flags);
static LRESULT CALLBACK RGE_GDI_WNDPROC(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

static HBITMAP   s_gdi_hbmp  = NULL;
static HDC       s_gdi_hdc   = NULL;
static pthread_t s_gdi_thread= (pthread_t)NULL;
static BOOL      s_gdi_inited= FALSE;

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

    g_ffrge_hwnd = CreateWindow(RGE_WND_CLASS, RGE_WND_NAME, WS_SYSMENU|WS_MINIMIZEBOX,
        CW_USEDEFAULT, CW_USEDEFAULT, pbmp->width, pbmp->height,
        NULL, NULL, wc.hInstance, NULL);
    if (!g_ffrge_hwnd) goto done;

    GetClientRect(g_ffrge_hwnd, &rect);
    w = pbmp->width  + (pbmp->width  - rect.right );
    h = pbmp->height + (pbmp->height - rect.bottom);
    x = (GetSystemMetrics(SM_CXSCREEN) - w) / 2;
    y = (GetSystemMetrics(SM_CYSCREEN) - h) / 2;
    x = x > 0 ? x : 0;
    y = y > 0 ? y : 0;

    MoveWindow  (g_ffrge_hwnd, x, y, w, h, FALSE);
    ShowWindow  (g_ffrge_hwnd, SW_SHOW);
    UpdateWindow(g_ffrge_hwnd);
    s_gdi_inited = TRUE;

    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage (&msg);
    }

done:
    if (g_ffrge_hwnd) { DestroyWindow(g_ffrge_hwnd); g_ffrge_hwnd = NULL; }
    s_gdi_inited = TRUE;
    return NULL;
}

static void _gdi_bmp_create(void *pb)
{
    BMP        *pbmp    = (BMP*)pb;
    BITMAP      bitmap  = {0};
    BITMAPINFO *bmpinfo = (BITMAPINFO*)&bitmap;
    int         i;

    if (g_ffrge_hwnd) return;
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

    s_gdi_inited = FALSE;
    pthread_create(&s_gdi_thread, NULL, _gdi_bmp_thread_proc, pb);
    for (i = 0; i < 10 && !s_gdi_inited; i++) Sleep(100);
    return;

failed:
    _gdi_bmp_destroy(pb, 0);
}

static void _gdi_bmp_destroy(void *pb, int flags)
{
    if (flags) PostMessage(g_ffrge_hwnd, WM_CLOSE, 0, 0);
    if (s_gdi_thread) { pthread_join(s_gdi_thread, NULL); s_gdi_thread = (pthread_t)NULL; }
    if (s_gdi_hdc   ) { DeleteDC     (s_gdi_hdc ); s_gdi_hdc  = NULL; }
    if (s_gdi_hbmp  ) { DeleteObject (s_gdi_hbmp); s_gdi_hbmp = NULL; }
    ((BMP*)pb)->pdata = NULL;
    g_ffrge_hwnd      = NULL;
}

static void _gdi_bmp_lock  (void *pb) {}
static void _gdi_bmp_unlock(void *pb, int flags)
{
    if (g_ffrge_hwnd) {
        if (flags) {
            HDC hdc = GetDC(g_ffrge_hwnd);
            BitBlt(hdc, 0, 0, ((BMP*)pb)->width, ((BMP*)pb)->height, s_gdi_hdc, 0, 0, SRCCOPY);
            ReleaseDC(g_ffrge_hwnd ,hdc);
        } else InvalidateRect(g_ffrge_hwnd, NULL, FALSE);
    }
}

static LRESULT CALLBACK RGE_GDI_WNDPROC(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    PAINTSTRUCT ps = {0};
    HDC        hdc = NULL;
    switch (uMsg) {
    case WM_TIMER: case WM_KEYUP: case WM_KEYDOWN: case WM_SYSKEYUP: case WM_SYSKEYDOWN: case WM_MOUSEMOVE: case WM_MOUSEWHEEL:
    case WM_LBUTTONUP: case WM_LBUTTONDOWN: case WM_RBUTTONUP: case WM_RBUTTONDOWN: case WM_MBUTTONUP: case WM_MBUTTONDOWN:
        if (g_ffrge_input_wndproc) {
            return g_ffrge_input_wndproc(hwnd, uMsg, wParam, lParam);
        }
        break;
    }
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

#ifdef _TEST_SCREEN_
int PASCAL WinMain(HINSTANCE hInst, HINSTANCE hPreInst, LPSTR lpszCmdLine, int nCmdShow)
{
    int i, j, k;
    bitmap_create(&SCREEN, 1440, 900);
    for (k = 0; k < 100; k++) {
        bitmap_lock(&SCREEN);
        for (i = 0; i < SCREEN.height; i++) {
            for (j = 0; j < SCREEN.width; j++) {
                *(DWORD*)((BYTE*)SCREEN.pdata + i * SCREEN.stride + j * sizeof(DWORD)) = RGB(i, j, k);
            }
        }
        bitmap_unlock(&SCREEN);
        Sleep(20);
    }
    bitmap_destroy(&SCREEN, 0);
    return 0;
}
#endif
