#define _CRT_SECURE_NO_WARNINGS

#include <math.h>
#include <assert.h>
#include <stdio.h>
#include <time.h>
#include <math.h>
#include "ttf2mesh.h"
#include "glwindow.h"

static ttf_t *font = NULL;
static ttf_glyph_t *glyph = NULL;
static ttf_mesh3d_t *mesh = NULL;

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

    // make 3d object from the glyph

    ttf_mesh3d_t *out;
    if (ttf_glyph2mesh3d(&font->glyphs[index], &out, TTF_QUALITY_NORMAL, TTF_FEATURES_DFLT, 0.1f) != TTF_DONE)
        return;

    // if successful, release the previous object and save the state

    ttf_free_mesh3d(mesh);
    glyph = &font->glyphs[index];
    mesh = out;
}

static void set_light()
{
    static const float light0_ambient[4]  = {0.0f, 0.0f, 0.0f, 1.0f};
    static const float light0_diffuse[4]  = {1.0f, 1.0f, 1.0f, 1.0f};
    static const float light0_specular[4] = {0.0f, 0.0f, 0.0f, 1.0f};
    static const float light0_position[4] = {0.0f, 0.0f, 1.0f, 0.0f};
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glLightfv(GL_LIGHT0, GL_AMBIENT, light0_ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light0_diffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, light0_specular);
    glLightfv(GL_LIGHT0, GL_POSITION, light0_position);
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

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_COLOR_MATERIAL);
    glShadeModel(GL_SMOOTH);

    set_light();

    if (mesh != NULL)
    {
        glTranslatef(width / 2, height / 10, 0);
        glScalef(0.9f * height, 0.9f * height, 1.0f);

        float angle = 1e-3f * glwindow_time_ms() * 90;
        glRotatef(angle, 0.0f, 1.0f, 0.0f);
        glRotatef(-20, 1.0f, 0.0f, 0.0f);
        glTranslatef(-(glyph->xbounds[0] + glyph->xbounds[1]) / 2, -glyph->ybounds[0], 0.0f);

        glColor3f(0.7, 0.7, 0.7);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glEnableClientState(GL_VERTEX_ARRAY);
        glVertexPointer(3, GL_FLOAT, 0, &mesh->vert->x);
        glEnableClientState(GL_NORMAL_ARRAY);
        glNormalPointer(GL_FLOAT, 0, &mesh->normals->x);
        glDrawElements(GL_TRIANGLES, mesh->nfaces * 3,  GL_UNSIGNED_INT, &mesh->faces->v1);

        glColor3f(0, 0, 1);
        glLineWidth(1.0f);
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glEnable(GL_POLYGON_OFFSET_LINE);
        glPolygonOffset(0, -1000.0);
        glDrawElements(GL_TRIANGLES, mesh->nfaces * 3,  GL_UNSIGNED_INT, &mesh->faces->v1);
        glDisable(GL_POLYGON_OFFSET_LINE);

        glDisableClientState(GL_NORMAL_ARRAY);
        glDisableClientState(GL_VERTEX_ARRAY);
    }

    glwindow_end_draw();
}

static void on_key_event(wchar_t key, const bool ctrl_alt_shift[3], bool pressed)
{
    if (key == 27 && !pressed) // escape
    {
        glwindow_destroy();
        return;
    }

    if (pressed)
    {
        if (key >= 'a' && key <= 'z' && ctrl_alt_shift[2])
            key = key - 'a' + 'A';
        choose_glyph(key);
        glwindow_repaint();
    }
}

int app_main()
{
    if (!load_system_font())
    {
        fprintf(stderr, "unable to load system font\n");
        return 1;
    }

    choose_glyph(L'B');

    if (!glwindow_create(400, 400, "Press [A...Z] to select glyph"))
    {
        fprintf(stderr, "unable to create opengl window\n");
        return 2;
    }

    // init window callbacks
    // and go to event loop

    glwindow_key_cb = on_key_event;
    glwindow_render_cb = on_render;
    glwindow_timer_cb = on_render;
    glwindow_set_timeout(1);
    glwindow_eventloop();

    return 0;
}
