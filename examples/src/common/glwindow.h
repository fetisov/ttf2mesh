#ifndef GLWINDOW_H
#define GLWINDOW_H

#if defined(__WINNT__) || defined(_WIN32) || defined(_WIN64)
#include <Windows.h>
#endif

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <wchar.h>
#include <GL/gl.h>

#ifdef __cplusplus
extern "C" {
#endif

bool glwindow_create(int width, int height, const char *title);
void glwindow_destroy();
void glwindow_repaint();
void glwindow_get_size(int *width, int *height);
void glwindow_begin_draw();
void glwindow_end_draw();
void glwindow_set_timeout(int ms);
void glwindow_eventloop();
uint32_t glwindow_time_ms();

extern void (*glwindow_key_cb)(wchar_t key, const bool ctrl_alt_shift[3], bool pressed);
extern void (*glwindow_mouse_cb)(char btn, int x, int y);
extern void (*glwindow_timer_cb)();
extern void (*glwindow_render_cb)();

#ifdef __cplusplus
}
#endif

#endif
