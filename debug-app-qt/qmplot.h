#ifndef QMPLOT_H
#define QMPLOT_H

#include <math.h>
#include <QStringList>
#include <QWidget>
#include <QFrame>
#include <QLabel>
#include <QGridLayout>
#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QPainter>
#include <QMouseEvent>
#include <QTextDocument>
#include <QPainter>
#include <QColor>
#include <float.h>
#include "linalg.h"
#include "qmwrap.h"

/*
Функциональность
    + нет режима масштаб/сдвиг
    + нет подписи экспоненты
    + нет объектов text()
    + нет легенды
    + нет подписи осей
    + нет перспективы
    - нет формульного рендера
    - нет полноценных фигур
    - нет data aspect ratio
    - нет стиля маркера
    - нет растрового рендера
    - нет polar grid
Визуализация
    - много знаков после запятой
    - непостоянство градуеровки лево/право
*/

#if (__cplusplus < 201103L) && !defined(nullptr)
#define nullptr NULL
#endif

class QMAxes;
class QMWidget;
class QMLegendWidget;

using namespace qmwrap;

typedef Range<double> QMRange;

class QMAxis
{
public:
    QMAxis();
    QString label;
    QMRange manualLim; ///< user defined limits
    QMRange preferredLim; ///< auto limits
    QMRange lim; ///< current limits
    bool logScale;
    bool autoLim;
    bool autoTickLabels;
    bool grid;
    bool gridMinor;
    double tickLabelRotation;
    int exp;
    dvec ticks;
    QStringList tickLabels;
    void adaptate(int nticks, bool roundLimits);
};

class QMText
{
public:
    QMText();
    QMText(const Vec3d &pos, const QString &str,
           Qt::Alignment al = Qt::AlignHCenter | Qt::AlignBottom,
           const QColor &c = QColor(Qt::black));
    QMText(float x, float y, float z, const QString &str,
           Qt::Alignment al = Qt::AlignHCenter | Qt::AlignBottom,
           const QColor &c = QColor(Qt::black));
    Qt::Alignment align;
    QString string;
    QFont font;
    QColor color;
    Vec3d position;
    float rotation;
    bool visible;
};

class QMCamera
{
public:
    QMCamera();
    bool persp;
    bool autoMode;
    float viewAngle;
    Vec3d position;
    Vec3d upVector;
    Vec3d target;
    double nearPlane;
    double farPlane;
    // camera position in spherical
    // coordinate system and zoom,
    // for camera auto mode only:
    float azimuth;
    float elevation;
};

class QMLineSpec
{
public:
    enum Style
    {
        None,
        Solid,  // _____
        Dashed, // - - -
        Dotted, // .....
        DashDot // .-.-.
    };
    QMLineSpec();
    QMLineSpec(const QString &linespec);
    int width;
    Style style; // line style
    char marker; // + o * . x s d ^ v > < p h
    QColor color;
    void setLineSpec(const QString &linespec);
};

class QMGraphic
{
public:
    QMGraphic(QMAxes &parent);
    virtual ~QMGraphic();
    dvec x;
    dvec y;
    dvec z;
    bool visible;
    bool focusable;
    QMLineSpec linespec;
    QMAxes &parent();
    const QMAxes &parent() const;
    virtual bool isEmpty() const;
    virtual void dataChanged();
    virtual void setFocus(const QMRange &x, const QMRange &y, const Mat4d &modelView,
                          const Mat4d &projection, int w, int h, bool concatenation);
protected:
    friend class QMAxes;
    QMAxes &prn;
    void bounds(double x[2], double y[2], double z[2]);
    virtual void draw(QMAxes &axes) = 0;
    double xb[2];
    double yb[2];
    double zb[2];
};

class QMLines: public QMGraphic
{
public:
    explicit QMLines(QMAxes &parent);
    ~QMLines();
    void dataChanged();
    void draw(QMAxes &axes);
    void setFocus(const QMRange &x, const QMRange &y, const Mat4d &modelView,
                  const Mat4d &projection, int w, int h, bool concatenation);
    bvec focused;
protected:
    double *cache;
    size_t cacheSize;
    double *focusCache;
    size_t focusCacheSize;
};

class QMBar: public QMGraphic
{
public:
    explicit QMBar(QMAxes &parent);
    void dataChanged();
    void draw(QMAxes &axes);
    QVector<QColor> barColors;
protected:
    double minDelta;
    double *cach;
    int cachsize;
};

