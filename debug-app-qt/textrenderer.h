#ifndef TEXTRENDERER_H
#define TEXTRENDERER_H

#include <QWidget>
#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include "ttf2mesh.h"

class TextRenderer : public QOpenGLWidget
{
    Q_OBJECT
public:
    explicit TextRenderer(QWidget *parent = nullptr);
    QString text;
    ttf_t *font;
    int textHeight;
    int meshQuality;

    int numTriangles;

protected:
    void drawGlyph(ttf_mesh_t *mesh);
    void initializeGL();
    void paintGL();

};

#endif // TEXTRENDERER_H
