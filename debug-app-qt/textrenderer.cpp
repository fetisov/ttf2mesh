#include "textrenderer.h"

TextRenderer::TextRenderer(QWidget *parent) :
    QOpenGLWidget(parent), text("Text"), font(NULL), textHeight(12), meshQuality(16), numTriangles(0)
{
    QSurfaceFormat format;
    format.setDepthBufferSize(24);
    format.setStencilBufferSize(8);
    format.setSamples(32);
    format.setProfile(QSurfaceFormat::CoreProfile);
    setFormat(format);
}

void TextRenderer::drawGlyph(ttf_mesh_t *mesh)
{
    glColor3f(0.0, 0.0, 0.0);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(2, GL_FLOAT, 0, &mesh->vert->x);
    glDrawElements(GL_TRIANGLES, mesh->nfaces * 3,  GL_UNSIGNED_INT, &mesh->faces->v1);
    glDisableClientState(GL_VERTEX_ARRAY);
}

void TextRenderer::initializeGL()
{
    glDisable(GL_DEPTH_TEST);
    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);
    glEnable(GL_MULTISAMPLE);
}

void TextRenderer::paintGL()
{
    int w = width();
    int h = height();
    numTriangles = 0;

    glViewport(0, 0, w, h);
    glClearColor(1, 1, 1, 1);
    glClear(GL_COLOR_BUFFER_BIT);

    if (font == NULL || text.isEmpty()) return;

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, w, -h, 0, -1, 1);

    float x = 0;
    float y = -1;

    for (int i = 0; i < text.length(); i++)
    {
        uint16_t c = text[i].unicode();
        if (c == '\n')
        {
            x = 0;
            y -= 1;
            continue;
        }
        int index = ttf_find_glyph(font, c);
        if (index < 0) continue;
        ttf_glyph_t *g = font->glyphs + index;
        ttf_mesh_t *m;
        ttf_glyph2mesh(g, &m, meshQuality, 0);
        if (m != NULL)
        {
            glMatrixMode(GL_MODELVIEW);
            glLoadIdentity();
            glScalef(textHeight, textHeight, textHeight);
            glTranslatef(x, y, 0);
            drawGlyph(m);
            numTriangles += m->nfaces;
            ttf_free_mesh(m);
        }
        x += g->advance;
    }

}