class QMAxes
{
public:
    QMAxes();
    QMText title;
    QMAxis xAxis;
    QMAxis yAxis;
    QMAxis zAxis;
    QMCamera camera;
    QVector<QMText> text;
    QVector<QMGraphic *> graphics;
    QColor bgColor;
    QFont axisLabelFont;
    QFont axisTickFont;
    Qt::Corner axisTicksCorner;
    bool hold;
    void clear();

    class VPFrame
    {
    public:
        QPoint point[2];
        QMLineSpec linespec;
        bool visible;
        VPFrame() : visible(false) {}
    };
    VPFrame zoomRect;
    VPFrame focusRect;

protected:
    class TextOutHint
    {
    public:
        QString str;
        Vec2d mid;
        QSizeF sz;
    };

    class LabelHint
    {
    public:
        inline LabelHint() : hor(true), fitness(0) {}
        inline LabelHint(bool hor, const QSizeF &sz, const Vec2d &mid, float fitness = 0) :
            hor(hor), sz(sz), mid(mid), fitness(fitness) {}
        bool hor;
        QSizeF sz;
        Vec2d mid;
        float fitness;
        QString label;
    };

    class FgEdge
    {
    public:
        bool valid;
        int cubeEdge;
        int cubeVert[2];
        QMAxis *a;
        Vec2d cubeLine[2];
        Vec2d line[2];
        Vec2d lineDir;
        Vec2d lineNormal;
        float labelsPadding;
        float size;
        LabelHint lh;
    };

    class TickHint
    {
    public:
        FgEdge *edge;
        Vec2d line[2];
        QString *label;
        Vec2d mid;
        QSizeF sz;
    };

    class GridHint
    {
    public:
        int quad;
        int axis;
        int edge[2];
        int tick;
        Vec2d line[2];
    };

    class TitleHint
    {
    public:
        Vec2d mid;
        Vec2d mod;
        QSizeF sz;
    };

    FgEdge fge[3];

    Mat4d M;
    Mat4d V;
    Mat4d P;
    TitleHint titleHint;
    QVector<GridHint> gridHints;
    QVector<TickHint> tickHints;
    // LabelHint labelHints[3];
    Vec3d vproj[8 + 1];
    bool bgQuads[6];
    bool bgEdges[12];
    void updateAxes();
    void calcModelMatrix();
    void calcViewMatrix();
    void calcProjMatrix(int w, int h, int margin_lrud[4]);
    void placeCamera();
    void calcCubeProj(int w, int h);
    void placeBgQuads(int w, int h);
    void placeBgEdges();
    void placeTicks();
    void placeTickLabels();
    float axisEdgeFitness(int edge, const Vec3d proj[8]);
    void getNicestAxesEdges(int edges[3]);
    void placeFgEdges();
    float labelFitness(QSizeF sz, Vec2d mid, int w, int h);
    void placeAxisLabel(FgEdge &e, const QString &s, int w, int h);
    void placeAxesLabels(int w, int h);
    void placeGrids(int q, int e1, int e2, int w, int h);
    void placeGrids(int w, int h);
    void placeTitle(int w, int h);
    QRectF oob(int w, int h);
    void prepareToDraw(int w, int h);
    void draw(int w, int h, QPaintDevice *dev);
    void drawCube(int w, int h);
    void drawVPFrames(int w, int h);
    void outCamParams(QPaintDevice *dev, int w, int h);
    void outTextLabels(QPaintDevice *dev, int w, int h);
};

//class QMMouseTool: public QObject
//{
//public:
//    explicit QMMouseTool(QMWidget &widget, Qt::MouseButtons btn, Qt::KeyboardModifiers mod = 0, bool enable = true);
//    virtual ~QMMouseTool();
//    inline bool isEnabled() const { return enable; }
//    inline bool isActive() const { return active; }
//    inline Qt::KeyboardModifiers startModifiers() const { return mod; }
//    inline Qt::MouseButtons startButtons() const { return btn; }
//    void setEnable(bool enable);
//    void setCondition(Qt::MouseButtons btn, Qt::KeyboardModifiers mod = 0);
//protected:
//    bool enable;
//    bool active;
//    Qt::MouseButtons btn;
//    Qt::KeyboardModifiers mod;
//    inline QMWidget *p() { return (QMWidget *)parent(); }
//    bool eventFilter(QObject *watched, QEvent *event);
//    virtual bool moveFilteredEvent(QMouseEvent *) { return false; }
//    virtual bool mouseFilteredEvent(QMouseEvent *) { return false; }
//    virtual bool wheelFilteredEvent(QWheelEvent *) { return false; }
//};

