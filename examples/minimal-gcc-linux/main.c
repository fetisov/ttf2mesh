#include <stdio.h>
#include <stdlib.h>
#include <X11/X.h>
#include <X11/Xlib.h>
#include <GL/gl.h>
#include <GL/glx.h>
#include <GL/glu.h>
#include "ttf2mesh.h"

/* X11 Window and opengl context */

Display                 *dpy;
Window                  root;
GLint                   att[] = { GLX_RGBA, GLX_DEPTH_SIZE, 24, GLX_DOUBLEBUFFER,
                                  GLX_SAMPLE_BUFFERS_ARB, 1, GLX_SAMPLES_ARB, 1,
                                  GLX_SAMPLES, 8, None };
XVisualInfo             *vi;
Colormap                cmap;
XSetWindowAttributes    swa;
Window                  win;
GLXContext              glc;
XWindowAttributes       gwa;

/* Font variables */

ttf_t                  *ttf   = NULL;
ttf_glyph_t            *glyph = NULL;
ttf_mesh_t             *mesh  = NULL;

bool set_drawing_symbol(char symbol)
{
    int glyph_index = ttf_find_glyph(ttf, symbol);
    if (glyph_index == -1)
    {
        printf("unable to locate symbol '%c' in font\n", symbol);
        return false;
    }
    glyph = &ttf->glyphs[glyph_index];
    ttf_free_mesh(mesh);
    if (ttf_glyph2mesh(glyph, &mesh, TTF_QUALITY_NORMAL, 0) != 0)
    {
        mesh = NULL;
        printf("unable to create mesh\n");
        return false;
    }
    return true;
}

//void draw_symbol(unsigned mode)
//{
//    glClearColor(1.0, 1.0, 1.0, 1.0);
//    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

//    glMatrixMode(GL_PROJECTION);
//    glLoadIdentity();
//    glOrtho(-0.5, 1, -0.5, 1, -1, 1);

//    glMatrixMode(GL_MODELVIEW);
//    glLoadIdentity();

//    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
//    glEnable(GL_BLEND);

//    if (mesh == NULL) return;

//    /* Draw contours */
//    if (mode == 0)
//    {
//        int i;
//        for (i = 0; i < mesh->outline->ncontours; i++)
//        {
//            glColor3f(0.0, 0.0, 0.0);
//            glEnableClientState(GL_VERTEX_ARRAY);
//            glVertexPointer(2, GL_FLOAT, sizeof(ttf_point_t), &mesh->outline->cont[i].pt->x);
//            glDrawArrays(GL_LINE_LOOP, 0, mesh->outline->cont[i].length);
//            glDisableClientState(GL_VERTEX_ARRAY);
//        }
//    }

//    /* Draw wireframe/solid */
//    if (mode == 1 || mode == 2)
//    {
//        glColor3f(0.0, 0.0, 0.0);
//        glPolygonMode(GL_FRONT_AND_BACK, mode == 1 ? GL_LINE : GL_FILL);
//        glEnableClientState(GL_VERTEX_ARRAY);
//        glVertexPointer(2, GL_FLOAT, 0, &mesh->vert->x);
//        glDisable(GL_POLYGON_SMOOTH);
//        glDrawElements(GL_TRIANGLES, mesh->nfaces * 3,  GL_UNSIGNED_INT, &mesh->faces->v1);
//        glEnable(GL_POLYGON_SMOOTH);
//        glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
//        glDrawElements(GL_TRIANGLES, mesh->nfaces * 3,  GL_UNSIGNED_INT, &mesh->faces->v1);
//        glDisableClientState(GL_VERTEX_ARRAY);
//    }
//}

void draw_symbol(unsigned mode)
{
    glClearColor(1.0, 1.0, 1.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-0.5, 1, -0.5, 1, -1, 1);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glEnable(GL_MULTISAMPLE_ARB);

    if (mesh == NULL) return;

    /* Draw contours */
    if (mode == 0)
    {
        int i;
        for (i = 0; i < mesh->outline->ncontours; i++)
        {
            glColor3f(0.0, 0.0, 0.0);
            glEnableClientState(GL_VERTEX_ARRAY);
            glVertexPointer(2, GL_FLOAT, sizeof(ttf_point_t), &mesh->outline->cont[i].pt->x);
            glDrawArrays(GL_LINE_LOOP, 0, mesh->outline->cont[i].length);
            glDisableClientState(GL_VERTEX_ARRAY);
        }
    }

    /* Draw wireframe/solid */
    if (mode == 1 || mode == 2)
    {
        glColor3f(0.0, 0.0, 0.0);
        glPolygonMode(GL_FRONT_AND_BACK, mode == 1 ? GL_LINE : GL_FILL);
        glEnableClientState(GL_VERTEX_ARRAY);
        glVertexPointer(2, GL_FLOAT, 0, &mesh->vert->x);
        glDrawElements(GL_TRIANGLES, mesh->nfaces * 3,  GL_UNSIGNED_INT, &mesh->faces->v1);
        glDisableClientState(GL_VERTEX_ARRAY);
    }
}

bool create_gl_window()
{
    dpy = XOpenDisplay(NULL);
    if(dpy == NULL)
    {
        printf("cannot connect to X server\n");
        return false;
    }
    root = DefaultRootWindow(dpy);
    vi = glXChooseVisual(dpy, 0, att);
    if(vi == NULL)
    {
        printf("no appropriate visual found\n");
        return false;
    }
    cmap = XCreateColormap(dpy, root, vi->visual, AllocNone);

    swa.colormap = cmap;
    swa.event_mask = ExposureMask | KeyPressMask;
    win = XCreateWindow(dpy, root, 0, 0, 600, 600, 0, vi->depth, InputOutput, vi->visual, CWColormap | CWEventMask, &swa);

    XMapWindow(dpy, win);
    XStoreName(dpy, win, "Control keys: Space, Escape, ASCII symbol");

    glc = glXCreateContext(dpy, vi, NULL, GL_TRUE);
    glXMakeCurrent(dpy, win, glc);

    return true;
}

int main(int argc, const char **argv)
{
    unsigned mode; /* current drawing mode */

    if (argc != 2)
    {
        printf("usage: %s <path/to/font.ttf>\n", argv[0]);
        return 0;
    }

    if (ttf_load_from_file(argv[1], &ttf, false) != 0)
    {
        printf("unable to load font %s\n", argv[1]);
        return 1;
    }
    printf("Font name: %s (%s)\n", ttf->info.family, ttf->info.subfamily);

    if (!set_drawing_symbol('a'))
        return 1;

    if (!create_gl_window())
        return 2;

    mode = 0;
    while(1)
    {
        XEvent xev;
        KeySym key;
        XNextEvent(dpy, &xev);
        switch (xev.type)
        {
        case Expose:
            XGetWindowAttributes(dpy, win, &gwa);
            glViewport(0, 0, gwa.width, gwa.height);
            draw_symbol(mode % 3);
            glXSwapBuffers(dpy, win);
            break;
        case KeyPress:
            key = XLookupKeysym(&xev.xkey, 0);
            if (key == XK_space)
            {
                mode++;
                XClearArea(dpy, win, 0, 0, 1, 1, true);
                break;
            }
            if (key == XK_Escape)
            {
                glXMakeCurrent(dpy, None, NULL);
                glXDestroyContext(dpy, glc);
                XDestroyWindow(dpy, win);
                XCloseDisplay(dpy);
                return 0;
            }
            set_drawing_symbol(key);
            XClearArea(dpy, win, 0, 0, 1, 1, true);
            break;
        }
    }
}
