#define _CRT_SECURE_NO_WARNINGS

#include <math.h>
#include <assert.h>
#include <stdio.h>
#include <time.h>
#include <math.h>
#include "ttf2mesh.h"
#include "glwindow.h"

static int view_mode = 0;

static ttf_t *font = NULL;
static ttf_glyph_t *glyph = NULL;
static ttf_mesh_t *mesh = NULL;

static bool load_system_font()
{
    // list all system fonts by filename mask:

    ttf_t **list = ttf_list_system_fonts("DejaVuSans*|Ubuntu*|FreeSerif*|Arial*|Cour*");
    if (list == NULL) return false; // no memory in system
    if (list[0] == NULL) return false; // no fonts were found

    // load the first font from the list

    ttf_load_from_file(list[0]->filename, &font, false);
    ttf_free_list(list);
    if (font == NULL) return false;

    printf("font \"%s\" loaded\n", font->names.full_name);
    return true;
}

static void choose_glyph(wchar_t symbol)
{
    // find a glyph in the font file

    int index = ttf_find_glyph(font, symbol);
    if (index < 0) return;

    // make mesh object from the glyph

    ttf_mesh_t *out;
    if (ttf_glyph2mesh(&font->glyphs[index], &out, TTF_QUALITY_NORMAL, TTF_FEATURES_DFLT) != TTF_DONE)
        return;

    // if successful, release the previous object and save the state

    ttf_free_mesh(mesh);
    glyph = &font->glyphs[index];
    mesh = out;
}

static void on_render()
{
    int width, height;
    glwindow_get_size(&width, &height);

    glwindow_begin_draw();

    glViewport(0, 0, width, height);
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0.0, width, 0.0, height, -1.0, 1.0);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    if (mesh != NULL)
    {
        glTranslatef(width / 2, height / 10, 0);
        glScalef(0.9f * height, 0.9f * height, 1.0f);
        glScalef(1.0f, 1.0f, 0.1f);
        glTranslatef(-(glyph->xbounds[0] + glyph->xbounds[1]) / 2, -glyph->ybounds[0], 0.0f);

        /* Draw wireframe/solid */
        if (view_mode == 0 || view_mode == 1)
        {
            glColor3f(0.0, 0.0, 0.0);
            glPolygonMode(GL_FRONT_AND_BACK, view_mode == 1 ? GL_LINE : GL_FILL);
            glEnableClientState(GL_VERTEX_ARRAY);
            glVertexPointer(2, GL_FLOAT, 0, &mesh->vert->x);
            glDrawElements(GL_TRIANGLES, mesh->nfaces * 3,  GL_UNSIGNED_INT, &mesh->faces->v1);
            glDisableClientState(GL_VERTEX_ARRAY);
        }

        /* Draw contours */
        if (view_mode == 2)
        {
            int i;
            for (i = 0; i < mesh->outline->ncontours; i++)
            {
                glColor3f(0.0, 0.0, 0.0);
                glLineWidth(1.0);
                glEnableClientState(GL_VERTEX_ARRAY);
                glVertexPointer(2, GL_FLOAT, sizeof(ttf_point_t), &mesh->outline->cont[i].pt->x);
                glDrawArrays(GL_LINE_LOOP, 0, mesh->outline->cont[i].length);
                glDisableClientState(GL_VERTEX_ARRAY);
            }
        }
    }

    glwindow_end_draw();
}

static void on_key_event(wchar_t key, const bool ctrl_alt_shift[3], bool pressed)
{
    (void)ctrl_alt_shift;

    if (key == 27 && !pressed) // escape
    {
        glwindow_destroy();
        return;
    }

    if (key == L' ' && pressed)
    {
        //view_mode = view_mode ^ 1;
        view_mode = (view_mode + 1) % 3;
        glwindow_repaint();
        return;
    }

    choose_glyph(key);
    glwindow_repaint();
}

int app_main()
{
    if (!load_system_font())
    {
        fprintf(stderr, "unable to load system font\n");
        return 1;
    }

    choose_glyph(L'A');

    if (!glwindow_create(400, 400, "Press [space] or [A...Z] to select view"))
    {
        fprintf(stderr, "unable to create opengl window\n");
        return 2;
    }

    // init window callbacks
    // and go to event loop

    glwindow_key_cb = on_key_event;
    glwindow_render_cb = on_render;
    glwindow_eventloop();

    return 0;
}
