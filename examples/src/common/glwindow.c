#include "glwindow.h"

#ifdef __cplusplus
extern "C" {
#endif

void (*glwindow_key_cb)(wchar_t key, const bool ctrl_alt_shift[3], bool pressed) = NULL;
void (*glwindow_mouse_cb)(char btn, int x, int y);
void (*glwindow_timer_cb)();
void (*glwindow_render_cb)();

#if defined(__linux) || defined(__linux__)

#include <X11/X.h>
#include <X11/Xlib.h>
#include <GL/gl.h>
#include <GL/glx.h>
#include <GL/glu.h>
#include <poll.h>
#include <time.h>

static Display    *glx_dpy;
static Window     glx_win;
static GLXContext glx_ctx;

static bool repaint_call_blocked = false;
static bool destroyed = false;
static int timeout = -1;

int main(int argc, const char **argv)
{
    (void)argc;
    (void)argv;
    extern int app_main();
    app_main();
}

bool glwindow_create(int width, int height, const char *title)
{
    static GLint att[] =
    {
        GLX_RGBA,
        GLX_DEPTH_SIZE,     24,
        GLX_DOUBLEBUFFER,
        GLX_STENCIL_SIZE,   1,
        GLX_SAMPLE_BUFFERS, 1,
        GLX_SAMPLES,        16,
        None
    };

    glx_dpy = XOpenDisplay(NULL);
    if(glx_dpy == NULL)
    {
        printf("cannot connect to X server\n");
        return false;
    }
    Window root = DefaultRootWindow(glx_dpy);
    XVisualInfo *vi;
    vi = glXChooseVisual(glx_dpy, 0, att);
    if (vi == NULL)
    {
        att[9] = 8;
        vi = glXChooseVisual(glx_dpy, 0, att);
    }
    if (vi == NULL)
    {
        att[9] = 4;
        vi = glXChooseVisual(glx_dpy, 0, att);
    }
    if (vi == NULL)
    {
        att[9] = 2;
        vi = glXChooseVisual(glx_dpy, 0, att);
        if (vi != NULL)
            printf("warning: the samling level is low!\n");
    }
    if (vi == NULL)
    {
        printf("no appropriate visual found\n");
        return false;
    }
    Colormap cmap = XCreateColormap(glx_dpy, root, vi->visual, AllocNone);
    XSetWindowAttributes swa;
    swa.colormap = cmap;
    swa.event_mask = ExposureMask | KeyPressMask | KeyReleaseMask | ButtonPressMask | ButtonReleaseMask | PointerMotionMask;
    glx_win = XCreateWindow(glx_dpy, root, 0, 0, width, height, 0, vi->depth, InputOutput, vi->visual, CWColormap | CWEventMask, &swa);

    XMapWindow(glx_dpy, glx_win);
    XStoreName(glx_dpy, glx_win, title);

    glx_ctx = glXCreateContext(glx_dpy, vi, NULL, GL_TRUE);
    glXMakeCurrent(glx_dpy, glx_win, glx_ctx);

    return true;
}

void glwindow_destroy()
{
    glXMakeCurrent(glx_dpy, None, NULL);
    glXDestroyContext(glx_dpy, glx_ctx);
    XDestroyWindow(glx_dpy, glx_win);
    XCloseDisplay(glx_dpy);
    destroyed = true;
}

void glwindow_get_size(int *width, int *height)
{
    XWindowAttributes gwa;
    XGetWindowAttributes(glx_dpy, glx_win, &gwa);
    *width = gwa.width;
    *height = gwa.height;
}

void glwindow_begin_draw()
{
}

void glwindow_end_draw()
{
    glFinish();
    glXSwapBuffers(glx_dpy, glx_win);
}

bool key_is_pressed(KeySym ks)
{
    char keys_return[32];
    XQueryKeymap(glx_dpy, keys_return);
    KeyCode kc2 = XKeysymToKeycode(glx_dpy, ks);
    return !!(keys_return[kc2 >> 3] & (1 << (kc2 & 7)));
}

static void handle_key_event(XEvent *xev)
{
    if (glwindow_key_cb == NULL) return;

    unsigned key = XLookupKeysym(&xev->xkey, 0);

    bool ctrl_alt_shift[3] = {0, 0, 0};

    switch (key)
    {
    case XK_BackSpace:
    case XK_Tab:
    case XK_Linefeed:
    case XK_Clear:
    case XK_Return:
    case XK_Pause:
    case XK_Scroll_Lock:
    case XK_Sys_Req:
    case XK_Escape:
    case XK_Delete:
        key &= 127;
        break;
    }

    ctrl_alt_shift[0] = key_is_pressed(XK_Control_L) || key_is_pressed(XK_Control_R);
    ctrl_alt_shift[1] = key_is_pressed(XK_Alt_L) || key_is_pressed(XK_Alt_R);
    ctrl_alt_shift[2] = key_is_pressed(XK_Shift_L) || key_is_pressed(XK_Shift_L);

    glwindow_key_cb(key, ctrl_alt_shift, xev->type == KeyPress);
}

static void handle_mouse_event(XEvent *xev)
{
    if (glwindow_mouse_cb == NULL) return;
    char b = 0;
    int x = 0;
    int y = 0;
    if (xev->type == ButtonPress || xev->type == ButtonRelease)
    {
        if (xev->xbutton.button == Button1)
            b = xev->type == ButtonPress ? 'l' : 'L';
        if (xev->xbutton.button == Button2)
            b = xev->type == ButtonPress ? 'm' : 'M';
        if (xev->xbutton.button == Button3)
            b = xev->type == ButtonPress ? 'r' : 'R';
        if (b == 0) return;
        x = xev->xbutton.x;
        y = xev->xbutton.y;
    }
    if (xev->type == MotionNotify)
    {
        x = xev->xmotion.x;
        y = xev->xmotion.y;
    }
    glwindow_mouse_cb(b, x, y);
}

void glwindow_repaint()
{
    if (repaint_call_blocked) return;
    XClearArea(glx_dpy, glx_win, 0, 0, 1, 1, true);
    repaint_call_blocked = true;
}

uint32_t glwindow_time_ms(void)
{
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC, &t);
    return (uint32_t)t.tv_sec * 1000 + t.tv_nsec / 1000000;
}