//class QMZoomRectTool: public QMMouseTool
//{
//public:
//    explicit QMZoomRectTool(QMWidget &widget, Qt::MouseButtons btn = Qt::LeftButton,
//                            Qt::KeyboardModifiers mod = 0, bool enable = true);
//protected:
//    virtual bool mouseFilteredEvent(QEvent *event);
//};

class QMController: public QObject
{
    Q_OBJECT
public:
    explicit QMController(QMWidget *parent);
    virtual ~QMController();
    virtual void mousePressEvent(QMouseEvent *event) = 0;
    virtual void mouseReleaseEvent(QMouseEvent *event) = 0;
    virtual void mouseMoveEvent(QMouseEvent *event) = 0;
    virtual void wheelEvent(QWheelEvent *event) = 0;
signals:
    void mouseMoving(QMouseEvent *event);
};

class QMDefController: public QMController
{
public:
    QMDefController(QMWidget *parent);
    virtual void mousePressEvent(QMouseEvent *event);
    virtual void mouseReleaseEvent(QMouseEvent *event);
    virtual void mouseMoveEvent(QMouseEvent *event);
    virtual void wheelEvent(QWheelEvent *event);
protected:
    enum State { None, Rot, Zoom, Focus, MoveStarting, Move };
    State state;
    QPoint pressedPos;
};

class QMLegendItem: public QWidget
{
public:
    explicit QMLegendItem(QWidget *parent = nullptr);
    inline const QString text() const { return _ltext->text(); }
    inline const QFont &font() const { return _ltext->font(); }
    inline const QMLineSpec &linespec() const { return _ls; }
    void setText(const QString &text);
    void setFont(const QFont &font);
    void setLinespec(const QMLineSpec &linespec);
private:
    QMLineSpec _ls;
    QLabel *_licon;
    QLabel *_ltext;
    //QWidget _inserted;
};

class QMLegendWidget: public QFrame
{
public:
    explicit QMLegendWidget(QWidget &parent);
    inline const QMLegendItem *item(int index) const { return _items[index]; }
    inline QMLegendItem *item(int index) { return _items[index]; }
    QMLegendItem *addItem(const QMLineSpec &linespec = QMLineSpec(), const QString &text = QString());
    void removeItem(int index);
    void clear();
    inline Qt::Corner bindingCorner() const { return bc; }
    inline QPoint bindingPosition() const { return bp; }
    void setBindingCorner(Qt::Corner corner);
    void setBindingPosition(const QPoint &pos);
    inline void move(int x, int y) { move(QPoint(x, y)); }
    void move(const QPoint &);
protected:
    void mousePressEvent(QMouseEvent *e);
    void mouseReleaseEvent(QMouseEvent *e);
    void mouseMoveEvent(QMouseEvent *e);
    bool eventFilter(QObject *o, QEvent *e);
    void resizeEvent(QResizeEvent *);
    void bindCurrentPos();
    void moveToBindingPos();

private:
    Qt::Corner bc;
    QPoint bp;
    bool moving;
    QPoint ppos;
    QPoint wpos;
    QVector<QMLegendItem *> _items;
};

class QMWidget: public QOpenGLWidget, public QMAxes
{
    Q_OBJECT
public:
    QMWidget(QWidget *parent = nullptr);
    ~QMWidget();
    QMLegendWidget legend;
    void fillLegend(const QStringList &text);
    void ctlZoom(const QRect &rect);
    void ctlZoom(const QPoint &at, float ticks);
    void ctlMove(int dx, int dy);
    void ctlFocus(const QRect &rect, bool concatenation = true);
    void unproject(const QPoint &pt, Vec3d *outPoint, Vec3d *outDir);
    inline QMController *controller() { return ctl; }
signals:
    void dataFocused();
protected:
    QMController *ctl;
    QMDefController *dc;
    int cubeIntersections(Vec3d *res, const Vec3d &d, const Vec3d &p);
    int cubeIntersections(Vec3d *res, const QPoint &p);
    void initializeGL();
    void resizeGL(int w, int h);
    void paintGL();
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void wheelEvent(QWheelEvent *event);
};

