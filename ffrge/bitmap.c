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

void bitmap_lock  (BMP *pb) { if (pb && pb->lock  ) pb->lock  (pb); }
void bitmap_unlock(BMP *pb) { if (pb && pb->unlock) pb->unlock(pb); }

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
        uint8_t  *pdst = (uint8_t*)pb->pdata + (pb->height - 1) * pb->stride;
        int       skip = (4 - (bmpfile.imagewidth * bmpfile.bitsperpixel / 8) & 0x3) & 0x3;
        int       i, j;
        fseek(fp, bmpfile.dataoffset, SEEK_SET);
        for (i = 0; i < pb->height; i++) {
            for (j = 0; j < pb->width; j++) {
                *pdst++ = fgetc(fp);
                *pdst++ = fgetc(fp);
                *pdst++ = fgetc(fp);
                *pdst++ = 0;
            }
            fseek(fp, skip, SEEK_CUR);
            pdst -= 2 * pb->stride;
        }
    }
    bitmap_unlock(pb);
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
