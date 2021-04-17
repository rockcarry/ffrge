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
    void (*unlock )(void *pb);
} BMP;

void bitmap_create  (BMP *pb, int w, int h);
void bitmap_destroy (BMP *pb, int flags);
void bitmap_lock    (BMP *pb);
void bitmap_unlock  (BMP *pb);
void bitmap_load    (BMP *pb, char *file);
void bitmap_save    (BMP *pb, char *file);
void bitmap_putpixel(BMP *pb, int x, int y, int c);
int  bitmap_getpixel(BMP *pb, int x, int y);

#ifdef __cplusplus
}
#endif

#endif