void glwindow_set_timeout(int ms)
{
    timeout = ms;
}

void glwindow_eventloop()
{
    XEvent xev;
    struct pollfd fds[1];
    fds[0].events = POLLIN;
    fds[0].fd = ConnectionNumber(glx_dpy);
    glwindow_repaint();
    while (!destroyed)
    {
        if (!XPending(glx_dpy))
        {
            if (poll(fds, 1, glwindow_timer_cb == NULL ? -1 : timeout) == 0)
                if (glwindow_timer_cb != NULL)
                    glwindow_timer_cb();
            continue;
        }
        XNextEvent(glx_dpy, &xev);
        switch (xev.type)
        {
        case Expose:
            repaint_call_blocked = false;
            if (glwindow_render_cb)
                glwindow_render_cb();
            break;
        case KeyPress:
        case KeyRelease:
            handle_key_event(&xev);
            break;
        case ButtonPress:
        case ButtonRelease:
        case MotionNotify:
            handle_mouse_event(&xev);
            break;
        }
    }
}

#endif

#if defined(__WINNT__) || defined(_WIN32) || defined(_WIN64)

#include <Windows.h>
#include <GL/GL.h>
#include <GL/GLU.h>

HDC      hDC;   /* device context */
HGLRC    hRC;   /* opengl context */
HWND     hWnd;  /* window */

int APIENTRY WinMain(HINSTANCE hCurrentInst, HINSTANCE hPreviousInst, LPSTR lpszCmdLine, int nCmdShow)
{
    (void)hCurrentInst;
    (void)hPreviousInst;
    (void)lpszCmdLine;
    (void)nCmdShow;
    extern int app_main();
    app_main();
}

