#include <windows.h>
#include <pthread.h>
#include "screen.h"

#define DDSCL_FULLSCREEN       0x00000001
#define DDSCL_EXCLUSIVE        0x00000010
#define	DDSD_CAPS		       0x00000001
#define DDSCAPS_PRIMARYSURFACE 0x00000200
#define DDLOCK_WAIT            0x00000001

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
static void _ddraw_bmp_unlock (void *pb);
static LRESULT CALLBACK RGE_DDRAW_WNDPROC(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

static HWND                s_ddraw_hwnd  = NULL;
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

    s_ddraw_hwnd = CreateWindow(RGE_WND_CLASS, RGE_WND_NAME, WS_POPUP,
        CW_USEDEFAULT, CW_USEDEFAULT, pbmp->width, pbmp->height,
        NULL, NULL, wc.hInstance, NULL);
    if (!s_ddraw_hwnd) goto done;

    ShowWindow  (s_ddraw_hwnd, SW_SHOW);
    UpdateWindow(s_ddraw_hwnd);

    s_hDllDDraw = LoadLibrary(TEXT("ddraw.dll"));
    if (!s_hDllDDraw) goto done;
    create = (PFN_DirectDrawCreate)GetProcAddress(s_hDllDDraw, "DirectDrawCreate");
    if (!create) goto done;
    create(NULL, &s_lpDirectDraw, NULL);
    if (s_lpDirectDraw == NULL) goto done;
    s_lpDirectDraw->pVtbl->SetCooperativeLevel(s_lpDirectDraw, s_ddraw_hwnd, DDSCL_FULLSCREEN|DDSCL_EXCLUSIVE);
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
    if (s_ddraw_hwnd  ) { DestroyWindow(s_ddraw_hwnd); s_ddraw_hwnd = NULL; }
    if (s_hDllDDraw   ) { FreeLibrary(s_hDllDDraw);    s_hDllDDraw  = NULL; }
    s_ddraw_inited = TRUE;
    return NULL;
}

static void _ddraw_bmp_create(void *pb)
{
    int i;
    s_ddraw_inited = FALSE;
    pthread_create(&s_ddraw_thread, NULL, _ddraw_bmp_thread_proc, pb);
    for (i = 0; i < 10 && !s_ddraw_inited; i++) Sleep(100);
}

static void _ddraw_bmp_destroy(void *pb, int flags)
{
    if (flags) PostMessage(s_ddraw_hwnd, WM_CLOSE, 0, 0);
    if (s_ddraw_thread) { pthread_join(s_ddraw_thread, NULL); s_ddraw_thread = (pthread_t)NULL; }
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

static void _ddraw_bmp_unlock(void *pb)
{
    if (s_lpDDSPrimary) s_lpDDSPrimary->pVtbl->Unlock(s_lpDDSPrimary, NULL);
    ((BMP*)pb)->pdata = NULL;
}

static LRESULT CALLBACK RGE_DDRAW_WNDPROC(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
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
