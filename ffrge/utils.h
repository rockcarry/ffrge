#ifndef __RGE_UTILS_H__
#define __RGE_UTILS_H__

#include <stdint.h>
#ifdef WIN32
#include <windows.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifdef WIN32
#define msleep(ms)      Sleep(ms)
#define get_tick_count  GetTickCount
#else
#include <unistd.h>
#define msleep(ms)      usleep(ms * 1000)
uint32_t get_tick_count(void);
#endif

void log_init  (char *file);
void log_done  (void);
void log_printf(char *format, ...);

#ifdef __cplusplus
}
#endif

#endif
