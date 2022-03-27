#ifndef __RGE_BITMAP_H__
#define __RGE_BITMAP_H__

#ifdef __cplusplus
extern "C" {
#endif

enum {
    FILL_COLOR  = 1,
    FILL_BITMAP = 2,
    FILL_RASTER = 3,
};

#define FILL_RASTERW(w)  (((w) & 0x3FFF) << 4 )
#define FILL_RASTERH(h)  (((h) & 0x3FFF) << 18)
#define FILL_MASKC(c)    ((((c) & 0xFFFFFFF) << 4) | (1 << 28))
#define FILL_ALPHA(a)    (((a) & 0xFF) << 24)

typedef struct {
    int    width;
    int    height;
    int    stride;
    void  *pdata;
    void (*create )(void *pb);
    void (*destroy)(void *pb, int flags);
    void (*lock   )(void *pb);
    void (*unlock )(void *pb, int flags);
} BMP;

void bitmap_create  (BMP *pb, int w, int h);
void bitmap_destroy (BMP *pb, int flags);
void bitmap_lock    (BMP *pb);
void bitmap_unlock  (BMP *pb, int flags);
void bitmap_load    (BMP *pb, char *file);
void bitmap_save    (BMP *pb, char *file);
void bitmap_putpixel(BMP *pb, int x, int y, int c);
int  bitmap_getpixel(BMP *pb, int x, int y);
void bitmap_scanline(BMP *pb, int x1, int x2, int y, int type, int color, void *data, int orgx, int orgy);
void bitmap_line    (BMP *pb, int x1, int y1, int x2, int y2, int c);
void bitmap_rect    (BMP *pb, int x1, int y1, int x2, int y2, int type, int color, void *data, int orgx, int orgy);
void bitmap_bitblt  (BMP *pbdst, int dstx, int dsty, BMP *pbsrc, int srcx, int srcy, int srcw, int srch, int type);
void bitmap_putbmp  (BMP *pbdst, int x, int y, BMP *pbsrc, int type);

#ifdef __cplusplus
}
#endif

#endif


