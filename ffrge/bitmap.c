#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "bitmap.h"

#pragma warning(disable:4996)

#define DEF_BITMAP_WIDTH   640
#define DEF_BITMAP_HEIGHT  480

void bitmap_create(BMP *pb, int w, int h)
{
    if (!pb) return;
    pb->width  = w ? w : DEF_BITMAP_WIDTH;
    pb->height = h ? h : DEF_BITMAP_HEIGHT;
    pb->stride = pb->width * sizeof(uint32_t);
    if (pb->create) pb->create(pb);
    else pb->pdata = calloc(1, pb->stride * pb->height);
}

void bitmap_destroy(BMP *pb, int flags)
{
    if (!pb) return;
    if (pb->destroy) pb->destroy(pb, flags);
    else { free(pb->pdata); pb->pdata = NULL; }
}

void bitmap_lock  (BMP *pb)            { if (pb && pb->lock  ) pb->lock(pb);          }
void bitmap_unlock(BMP *pb, int flags) { if (pb && pb->unlock) pb->unlock(pb, flags); }

/* 内部类型定义 */
/* BMP 文件结构类型定义 */
#pragma pack(1)
typedef struct {
    uint16_t  filetype;
    uint32_t  filesize;
    uint32_t  reserved;
    uint32_t  dataoffset;
    uint32_t  infosize;
    int32_t   imagewidth;
    int32_t   imageheight;
    uint16_t  planes;
    uint16_t  bitsperpixel;
    uint32_t  compression;
    uint32_t  imagesize;
    int32_t   xpixelpermeter;
    int32_t   ypixelpermeter;
    uint32_t  colorused;
    uint32_t  colorimportant;
} BMPFILE;
#pragma pack()

void bitmap_load(BMP *pb, char *file)
{
    FILE   *fp = NULL;
    BMPFILE bmpfile;

    if (!pb || !file || !(fp = fopen(file, "rb"))) return;
    if (sizeof(bmpfile) != fread(&bmpfile, 1, sizeof(bmpfile), fp)) goto done;
    if (bmpfile.filetype != (('B' << 0) | ('M' << 8))) goto done;
    bitmap_create(pb, bmpfile.imagewidth, bmpfile.imageheight);

    bitmap_lock(pb);
    if (pb->pdata) {
        uint8_t *pdst = (uint8_t*)pb->pdata + (pb->height - 1) * pb->stride;
        int      skip = (4 - ((bmpfile.imagewidth * bmpfile.bitsperpixel + 7) / 8) & 0x3) & 0x3, i, j;
        uint8_t  pal[256 * 4], c1, c2, r, g, b, a = 0;
        if (bmpfile.bitsperpixel == 4 || bmpfile.bitsperpixel == 8) {
            fseek(fp, sizeof(BMPFILE), SEEK_SET);
            fread(pal, 1, (1 << bmpfile.bitsperpixel) * 4, fp);
        }
        fseek(fp, bmpfile.dataoffset, SEEK_SET);
        for (i = 0; i < pb->height; i++) {
            for (j = 0; j < pb->width; j++) {
                switch (bmpfile.bitsperpixel) {
                case 1:
                    if (!(j & 7)) {
                        c1 = fgetc(fp);
                    }
                    r = g = b = (c1 & (1 << 7)) ? 255 : 0;
                    c1 <<= 1;
                    break;
                case 4: case 8:
                    if (bmpfile.bitsperpixel == 4) {
                        if (!(j & 1)) c1 = fgetc(fp);
                        c2 = c1 >> 4;
                        c1 = c1 << 4;
                    } else c2 = fgetc(fp);
                    b = pal[c2 * 4 + 0]; g = pal[c2 * 4 + 1]; r = pal[c2 * 4 + 2];
                    break;
                case 24: b = fgetc(fp); g = fgetc(fp); r = fgetc(fp); break;
                case 32: b = fgetc(fp); g = fgetc(fp); r = fgetc(fp); a = fgetc(fp); break;
                }
                *pdst++ = b; *pdst++ = g; *pdst++ = r; *pdst++ = a;
            }
            fseek(fp, skip, SEEK_CUR);
            pdst -= 2 * pb->stride;
        }
    }
    bitmap_unlock(pb, 0);

done:
    if (fp) fclose(fp);
}

void bitmap_save(BMP *pb, char *file)
{
    BMPFILE  bmpfile = {0};
    void    *fp      = NULL;
    uint8_t *psrc    = NULL;
    int      skip, i, j;
    if (!pb || !pb->pdata || !file || !(fp = fopen(file, "wb"))) return;
    bmpfile.filetype     = ('B' << 0) | ('M' << 8);
    bmpfile.dataoffset   = sizeof(bmpfile);
    bmpfile.imagesize    = pb->stride * pb->height;
    bmpfile.filesize     = bmpfile.dataoffset + bmpfile.imagesize;
    bmpfile.infosize     = 40;
    bmpfile.imagewidth   = pb->width;
    bmpfile.imageheight  = pb->height;
    bmpfile.planes       = 1;
    bmpfile.bitsperpixel = 24;
    fwrite(&bmpfile, 1, sizeof(bmpfile), fp);
    psrc = (uint8_t*)pb->pdata + (pb->height - 1) * pb->stride;
    skip = (4 - (bmpfile.imagewidth * bmpfile.bitsperpixel / 8) & 0x3) & 0x3;
    for (i = 0; i < pb->height; i++) {
        for (j = 0; j < pb->width; j++) {
            fputc(*psrc++, fp);
            fputc(*psrc++, fp);
            fputc(*psrc++, fp);
            psrc++;
        }
        fseek(fp, skip, SEEK_CUR);
        psrc -= 2 * pb->stride;
    }
    if (fp) fclose(fp);
}

void bitmap_putpixel(BMP *pb, int x, int y, int c)
{
    if (pb && pb->pdata && x >= 0 && x < pb->width && y >= 0 && y < pb->height) {
        *(uint32_t*)((uint8_t*)pb->pdata + y * pb->stride + x * sizeof(uint32_t)) = c;
    }
}

int bitmap_getpixel(BMP *pb, int x, int y)
{
    if (pb && pb->pdata && x >= 0 && x < pb->width && y >= 0 && y < pb->height) {
        return *(uint32_t*)((uint8_t*)pb->pdata + y * pb->stride + x * sizeof(uint32_t));
    } else return 0;
}

void bitmap_line(BMP *pb, int x1, int y1, int x2, int y2, int c)
{
    int x, y, dx, dy, e;

    dx = abs(x1 - x2);
    dy = abs(y1 - y2);
    e  = -dx;

    if (dy < dx) {
        if (x1 > x2) {
            x  = x1; x1 = x2; x2 = x;
            y  = y1; y1 = y2; y2 = y;
        }

        y = y1; x = x1;
        while (x <= x2) {
            bitmap_putpixel(pb, x, y, c);
            e += dy * 2;
            if (e >= 0) {
                if (y1 < y2) y++;
                else y--;
                e -= dx * 2;
            }
            x++;
        }
    } else {
        if (y1 > y2) {
            x  = x1; x1 = x2; x2 = x;
            y  = y1; y1 = y2; y2 = y;
        }

        y = y1; x = x1;
        while (y <= y2) {
            bitmap_putpixel(pb, x, y, c);
            e += dx * 2;
            if (e > 0) {
                if (x1 < x2) x++;
                else x--;
                e -= dy * 2;
            }
            y++;
        }
    }
}