void handle_wm_key(int key, bool down)
{
    if (glwindow_key_cb == NULL) return;
    static bool esc_pressed = false;
    bool ctrl_alt_shift[3];
    if (!esc_pressed && key == 27 && !down) return;
    if (key == 27) esc_pressed = down;
    glwindow_key_cb(key, ctrl_alt_shift, down);
}

void handle_mouse_event(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if (glwindow_mouse_cb == NULL) return;
    int x = (int16_t)(lParam & 0xFFFF);
    int y = (int16_t)((lParam >> 16) & 0xFFFF);
    char b = 0;
    switch (uMsg)
    {
    case WM_LBUTTONDOWN: b = 'l'; break;
    case WM_LBUTTONUP:   b = 'L'; break;
    case WM_RBUTTONDOWN: b = 'r'; break;
    case WM_RBUTTONUP:   b = 'R'; break;
    }
    glwindow_mouse_cb(b, x, y);
}

LONG WINAPI WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_PAINT:
        PostMessage(hWnd, WM_USER, 0, 0);
        break;
    case WM_SIZE:
        break;
    case WM_USER:
        if (glwindow_render_cb != NULL)
            glwindow_render_cb();
        return 0;
    case WM_KEYDOWN:
    case WM_KEYUP:
        handle_wm_key(wParam, uMsg == WM_KEYDOWN);
        return 0;
    case WM_LBUTTONDOWN:
    case WM_RBUTTONDOWN:
    case WM_MBUTTONDOWN:
        SetCapture(hWnd);
        handle_mouse_event(uMsg, wParam, lParam);
        return 0;
    case WM_LBUTTONUP:
    case WM_RBUTTONUP:
    case WM_MBUTTONUP:
        handle_mouse_event(uMsg, wParam, lParam);
        ReleaseCapture();
        return 0;
    case WM_MOUSEMOVE:
    case WM_MOUSEWHEEL:
        handle_mouse_event(uMsg, wParam, lParam);
        return 0;
    case WM_TIMER:
        if (glwindow_timer_cb != NULL)
            glwindow_timer_cb();
        return 0;
    case WM_CLOSE:
        PostMessageA(hWnd, WM_QUIT, 0, 0);
        return 0;
    }
    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}


#define WGL_SAMPLE_BUFFERS_ARB             0x2041
#define WGL_SAMPLES_ARB                    0x2042