namespace qmplot {

QWidget *gcf();
QMWidget *gca(QMWidget *setTo = nullptr);
QMWidget *axes(QWidget *parent = nullptr);
void hold(bool on);
void cla();
void xlim(double min, double max, bool automode = false);
void ylim(double min, double max, bool automode = false);
void zlim(double min, double max, bool automode = false);
QMLines *plot(const QString &fmt = "-");
QMLines *plot(int count, const float *x, const float *y = nullptr, const float *z = nullptr, const QString &fmt = "-", int xstride = 4, int ystride = 4, int zstride = 4);
QMLines *plot(int count, const double *x, const double *y = nullptr, const double *z = nullptr, const QString &fmt = "-", int xstride = 8, int ystride = 8, int zstride = 8);
void view(float az, float el);
QMText *text(float x, float y, float z, const QString &string);
void title(const QString &string, Qt::Alignment alignment = Qt::AlignHCenter | Qt::AlignTop);
QMLegendWidget *legend(const QStringList &items);
void xlabel(const QString &string);
void ylabel(const QString &string);
void zlabel(const QString &string);

template <typename T>
void hist(const Vec<T> &data, Vec<T> &bins, Vec<T> &freq, int nbins = 0, bool normalize = false)
{
    T min, max;
    bins.clear();
    freq.clear();
    if (data.count() == 0) return;
    if (nbins <= 0)
    {
        if (data.count() < 10)
            nbins = data.count();
        else
        if (data.count() < 1000)
            nbins = 10;
        else
        if (data.count() < 10000)
            nbins = 20;
        else
            nbins = 40;
    }
    minmax_finit(data, min, max);
    if (min == max || nbins == 1)
    {
        bins.push_back((min + max) / 2);
        freq.push_back(normalize ? 1 : data.count());
        return;
    }
    /*

        min            max
         |              |
       [ 0 ][ 1 ][ 2 ][ 3 ]     - nbins = 4

    */
    T step = (max - min) / (nbins - 1);
    bins.resize(nbins);
    freq.resize(nbins);
    for (int i = 0; i < nbins; i++)
    {
        bins[i] = min + step * i;
        freq[i] = 0;
    }
    T offset = min - step / 2;
    // S = binwidth * count = step * count()
    //T inc = normalize ? 1.0 / data.count() : 1;
    T inc = normalize ? 1.0 / (step * data.count()) : 1;
    for (int i = 0; i < data.count(); i++)
    {
        int j = trunc((data[i] - offset) / step);
        if (j < 0) j = 0;
        if (j >= nbins) j = nbins - 1;
        freq[j] += inc;
    }
}

template <typename T>
QMGraphic *hist(const Vec<T> &data, int nbins, bool normalize = false)
{
    QMWidget *a = gca();
    if (a == nullptr) a = axes(nullptr);
    if (a == nullptr) return nullptr;
    if (!a->hold) a->clear();
    QMBar *b = new QMBar(*a);
    hist(dvec(data), b->x, b->y, nbins, normalize);
    b->dataChanged();
    a->update();
    return b;
}

template <typename Tx>
QMLines *plot(const Vec<Tx> &y, const QString &fmt = "-", double xadd = 0, double xmul = 1.0)
{
    QMLines *res = plot(fmt);
    if (res == nullptr) return nullptr;
    res->y = y;
    res->x.resize(y.count());
    for (int i = 0; i < y.count(); i++)
        res->x[i] = xmul * i + xadd;
    res->dataChanged();
    gca()->update();
    return res;
}

template <typename Tx, typename Ty>
QMLines *plot(const Vec<Tx> &x, const Vec<Ty> &y, const QString &fmt = "-")
{
    QMLines *res = plot(fmt);
    if (res == nullptr) return nullptr;
    res->x = x;
    res->y = y;
    res->dataChanged();
    gca()->update();
    return res;
}

template <typename Tx, typename Ty, typename Tz>
QMLines *plot(const Vec<Tx> &x, const Vec<Ty> &y, const Vec<Tz> &z = Vec<Tz>(), const QString &fmt = "-")
{
    QMLines *res = plot(fmt);
    if (res == nullptr) return nullptr;
    res->x = x;
    res->y = y;
    res->z = z;
    res->dataChanged();
    gca()->update();
    return res;
}


}

#endif // QMPLOT_H
