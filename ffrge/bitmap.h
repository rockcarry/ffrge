#ifndef __RGE_BITMAP_H__
#define __RGE_BITMAP_H__

#ifdef __cplusplus
extern "C" {
#endif

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
void bitmap_line    (BMP *pb, int x1, int y1, int x2, int y2, int c);

#ifdef __cplusplus
}
#endif

#endif