#define WGL_NUMBER_PIXEL_FORMATS_ARB       0x2000
#define WGL_DRAW_TO_WINDOW_ARB             0x2001
#define WGL_DRAW_TO_BITMAP_ARB             0x2002
#define WGL_ACCELERATION_ARB               0x2003
#define WGL_NEED_PALETTE_ARB               0x2004
#define WGL_NEED_SYSTEM_PALETTE_ARB        0x2005
#define WGL_SWAP_LAYER_BUFFERS_ARB         0x2006
#define WGL_SWAP_METHOD_ARB                0x2007
#define WGL_NUMBER_OVERLAYS_ARB            0x2008
#define WGL_NUMBER_UNDERLAYS_ARB           0x2009
#define WGL_TRANSPARENT_ARB                0x200A
#define WGL_TRANSPARENT_RED_VALUE_ARB      0x2037
#define WGL_TRANSPARENT_GREEN_VALUE_ARB    0x2038
#define WGL_TRANSPARENT_BLUE_VALUE_ARB     0x2039
#define WGL_TRANSPARENT_ALPHA_VALUE_ARB    0x203A
#define WGL_TRANSPARENT_INDEX_VALUE_ARB    0x203B
#define WGL_SHARE_DEPTH_ARB                0x200C
#define WGL_SHARE_STENCIL_ARB              0x200D
#define WGL_SHARE_ACCUM_ARB                0x200E
#define WGL_SUPPORT_GDI_ARB                0x200F
#define WGL_SUPPORT_OPENGL_ARB             0x2010
#define WGL_DOUBLE_BUFFER_ARB              0x2011
#define WGL_STEREO_ARB                     0x2012
#define WGL_PIXEL_TYPE_ARB                 0x2013
#define WGL_COLOR_BITS_ARB                 0x2014
#define WGL_RED_BITS_ARB                   0x2015
#define WGL_RED_SHIFT_ARB                  0x2016
#define WGL_GREEN_BITS_ARB                 0x2017
#define WGL_GREEN_SHIFT_ARB                0x2018
#define WGL_BLUE_BITS_ARB                  0x2019
#define WGL_BLUE_SHIFT_ARB                 0x201A
#define WGL_ALPHA_BITS_ARB                 0x201B
#define WGL_ALPHA_SHIFT_ARB                0x201C
#define WGL_ACCUM_BITS_ARB                 0x201D
#define WGL_ACCUM_RED_BITS_ARB             0x201E
#define WGL_ACCUM_GREEN_BITS_ARB           0x201F
#define WGL_ACCUM_BLUE_BITS_ARB            0x2020
#define WGL_ACCUM_ALPHA_BITS_ARB           0x2021
#define WGL_DEPTH_BITS_ARB                 0x2022
#define WGL_STENCIL_BITS_ARB               0x2023
#define WGL_AUX_BUFFERS_ARB                0x2024
#define WGL_NO_ACCELERATION_ARB            0x2025
#define WGL_GENERIC_ACCELERATION_ARB       0x2026
#define WGL_FULL_ACCELERATION_ARB          0x2027
#define WGL_SWAP_EXCHANGE_ARB              0x2028
#define WGL_SWAP_COPY_ARB                  0x2029
#define WGL_SWAP_UNDEFINED_ARB             0x202A
#define WGL_TYPE_RGBA_ARB                  0x202B
#define WGL_TYPE_COLORINDEX_ARB            0x202C

typedef BOOL(WINAPI * PFNWGLCHOOSEPIXELFORMATARBPROC)(HDC hdc, const int *piAttribIList, const FLOAT *pfAttribFList, UINT nMaxFormats, int *piFormats, UINT *nNumFormats);

bool init_multisample_context()
{
    // 1. Init gl context without MSAA support
    PIXELFORMATDESCRIPTOR pfd;
    memset(&pfd, 0, sizeof(pfd));
    pfd.nSize = sizeof(pfd);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 24;
    pfd.cAlphaBits = 8;
    pfd.cDepthBits = 16;
    int pixelFormat = ChoosePixelFormat(hDC, &pfd);
    if (pixelFormat == 0) return false;
    if (SetPixelFormat(hDC, pixelFormat, &pfd) == FALSE) return false;
    DescribePixelFormat(hDC, pixelFormat, sizeof(PIXELFORMATDESCRIPTOR), &pfd);
    hRC = wglCreateContext(hDC);
    wglMakeCurrent(hDC, hRC);

    // 2. Change the pixel format
    PFNWGLCHOOSEPIXELFORMATARBPROC wglChoosePixelFormatARB = (PFNWGLCHOOSEPIXELFORMATARBPROC)wglGetProcAddress("wglChoosePixelFormatARB");
    if (!wglChoosePixelFormatARB)
    {
        MessageBoxA(NULL, "Warning! Unable to init MSAA. Text will be displayed without anti-aliasing.", "Error", MB_OK);
        return true;
    }

    float fAttributes[] = { 0,0 };
    int iAttributes[] = {
        WGL_DRAW_TO_WINDOW_ARB, GL_TRUE, // Истинна, если формат пикселя может быть использован в окне
        WGL_SUPPORT_OPENGL_ARB, GL_TRUE, // Истинна, если поддерживается OpenGL
        WGL_ACCELERATION_ARB, WGL_FULL_ACCELERATION_ARB, // Полная аппаратная поддержка
        WGL_COLOR_BITS_ARB, 24,         // Цветность
        WGL_ALPHA_BITS_ARB, 8,          // Размерность альфа-канала
        WGL_DEPTH_BITS_ARB, 16,         // Глубина буфера глубины
        WGL_STENCIL_BITS_ARB, 8,        // Глубина буфера шаблона
        WGL_DOUBLE_BUFFER_ARB, GL_TRUE, // Истина, если используется двойная буферизация
        WGL_SAMPLE_BUFFERS_ARB, GL_TRUE,// Что мы и хотим
        WGL_SAMPLES_ARB, 8,             // проверка на 8x тип
        0,0};
    UINT numFormats;
    bool valid = wglChoosePixelFormatARB(hDC, iAttributes, fAttributes, 1, &pixelFormat, &numFormats);
    if (!valid || numFormats < 1)
    {
        iAttributes[19] = 4;
        valid = wglChoosePixelFormatARB(hDC, iAttributes, fAttributes, 1, &pixelFormat, &numFormats);
        if (!valid || numFormats < 1)
        {
            MessageBoxA(NULL, "Warning! Unable to init MSAA. Text will be displayed without anti-aliasing.", "Error", MB_OK);
            return true;
        }
    }

    if (SetPixelFormat(hDC, pixelFormat, &pfd) == FALSE)
    {
        wglMakeCurrent(0, 0);
        wglDeleteContext(hRC);
        return false;
    }

    return true;
}

bool glwindow_create(int width, int height, const char *title)
{
	static HINSTANCE hInstance = 0;

	/* only register the window class once - use hInstance as a flag. */
	if (!hInstance) {
		WNDCLASSEXW wcex;

		wcex.cbSize = sizeof(WNDCLASSEX);

		wcex.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
		wcex.lpfnWndProc = WindowProc;
		wcex.cbClsExtra = 0;
		wcex.cbWndExtra = 0;
		wcex.hInstance = hInstance;
		wcex.hIcon = LoadIcon(NULL, IDI_APPLICATION);
		wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
		wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
		wcex.lpszMenuName = NULL;
		wcex.lpszClassName = L"OpenGL";
		wcex.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

		if (!RegisterClassExW(&wcex))
		{
			MessageBoxA(NULL, "RegisterClass() failed: Cannot register window class.", "Error", MB_OK);
			return false;
		}
	}

	hWnd = CreateWindowW(L"OpenGL", L"", WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
		0, 0, width, height, NULL, NULL, hInstance, NULL);

	if (hWnd == NULL) 
    {
        UnregisterClassW(L"OpenGL", hInstance);
		MessageBoxA(NULL, "CreateWindow() failed:  Cannot create a window.", "Error", MB_OK);
		return false;
	}

    SetWindowTextA(hWnd, title);

	hDC = GetDC(hWnd);

    if (!init_multisample_context())
    {
        MessageBoxA(NULL, "Unable to init opengl context", "Error", MB_OK);
        return false;
    }

    ShowWindow(hWnd, SW_SHOWNORMAL);

    return true;
}

void glwindow_destroy()
{
    PostMessageA(hWnd, WM_QUIT, 0, 0);
}

void glwindow_get_size(int *width, int *height)
{
    RECT rect;
    GetWindowRect(hWnd, &rect);
    *width = rect.right - rect.left;
    *height = rect.bottom - rect.top;
}

void glwindow_begin_draw()
{
    wglMakeCurrent(hDC, hRC);
}

void glwindow_end_draw()
{
    glFinish();
    glFlush();
    wglMakeCurrent(0, 0);
    SwapBuffers(hDC);
}

void glwindow_repaint()
{
    PostMessage(hWnd, WM_USER, 0, 0);
}

uint32_t glwindow_time_ms(void)
{
	return GetTickCount();
}

void glwindow_set_timeout(int ms)
{
    SetTimer(hWnd, 1, ms, NULL);
}

void glwindow_eventloop()
{
    MSG msg;   /* message */

    while (GetMessage(&msg, hWnd, 0, 0) > 0)
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    wglMakeCurrent(NULL, NULL);
    ReleaseDC(hWnd, hDC);
    wglDeleteContext(hRC);
    DestroyWindow(hWnd);
    UnregisterClassW(L"OpenGL", 0);
}

#endif

#ifdef __cplusplus
}
#endif
