#include "qmplot.h"
#include "linalg.h"
#include <QDebug>

/*
Cube representation.

vertex indeces:
     ^ y
     |

5 _______ 4
 /      /|
/______/ |    -> x
|1    0| |7
|      | /
|2____3|/

   /
    z

edges:
       ^ y
       |

    ___1_____
   /|       /|
 10 7     11 6
 /__|_0___/  |
|   | _ _ |_ |   -> x
4  /   2  5  /
| /9      | /8
|/____3___|/

    /
    z

quads:
   ____1__
  /  4   /|
 /______/ |
3|      |2|
 |  0   | /
 |______|/
     5
*/

static const struct
{
    double v[8][3];
    double edge2pos[12][3];
    int quad2v[6][4];
    int edge2v[12][2];
    int edge2axis[12];
    int edge2quad[12][2];
    int edge2ortho[12][4];
    int v2edge[8][3];
    int quad2edge[6][4];
    double quadn[6][3];
} cube =
{
    {
        { 0.5,  0.5,  0.5}, // 0
        {-0.5,  0.5,  0.5}, // 1
        {-0.5, -0.5,  0.5}, // 2
        { 0.5, -0.5,  0.5}, // 3
        { 0.5,  0.5, -0.5}, // 4
        {-0.5,  0.5, -0.5}, // 5
        {-0.5, -0.5, -0.5}, // 6
        { 0.5, -0.5, -0.5}  // 7
    },
    {
        {0, 0.5, 0.5}, {0, 0.5, -0.5}, {0, -0.5, -0.5}, {0, -0.5, 0.5},
        {-0.5, 0, 0.5}, {0.5, 0, 0.5}, {0.5, 0, -0.5}, {-0.5, 0, -0.5},
        {0.5, -0.5, 0}, {-0.5, -0.5, 0}, {-0.5, 0.5, 0}, {0.5, 0.5, 0}
    },
    {
        {0, 1, 2, 3}, // front
        {7, 6, 5, 4}, // back
        {4, 0, 3, 7}, // right
        {5, 6, 2, 1}, // left
        {4, 5, 1, 0}, // top
        {7, 3, 2, 6}  // bottom
    },
    {
        {1, 0}, {5, 4}, {6, 7}, {2, 3}, // x axes
        {2, 1}, {3, 0}, {7, 4}, {6, 5}, // y axes
        {7, 3}, {6, 2}, {5, 1}, {4, 0}  // z axes
    },
    { 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2 },
    {
        {0, 4}, {1, 4}, {1, 5}, {0, 5},
        {0, 3}, {0, 2}, {1, 2}, {1, 3},
        {2, 5}, {3, 5}, {3, 4}, {2, 4}
    },
    {
        {11, 5, 10, 4}, {11, 6, 10, 7}, {6, 8, 7, 9},  {5, 8, 4, 9},
        {0, 10, 3, 9},  {0, 11, 3, 8},  {1, 11, 2, 8}, {1, 10, 2, 9},
        {5, 3, 6, 2},   {4, 3, 7, 2},   {1, 7, 0, 4},  {1, 6, 0, 5}
    },
    {
        {0, 5, 11}, {0, 4, 10}, {3, 4, 9}, {3, 5, 8},
        {1, 6, 11}, {1, 7, 10}, {2, 7, 9}, {2, 6, 8}
    },
    {
        {0, 3, 4, 5}, {1, 2, 6, 7}, {5, 6, 8, 11},
        {4, 7, 9, 10}, {0, 1, 10, 11}, {2, 3, 8, 9}
    },
    {
        {0, 0, +1}, {0, 0, -1},
        {+1, 0, 0}, {-1, 0, 0},
        {0, +1, 0}, {0, -1, 0}
    }
};

//class Cube
//{
//public:
//    class V;
//    class E;
//    class Q;
//    class V
//    {
//    public:
//        inline V(int index) : index(index) {}
//        int index;
//    };
//    class E
//    {
//    public:
//        inline E(int index) : index(index) {}
//        inline V v1() const { return cube.edge2v[index][0]; }
//        inline V v2() const { return cube.edge2v[index][0]; }
//        inline E neighbor(int index0to3) const { return cube.edge2ortho[index][index0to3]; }
//        inline Q quad(int index0to1) const { return cube.edge2quad[index][index0to1]; }
//        int index;
//    };
//};


// sprintf('{%.3f, %.3f, %.3f},\n', lines')
static float colormapLines[][3] =
{
    {0.000f, 0.447f, 0.741f},
    {0.850f, 0.325f, 0.098f},
    {0.929f, 0.694f, 0.125f},
    {0.494f, 0.184f, 0.556f},
    {0.466f, 0.674f, 0.188f},
    {0.301f, 0.745f, 0.933f},
    {0.635f, 0.078f, 0.184f},
    {0.000f, 0.447f, 0.741f},
    {0.850f, 0.325f, 0.098f},
    {0.929f, 0.694f, 0.125f},
    {0.494f, 0.184f, 0.556f},
    {0.466f, 0.674f, 0.188f},
    {0.301f, 0.745f, 0.933f},
    {0.635f, 0.078f, 0.184f},
    {0.000f, 0.447f, 0.741f},
    {0.850f, 0.325f, 0.098f},
    {0.929f, 0.694f, 0.125f},
    {0.494f, 0.184f, 0.556f},
    {0.466f, 0.674f, 0.188f},
    {0.301f, 0.745f, 0.933f},
    {0.635f, 0.078f, 0.184f},
    {0.000f, 0.447f, 0.741f},
    {0.850f, 0.325f, 0.098f},
    {0.929f, 0.694f, 0.125f},
    {0.494f, 0.184f, 0.556f},
    {0.466f, 0.674f, 0.188f},
    {0.301f, 0.745f, 0.933f},
    {0.635f, 0.078f, 0.184f},
    {0.000f, 0.447f, 0.741f},
    {0.850f, 0.325f, 0.098f},
    {0.929f, 0.694f, 0.125f},
    {0.494f, 0.184f, 0.556f},
    {0.466f, 0.674f, 0.188f},
    {0.301f, 0.745f, 0.933f},
    {0.635f, 0.078f, 0.184f},
    {0.000f, 0.447f, 0.741f},
    {0.850f, 0.325f, 0.098f},
    {0.929f, 0.694f, 0.125f},
    {0.494f, 0.184f, 0.556f},
    {0.466f, 0.674f, 0.188f},
    {0.301f, 0.745f, 0.933f},
    {0.635f, 0.078f, 0.184f},
    {0.000f, 0.447f, 0.741f},
    {0.850f, 0.325f, 0.098f},
    {0.929f, 0.694f, 0.125f},
    {0.494f, 0.184f, 0.556f},
    {0.466f, 0.674f, 0.188f},
    {0.301f, 0.745f, 0.933f},
    {0.635f, 0.078f, 0.184f},
    {0.000f, 0.447f, 0.741f},
    {0.850f, 0.325f, 0.098f},
    {0.929f, 0.694f, 0.125f},
    {0.494f, 0.184f, 0.556f},
    {0.466f, 0.674f, 0.188f},
    {0.301f, 0.745f, 0.933f},
    {0.635f, 0.078f, 0.184f},
    {0.000f, 0.447f, 0.741f},
    {0.850f, 0.325f, 0.098f},
    {0.929f, 0.694f, 0.125f},
    {0.494f, 0.184f, 0.556f},
    {0.466f, 0.674f, 0.188f},
    {0.301f, 0.745f, 0.933f},
    {0.635f, 0.078f, 0.184f},
    {0.000f, 0.447f, 0.741f},
};

static inline bool planeAndLineIntersection(const Vec3d &d, const Vec3d &p, const Vec3d &n, double D, Vec3d *x = nullptr, double *c = nullptr)
{
    vec4d plane = {n.x(), n.y(), n.z(), D};
    return v3d_lnp_intersection(x->v, d.v, p.v, plane, c);
}

static inline bool inrange2(double val1, double val2, const QMRange &range1, const QMRange &range2)
{
    return range1.in(val1) && range2.in(val2);
}

void qMinMax(const Vec3d *v, int count, Vec3d &min, Vec3d &max)
{
    min = Vec3d(DBL_MAX, DBL_MAX, DBL_MAX);
    max = Vec3d(-DBL_MAX, -DBL_MAX, -DBL_MAX);
    for (int i = 0; i < count; i++)
    {
        if (v[i].v[0] > max.v[0]) max.v[0] = v[i].v[0];
        if (v[i].v[1] > max.v[1]) max.v[1] = v[i].v[1];
        if (v[i].v[2] > max.v[2]) max.v[2] = v[i].v[2];
        if (v[i].v[0] < min.v[0]) min.v[0] = v[i].v[0];
        if (v[i].v[1] < min.v[1]) min.v[1] = v[i].v[1];
        if (v[i].v[2] < min.v[2]) min.v[2] = v[i].v[2];
    }
    if (min.v[0] == DBL_MAX) min.v[0] = 0;
    if (min.v[1] == DBL_MAX) min.v[1] = 0;
    if (min.v[2] == DBL_MAX) min.v[2] = 0;
    if (max.v[0] == -DBL_MAX) max.v[0] = 0;
    if (max.v[1] == -DBL_MAX) max.v[1] = 0;
    if (max.v[2] == -DBL_MAX) max.v[2] = 0;
}

QMCamera::QMCamera() :
    persp(false), autoMode(true), viewAngle(15),
    nearPlane(0), farPlane(0), azimuth(0),
    elevation(90)
{
}

QMGraphic::QMGraphic(QMAxes &parent) : prn(parent)
{
    visible = true;
    focusable = false;
    xb[0] = -1; xb[1] = 1;
    yb[0] = -1; yb[1] = 1;
    zb[0] = -1; zb[1] = 1;
    const float *c = colormapLines[parent.graphics.count() & 63];
    linespec.color = QColor::fromRgbF(c[0], c[1], c[2]);
    parent.graphics.push_back(this);
}

QMGraphic::~QMGraphic()
{
    prn.graphics.removeOne(this);
}

QMAxes &QMGraphic::parent()
{
    return prn;
}

const QMAxes &QMGraphic::parent() const
{
    return prn;
}

bool QMGraphic::isEmpty() const
{
    return x.count() == 0 && y.count() == 0 && z.count() == 0;
}

void QMGraphic::dataChanged()
{
    qmwrap::minmax_finit(this->x, xb[0], xb[1]);
    qmwrap::minmax_finit(this->y, yb[0], yb[1]);
    qmwrap::minmax_finit(this->z, zb[0], zb[1]);
}

void QMGraphic::bounds(double x[2], double y[2], double z[2])
{
    x[0] = xb[0]; x[1] = xb[1];
    y[0] = yb[0]; y[1] = yb[1];
    z[0] = zb[0]; z[1] = zb[1];
}

void QMGraphic::setFocus(const QMRange &, const QMRange &, const Mat4d &, const Mat4d &, int, int, bool)
{
    // do nothing
}

QMAxes::QMAxes()
{
    hold = false;
    title.font = QFont("Times New Roman", 14, QFont::Bold);
    title.align = Qt::AlignTop | Qt::AlignHCenter;
    bgColor = QColor(Qt::white);
    zoomRect.linespec.setLineSpec(":#9");
    focusRect.linespec.setLineSpec("-r");
    axisTickFont = QFont("Times", 9, QFont::Normal);
    axisLabelFont = QFont("Times New Roman", 10, QFont::Normal);
    axisTicksCorner = Qt::BottomLeftCorner;
}

void QMAxes::clear()
{
    for (int i = 0; i < graphics.count(); i++)
        delete graphics[i];
    graphics.clear();
}

void QMAxes::updateAxes()
{
    // recalc bounds
    double xlim[2] = {-1, 1};
    double ylim[2] = {-1, 1};
    double zlim[2] = {-1, 1};
    if (!graphics.empty())
    {
        bool initial = true;
        for (int i = 0; i < graphics.count(); i++)
        {
            double tmp[6];
            if (!graphics[i]->visible || graphics[i]->isEmpty()) continue;
            graphics[i]->bounds(tmp + 0, tmp + 2, tmp + 4);
            if (initial)
            {
                xlim[0] = tmp[0];
                xlim[1] = tmp[1];
                ylim[0] = tmp[2];
                ylim[1] = tmp[3];
                zlim[0] = tmp[4];
                zlim[1] = tmp[5];
                initial = false;
                continue;
            }
            xlim[0] = qMin(xlim[0], tmp[0]);
            ylim[0] = qMin(ylim[0], tmp[2]);
            zlim[0] = qMin(zlim[0], tmp[4]);
            xlim[1] = qMax(xlim[1], tmp[1]);
            ylim[1] = qMax(ylim[1], tmp[3]);
            zlim[1] = qMax(zlim[1], tmp[5]);
        }
        if (xlim[1] - xlim[0] < 1e-20) { xlim[0] -= 1; xlim[1] += 1; }
        if (ylim[1] - ylim[0] < 1e-20) { ylim[0] -= 1; ylim[1] += 1; }
        if (zlim[1] - zlim[0] < 1e-20) { zlim[0] -= 1; zlim[1] += 1; }
    }

    // how many grid lines?
    double len[3] = {0, 0, 0};

    len[cube.edge2axis[fge[0].cubeEdge]] = (fge[0].line[1] - fge[0].line[0]).len();
    len[cube.edge2axis[fge[1].cubeEdge]] = (fge[1].line[1] - fge[1].line[0]).len();
    len[cube.edge2axis[fge[2].cubeEdge]] = (fge[2].line[1] - fge[2].line[0]).len();

    // set axes
    xAxis.preferredLim[0] = xlim[0]; xAxis.preferredLim[1] = xlim[1];
    yAxis.preferredLim[0] = ylim[0]; yAxis.preferredLim[1] = ylim[1];
    zAxis.preferredLim[0] = zlim[0]; zAxis.preferredLim[1] = zlim[1];

    xAxis.adaptate(len[0] < 5 ? 0 : qMax(static_cast<int>(len[0] / 40), 2), xAxis.autoLim);
    yAxis.adaptate(len[1] < 5 ? 0 : qMax(static_cast<int>(len[1] / 40), 2), yAxis.autoLim);
    zAxis.adaptate(len[2] < 5 ? 0 : qMax(static_cast<int>(len[2] / 20), 2), zAxis.autoLim);
}

void QMAxes::calcModelMatrix()
{
    M =
        Mat4d::scale(1.0 / (xAxis.lim[1] - xAxis.lim[0]),
            1.0 / (yAxis.lim[1] - yAxis.lim[0]),
            1.0 / (zAxis.lim[1] - zAxis.lim[0])) *
        Mat4d::translate(-(xAxis.lim[0] + xAxis.lim[1]) / 2,
            -(yAxis.lim[0] + yAxis.lim[1]) / 2,
            -(zAxis.lim[0] + zAxis.lim[1]) / 2);
}

Vec2d placeNearest(const Vec2d boundDir, const Vec2d middleDir, const QSizeF &size,
    float preferredDistByDir = 0, double minDistFromBoundLine = 0, double *maxDistFromBoundLine = nullptr)
{
    Mat2d m = Mat2d(middleDir, Vec2d(middleDir.y(), -middleDir.x()));
    Vec2d r[4] =
    {
        Vec2d(-size.width() / 2, +size.height() / 2),
        Vec2d(+size.width() / 2, +size.height() / 2),
        Vec2d(+size.width() / 2, -size.height() / 2),
        Vec2d(-size.width() / 2, -size.height() / 2)
    };
    Vec2d p[4] = { m * r[0], m * r[1], m * r[2], m * r[3] };

    int xmin = 0;
    if (p[1].x() < p[xmin].x()) xmin = 1;
    if (p[2].x() < p[xmin].x()) xmin = 2;
    if (p[3].x() < p[xmin].x()) xmin = 3;
    Vec2d res = middleDir * (preferredDistByDir - p[xmin].x());

/*
    code below is to prevent collision like it:
    |
 tick
   \|
    |
*/

    Vec2d ee1 = Vec2d(boundDir.y(), -boundDir.x());
    if (Vec2d::dot(ee1, middleDir) < 0) ee1 = -ee1;
    m = Mat2d(ee1, boundDir);
    p[0] = m * (r[0] + res);
    p[1] = m * (r[1] + res);
    p[2] = m * (r[2] + res);
    p[3] = m * (r[3] + res);
    int xmax = 0;
    xmin = 0;
    for (int i = 0; i < 3; i++)
    {
        if (p[i].x() < p[xmin].x()) xmin = i;
        if (p[i].x() > p[xmax].x()) xmax = i;
    }
    double xx = 0;
    if (p[xmin].x() < minDistFromBoundLine)
        xx = minDistFromBoundLine - p[xmin].x();
    res += ee1 * xx;
    if (maxDistFromBoundLine != nullptr)
        *maxDistFromBoundLine = xx + p[xmax].x();
    return res;
}

const float tickLen = 15;

void QMAxes::placeTickLabels()
{
    QFontMetrics fm = QFontMetrics(axisTickFont);
    for (int i = 0; i < tickHints.count(); i++)
    {
        TickHint &th = tickHints[i];
        if (th.label == nullptr) continue;
        th.sz = QSizeF(fm.tightBoundingRect(*th.label).size());
        double dist = 0;
        th.mid = th.line[0] + placeNearest(th.edge->lineDir,
                                           (th.line[1] - th.line[0]).normalized() ,
                                           th.sz, tickLen, 2, &dist);
        if (dist > th.edge->labelsPadding)
            th.edge->labelsPadding = static_cast<float>(dist);
    }
}

void QMAxes::placeTicks()
{
    tickHints.clear();
    for (int i = 0; i < 3; i++)
    {
        int e = fge[i].cubeEdge;
        int q1 = cube.edge2quad[e][0];
        int q2 = cube.edge2quad[e][1];
        if (bgQuads[q1] && bgQuads[q2]) continue;
        if (!bgQuads[q1] && !bgQuads[q2]) continue;
        if (!bgQuads[q1]) q1 = q2;
        TickHint th;
        th.edge = fge + i;
        for (int j = 0; j < gridHints.count(); j++)
        {
            GridHint &gh = gridHints[j];
            if (gh.quad != q1) continue;
            if (gh.edge[0] != e && gh.edge[1] != e) continue;
            Vec2d v[2] = {gh.line[0], gh.line[1]};
            if (gh.edge[0] != e) qSwap(v[0], v[1]);
            th.line[0] = v[0];
            th.line[1] = v[0] + (v[0] - v[1]).normalized() * tickLen;
            th.label = gh.tick < fge[i].a->tickLabels.count() ? &fge[i].a->tickLabels[gh.tick] : nullptr;
            tickHints.push_back(th);
        }
    }
}

float QMAxes::axisEdgeFitness(int edge, const Vec3d proj[])
{
    double res = 1.0;
    // Ребро должно быть по границе видимых граней
    bool vis1 = bgQuads[cube.edge2quad[edge][0]];
    bool vis2 = bgQuads[cube.edge2quad[edge][1]];
    if (!vis1 && !vis2) res *= 1e-3; // Совсем плохо
    if (vis1 && vis2) res *= 1e-2; // Нежелательно

    // Нормированные проекции двух вершин ребра
    Vec3d v1 = proj[cube.edge2v[edge][0]];
    Vec3d v2 = proj[cube.edge2v[edge][1]];

    // Видимая длина должна быть максимальная
    res *= (v2 - v1).toVec2d().len() + 0.01;

    Vec3d c = (v1 + v2) / 2;
    if (axisTicksCorner == Qt::TopRightCorner || axisTicksCorner == Qt::BottomRightCorner)
        c.x() = 1.0 - c.x();
    if (axisTicksCorner == Qt::TopRightCorner || axisTicksCorner == Qt::TopLeftCorner)
        c.y() = 1.0 - c.y();
    res *= 2.0 - c.x() * 0.5 - c.y() - c.z() * 0.1;

    return static_cast<float>(res);
}

void QMAxes::getNicestAxesEdges(int edges[])
{
    int e[12 * 2][3];
    float f[12 * 2];
    Vec3d proj[8];

    // Поиск и нормировка проекции вершин куба
    for (int i = 0; i < 8; i++)
        proj[i] = Vec3d(cube.v[i]).project(V, P, 1, 1);
    Vec3d min, max, range;
    qMinMax(proj, 8, min, max);
    range = max - min;
    for (int i = 0; i < 8; i++)
    {
        proj[i] = proj[i] - min;
        proj[i].x() = proj[i].x() / range.x()/* - 0.5*/;
        proj[i].y() = proj[i].y() / range.y()/* - 0.5*/;
        proj[i].z() = proj[i].z() / range.z()/* - 0.5*/;
    }

    // Перебираем все грани
    for (int i = 0; i < 12; i++)
    {
        e[i * 2 + 0][0] = cube.edge2ortho[i][0];
        e[i * 2 + 0][1] = i;
        e[i * 2 + 0][2] = cube.edge2ortho[i][3];
        e[i * 2 + 1][0] = cube.edge2ortho[i][1];
        e[i * 2 + 1][1] = i;
        e[i * 2 + 1][2] = cube.edge2ortho[i][2];
    }

    for (int i = 0; i < 12 * 2; i++)
    {
        f[i] = axisEdgeFitness(e[i][0], proj) *
            axisEdgeFitness(e[i][1], proj) *
            axisEdgeFitness(e[i][2], proj) * 1e3f;

        // Оси должны выглядеть максимально ортогонально
//        int vi[6] =
//        {
//            cube.edge2v[e[i][0]][0], cube.edge2v[e[i][0]][1],
//            cube.edge2v[e[i][1]][0], cube.edge2v[e[i][1]][1],
//            cube.edge2v[e[i][2]][0], cube.edge2v[e[i][2]][1]
//        };
//        Vec2d v[3] =
//        {
//            (proj[vi[0]] - proj[vi[1]]).toVec2d(),
//            (proj[vi[2]] - proj[vi[3]]).toVec2d(),
//            (proj[vi[4]] - proj[vi[5]]).toVec2d()
//        };
//        double l[3] = {v[0].len(), v[1].len(), v[2].len()};
//        double tmp = 1;
//        if (l[0] > 1e-3 && l[1] > 1e-3) tmp *= 1.0 - fabs(Vec2d::dot(v[0]/l[0], v[1]/l[1]));
//        if (l[1] > 1e-3 && l[2] > 1e-3) tmp *= 1.0 - fabs(Vec2d::dot(v[1]/l[1], v[2]/l[2]));
//        f[i] *= static_cast<float>(tmp);
    }

    int best = 0;
    for (int i = 0; i < 12 * 2; i++)
    {
        if (f[i] > f[best])
            best = i;
    }
    edges[0] = e[best][0];
    edges[1] = e[best][1];
    edges[2] = e[best][2];
}

void QMAxes::placeFgEdges()
{
    int edges[3];
    getNicestAxesEdges(edges);

    QMAxis *a[3] = {&xAxis, &yAxis, &zAxis};
    for (int i = 0; i < 3; i++)
    {
        fge[i].labelsPadding = 0;
        fge[i].cubeEdge = edges[i];
        fge[i].cubeVert[0] = cube.edge2v[edges[i]][0];
        fge[i].cubeVert[1] = cube.edge2v[edges[i]][1];
        fge[i].line[0] = vproj[fge[i].cubeVert[0]].toVec2d();
        fge[i].line[1] = vproj[fge[i].cubeVert[1]].toVec2d();
        fge[i].size = static_cast<float>((fge[i].line[1] - fge[i].line[0]).len());
        fge[i].lineDir = (fge[i].line[1] - fge[i].line[0]) / fge[i].size;
        fge[i].a = a[cube.edge2axis[fge[i].cubeEdge]];
    }

    for (int i = 0; i < 3; i++)
    {
        fge[i].lineNormal = Vec2d(fge[i].lineDir.y(), -fge[i].lineDir.x());
        Vec2d dir = (fge[i].line[0] + fge[i].line[1]) / 2 -  vproj[8].toVec2d();
        if (Vec2d::dot(fge[i].lineNormal, dir) < 0)
            fge[i].lineNormal = -fge[i].lineNormal;
    }
}

float QMAxes::labelFitness(QSizeF sz, Vec2d mid, int w, int h)
{
    struct
    {
        float l;
        float r;
        float b;
        float t;
    } r =
    {
        static_cast<float>(-sz.width() / 2 + mid.x()),
        static_cast<float>(sz.width() / 2 + mid.x()),
        static_cast<float>(-sz.height() / 2 + mid.y()),
        static_cast<float>(sz.height() / 2 + mid.y())
    };
    float lOOB = r.l > 0 ? 0 : -r.l;
    float rOOB = r.r < w ? 0 : r.r - w;
    float bOOB = r.b > 0 ? 0 : r.b;
    float tOOB = r.t < h ? 0 : r.t - h;
    return lOOB*lOOB + rOOB*rOOB + tOOB*tOOB + bOOB*bOOB;
}

void QMAxes::placeAxisLabel(FgEdge &e, const QString &s, int w, int h)
{
    if (s.isEmpty() || e.size < 50)
    {
        e.lh.mid = Vec2d(0, 0);
        e.lh.fitness = 0;
        e.lh.sz = QSizeF();
        e.lh.hor = true;
        e.lh.label.clear();
        return;
    }
    QFontMetrics fm = QFontMetrics(axisLabelFont);
    QSizeF sz = QSizeF(fm.tightBoundingRect(s).size());
    Vec2d vh = placeNearest(e.lineDir, e.lineNormal, sz, 0, 0, nullptr);
    Vec2d vv = placeNearest(e.lineDir, e.lineNormal, sz.transposed(), 0, 0, nullptr);

    Vec2d v0[3];
    v0[0] = (e.line[0] * 3/4 + e.line[1] * 1/4) + e.lineNormal * e.labelsPadding;
    v0[1] = (e.line[0] * 2/4 + e.line[1] * 2/4) + e.lineNormal * e.labelsPadding;
    v0[2] = (e.line[0] * 1/4 + e.line[1] * 3/4) + e.lineNormal * e.labelsPadding;

    LabelHint lh[6] =
    {
        LabelHint(true, sz, v0[0] + vh),
        LabelHint(true, sz, v0[1] + vh),
        LabelHint(true, sz, v0[2] + vh),
        LabelHint(false, sz.transposed(), v0[0] + vv),
        LabelHint(false, sz.transposed(), v0[1] + vv),
        LabelHint(false, sz.transposed(), v0[2] + vv)
    };
    int best = 0;
    for (int i = 0; i < 6; i++)
    {
        const float weights[6] = {1.0f, 0.5f, 1.0f, 1.1f, 0.6f, 1.1f};
        lh[i].fitness = (labelFitness(lh[i].sz, lh[i].mid, w, h) + 1) * weights[i];
        if (lh[i].fitness < lh[best].fitness) best = i;
    }
    e.lh = lh[best];
    e.lh.label = s;
}

void QMAxes::placeAxesLabels(int w, int h)
{
    placeAxisLabel(fge[0], fge[0].a->label, w, h);
    placeAxisLabel(fge[1], fge[1].a->label, w, h);
    placeAxisLabel(fge[2], fge[2].a->label, w, h);
}

void QMAxes::placeGrids(int q, int e1, int e2, int w, int h)
{
    QMAxis *atab[3] = {&xAxis, &yAxis, &zAxis};
    QMAxis &a = *atab[cube.edge2axis[e1]];

    GridHint gh;
    gh.quad = q;
    gh.axis = cube.edge2axis[e1];
    gh.edge[0] = e1;
    gh.edge[1] = e2;
    Vec3d v1(cube.edge2pos[e1]);
    Vec3d v2(cube.edge2pos[e2]);
    double tmp = 1.0 / a.lim.size();
    for (int i = 0; i < a.ticks.count(); i++)
    {
        v1.v[gh.axis] = (a.ticks[i] - a.lim[0]) * tmp - 0.5;
        v2.v[gh.axis] = v1.v[gh.axis];
        gh.line[0] = v1.project(V, P, w, h).toVec2d();
        gh.line[1] = v2.project(V, P, w, h).toVec2d();
        gh.tick = i;
        gridHints.push_back(gh);
    }
}

void QMAxes::placeGrids(int w, int h)
{
    gridHints.clear();
    for (int i = 0; i < 6; i++)
    {
        if (!bgQuads[i]) continue;
        placeGrids(i, cube.quad2edge[i][0], cube.quad2edge[i][1], w, h);
        placeGrids(i, cube.quad2edge[i][2], cube.quad2edge[i][3], w, h);
    }
}

void QMAxes::placeTitle(int w, int h)
{
    titleHint.sz = QSizeF(0, 0);
    titleHint.mid = Vec2d(0, 0);
    if (title.string.isEmpty()) return;
    QFontMetrics fm(title.font);
    QRectF r = oob(w, h);
    titleHint.sz = QSizeF(fm.tightBoundingRect(title.string).size());
    if (title.align & Qt::AlignBottom)
        titleHint.mid.y() = -titleHint.sz.height() / 2 - r.bottom(); else
        titleHint.mid.y() = h + r.top() - titleHint.sz.height() + titleHint.sz.height() / 2;
    float w2 = static_cast<float>(titleHint.sz.width()) / 2;
    if (title.align & Qt::AlignLeft)
        titleHint.mid.x() = -r.left() + w2;
    else
    if (title.align & Qt::AlignRight)
        titleHint.mid.x() = r.right() + w - w2;
    else
        titleHint.mid.x() = (-r.left() + w + r.right()) / 2;
}

void expandOOB(QRectF &r, const Vec2d &mid, const QSizeF &sz, int w, int h)
{
    qreal rl = mid.x() - sz.width() / 2;
    qreal rr = mid.x() + sz.width() / 2;
    qreal rt = mid.y() + sz.height() / 2;
    qreal rb = mid.y() - sz.height() / 2;
    if (rl < 0) r.setLeft(qMax(r.left(), -rl));
    if (rb < 0) r.setBottom(qMax(r.bottom(), -rb));
    if (rr > w) r.setRight(qMax(r.right(), rr - w));
    if (rt > h) r.setTop(qMax(r.top(), rt - h));
}

QRectF QMAxes::oob(int w, int h)
{
    QRectF res;
    for (int i = 0; i < tickHints.count(); i++)
        expandOOB(res, tickHints[i].mid, tickHints[i].sz, w, h);
    for (int i = 0; i < 3; i++)
        expandOOB(res, fge[i].lh.mid, fge[i].lh.sz, w, h);
    expandOOB(res, titleHint.mid, QSizeF(0, titleHint.sz.height()), w, h);
    return res;
}

void QMAxes::placeBgQuads(int w, int h)
{
    for (int i = 0; i < 6; i++)
    {
        Vec3d v1 = Vec3d(cube.v[cube.quad2v[i][0]]).project(V, P, w, h);
        Vec3d v2 = Vec3d(cube.v[cube.quad2v[i][1]]).project(V, P, w, h);
        Vec3d v3 = Vec3d(cube.v[cube.quad2v[i][2]]).project(V, P, w, h);
        Vec3d n = Vec3d::cross(v3 - v2, v2 - v1).normalized();
        bgQuads[i] = Vec3d::dot(n, Vec3d(0, 0, 1)) > 1e-3;
    }
}

void QMAxes::placeBgEdges()
{
    memset(bgEdges, 0, sizeof(bgEdges));
    for (int i = 0; i < 6; i++)
    {
        if (!bgQuads[i]) continue;
        for (int j = 0; j < 4; j++)
        {
            int edge = cube.quad2edge[i][j];
            bgEdges[edge] = edge != fge[0].cubeEdge &&
                edge != fge[1].cubeEdge &&
                edge != fge[2].cubeEdge;
        }
    }
}

void QMAxes::calcViewMatrix()
{
    Vec3d k = Vec3d(camera.target.x(), camera.target.y(), camera.target.z()).normalized();
    Vec3d j = Vec3d(camera.upVector.x(), camera.upVector.y(), camera.upVector.z()).normalized();
    Vec3d i = Vec3d::cross(k, j);
    V = Mat4d(1, 0, 0, -camera.position.x(),
              0, 1, 0, -camera.position.y(),
              0, 0, 1, -camera.position.z(),
              0, 0, 0, 1);
    V = Mat4d(i, j, -k) * V;
}

void QMAxes::calcProjMatrix(int w, int h, int margin_lrud[4])
{
    if (camera.persp)
    {
        double max[3] = {0, 0, 0};
        for (int i = 0; i < 8; i++)
        {
            Vec3d v = V * Vec3d(cube.v[i]);
            max[0] = qMax(max[0], v.x());
            max[1] = qMax(max[1], v.y());
            max[2] = qMax(max[2], v.z());
        }
        float ww = w - margin_lrud[0] - margin_lrud[1];
        float hh = h - margin_lrud[2] - margin_lrud[3];
        P =
            Mat4d::scale(2.0/w, 2.0/h, 1) *
            Mat4d::translate(
                -0.5 * (margin_lrud[0] + margin_lrud[1]) + margin_lrud[0],
                -0.5 * (margin_lrud[2] + margin_lrud[3]) + margin_lrud[3], 0) *
            Mat4d::scale(ww / w, hh / h, 1) *
            Mat4d::scale(w/2, h/2, 1) *
            Mat4d::perspective(camera.viewAngle, /*(float)w / h*/ 1, camera.nearPlane, camera.farPlane);
    }
    else
    {
        double max[3] = {0, 0, 0};
        for (int i = 0; i < 8; i++)
        {
            Vec3d v = V * Vec3d(cube.v[i]);
            max[0] = qMax(max[0], v.x());
            max[1] = qMax(max[1], v.y());
            max[2] = qMax(max[2], v.z());
        }
        float ww = w - margin_lrud[0] - margin_lrud[1];
        float hh = h - margin_lrud[2] - margin_lrud[3];
        P = Mat4d::scale(2.0/w, 2.0/h, 1) *
            Mat4d::translate(
                -0.5 * (margin_lrud[0] + margin_lrud[1]) + margin_lrud[0],
                -0.5 * (margin_lrud[2] + margin_lrud[3]) + margin_lrud[3], 0) *
            Mat4d::scale(ww / w, hh / h, 1) *
            Mat4d::scale(w/2, h/2, 1) *
            Mat4d::ortho(-max[0], max[0], -max[1], max[1], -2, 2);
    }
}

void QMAxes::placeCamera()
{
    if (!camera.autoMode) return;

    // set directions
    Mat3d mat = Mat3d::rotz(camera.azimuth/180*M_PI) * Mat3d::rotx(-camera.elevation/180*M_PI);
    camera.target = mat * Vec3d(0, 1, 0);
    camera.upVector = mat * Vec3d(0, 0, 1);

    // set position
    if (camera.persp)
    {
        /*

          \_______/ - far plane
           \  _  /
            \(_)/  - cube area
             \_/  - near plane
              X  - camera
        */
        double r = 0.87 / tan(camera.viewAngle / 2 / 180 * M_PI);
        camera.position = -camera.target * r;
        camera.nearPlane = r - 0.87; // sqrt(3*0.5^3)
        camera.farPlane = r + 0.87;
    }
    else
    {
        camera.position = -camera.target * 1;
    }
}

void QMAxes::calcCubeProj(int w, int h)
{
    for (int i = 0; i < 8; i++)
    {
        vproj[i] = Vec3d(cube.v[i]).project(V, P, w, h);
        vproj[i][2] = 0;
    }
    vproj[8] = Vec3d(0, 0, 0).project(V, P, w, h);
}

void QMAxes::drawCube(int w, int h)
{
    glMatrixMode(GL_PROJECTION);
    glLoadMatrixd(P.t().m);

    glMatrixMode(GL_MODELVIEW);
    glLoadMatrixd(V.t().m);

    // 1. draw background quads
    glColor3f(1, 1, 1);
    glBegin(GL_QUADS);
    for (int i = 0; i < 6; i++)
        if (bgQuads[i])
        {
            glVertex3dv(cube.v[cube.quad2v[i][0]]);
            glVertex3dv(cube.v[cube.quad2v[i][1]]);
            glVertex3dv(cube.v[cube.quad2v[i][2]]);
            glVertex3dv(cube.v[cube.quad2v[i][3]]);
        }
    glEnd();

    // 2. draw grids

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glMatrixMode(GL_PROJECTION);
    glLoadMatrixd(Mat4d::ortho(0, w, 0, h, -1, 1).t().m);

    glColor3f(0.8f, 0.8f, 0.8f);
    glBegin(GL_LINES);
    for (int i = 0; i < gridHints.count(); i++)
    {
        glVertex2dv(gridHints[i].line[0].v);
        glVertex2dv(gridHints[i].line[1].v);
    }
    glEnd();

    // 5. draw ticks

    glColor3f(0, 0, 0);
    glBegin(GL_LINES);
    for (int i = 0; i < tickHints.count(); i++)
    {
        glVertex2dv(&tickHints[i].line[0][0]);
        glVertex2dv(&tickHints[i].line[1][0]);
    }
    glEnd();

    // 3. draw axes

    glMatrixMode(GL_PROJECTION);
    glLoadMatrixd(P.t().m);

    glMatrixMode(GL_MODELVIEW);
    glLoadMatrixd(V.t().m);

    glColor3f(0, 0, 0);
    glLineWidth(1.2f);
    glBegin(GL_LINES);
    for (int i = 0; i < 3; i++)
    {
        glVertex3dv(cube.v[fge[i].cubeVert[0]]);
        glVertex3dv(cube.v[fge[i].cubeVert[1]]);
    }
    glEnd();

    // draw background edges

    glColor3f(0.5, 0.5, 0.5);
    glLineWidth(1.2f);
    glBegin(GL_LINES);
    for (int i = 0; i < 12; i++)
        if (bgEdges[i])
        {
            glVertex3dv(cube.v[cube.edge2v[i][0]]);
            glVertex3dv(cube.v[cube.edge2v[i][1]]);
        }
    glEnd();
}

void glSetLineSpec(const QMLineSpec &ls)
{
    glColor4f(ls.color.redF(), ls.color.greenF(), ls.color.blueF(), ls.color.alphaF());
    glLineWidth(ls.width);
    switch (ls.style)
    {
    case QMLineSpec::Dashed:
        glEnable(GL_LINE_STIPPLE);
        glLineStipple(10, 0xAAAA);
        break;
    case QMLineSpec::Dotted:
        glEnable(GL_LINE_STIPPLE);
        glLineStipple(2, 0xAAAA);
        break;
    case QMLineSpec::DashDot:
        glEnable(GL_LINE_STIPPLE);
        glLineStipple(5, 0xBEBE);
        break;
    default:
        glDisable(GL_LINE_STIPPLE);
        break;
    }
}

void QMAxes::drawVPFrames(int w, int h)
{
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, w, 0, h, -1, 1);

    if (zoomRect.visible)
    {
        glSetLineSpec(zoomRect.linespec);
        glBegin(GL_LINE_LOOP);
        glVertex2f(zoomRect.point[0].x(), h - zoomRect.point[0].y());
        glVertex2f(zoomRect.point[0].x(), h - zoomRect.point[1].y());
        glVertex2f(zoomRect.point[1].x(), h - zoomRect.point[1].y());
        glVertex2f(zoomRect.point[1].x(), h - zoomRect.point[0].y());
        glEnd();
        glDisable(GL_LINE_STIPPLE);
    }

    if (focusRect.visible)
    {
        glSetLineSpec(focusRect.linespec);
        glBegin(GL_LINE_LOOP);
        glVertex2f(focusRect.point[0].x(), h - focusRect.point[0].y());
        glVertex2f(focusRect.point[0].x(), h - focusRect.point[1].y());
        glVertex2f(focusRect.point[1].x(), h - focusRect.point[1].y());
        glVertex2f(focusRect.point[1].x(), h - focusRect.point[0].y());
        glEnd();
        glDisable(GL_LINE_STIPPLE);
    }

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();

    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}

void QMAxes::prepareToDraw(int w, int h)
{
    placeCamera();
    calcViewMatrix();

    int box_margin_lrud[4] = {10, 10, 10, 10};
    for (int i = 0; i < 2; i++)
    {
        calcProjMatrix(w, h, box_margin_lrud);
        calcCubeProj(w, h);
        placeBgQuads(w, h);
        placeFgEdges();
        placeBgEdges();
        updateAxes();
        placeGrids(w, h);

        calcModelMatrix();
        placeTicks();
        placeTickLabels();
        placeAxesLabels(w, h);
        placeTitle(w, h);
        QRectF r = oob(w, h);
        box_margin_lrud[0] += r.left() + 10;
        box_margin_lrud[1] += r.right() + 10;
        box_margin_lrud[2] += r.top() + 10;
        box_margin_lrud[3] += r.bottom() + 10;
    }
}

void QMAxes::draw(int w, int h, QPaintDevice *dev)
{
    //    glEnable(GL_DEPTH_TEST);
    //    glEnable(GL_CULL_FACE);
    //    glCullFace(GL_BACK);
    //    glDisable(GL_CULL_FACE);

    //    glEnable(GL_LINE_SMOOTH);
    //    glEnable(GL_BLEND);
    //    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    //    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);

    prepareToDraw(w, h);

    drawCube(w, h);


    glMatrixMode(GL_PROJECTION);
    glLoadMatrixd(P.t().m);
    glMatrixMode(GL_MODELVIEW);
    glLoadMatrixd(V.t().m);
    drawVPFrames(w, h);

    glMatrixMode(GL_MODELVIEW);
    glLoadMatrixd((V * M).t().m);

    double clip[6][4] =
    {
        {1, 0, 0, -xAxis.lim.expandedMin() },
        {-1, 0, 0, xAxis.lim.expandedMax() },
        {0, 1, 0, -yAxis.lim.expandedMin() },
        {0, -1, 0, yAxis.lim.expandedMax() },
        {0, 0, 1, -zAxis.lim.expandedMin() },
        {0, 0, -1, zAxis.lim.expandedMax() }
    };
    unsigned i2clip[6] =
    {
        GL_CLIP_PLANE0, GL_CLIP_PLANE1,
        GL_CLIP_PLANE2, GL_CLIP_PLANE3,
        GL_CLIP_PLANE4, GL_CLIP_PLANE5
    };
    for (int i = 0; i < 6; i++)
    {
        glEnable(i2clip[i]);
        glClipPlane(i2clip[i], clip[i]);
    }

    for (int i = 0; i < graphics.count(); i++)
        if (graphics[i]->visible)
            graphics[i]->draw(*this);

    for (int i = 0; i < 6; i++)
        glDisable(i2clip[i]);

    glFlush();
    glFinish();

    QOpenGLContext::currentContext()->doneCurrent();

    outCamParams(dev, w, h);
    outTextLabels(dev, w, h);
}

void QMAxes::outCamParams(QPaintDevice *dev, int w, int h)
{
    (void)w;
    return;
    // Render text
    QPainter painter(dev);
    painter.setPen(QColor(Qt::black));
    painter.setFont(QFont("Times", 10, QFont::Bold));
    QString s;
    s.sprintf("Az: %.0f El: %.0f", camera.azimuth, camera.elevation);
    painter.drawText(20, h - 10, s);
    painter.end();
}

QString int2sup(int val)
{
    const QString tab = "⁰¹²³⁴⁵⁶⁷⁸⁹⁻";
    QString res = QString::number(val);
    for (int i = 0; i < res.length(); i++)
    {
        int u = res[i].unicode();
        if (u >= '0' && u <= '9')
            res[i] = tab[u - '0'];
        else
        if (u == '-')
            res[i] = tab[10];
        else
            return QString();
    }
    return res;
}

void QMAxes::outTextLabels(QPaintDevice *dev, int w, int h)
{
    (void)w;

    // Render text
    QPainter painter(dev);
    painter.setPen(QColor(Qt::black));
    if (!title.string.isEmpty())
    {
        painter.setFont(title.font);
//        QTextOption o;
//        o.setAlignment(title.align);
//        painter.drawText(QRectF(0, 0, w, h), title.string, o);
        painter.translate(titleHint.mid.x(), h - titleHint.mid.y());
        painter.translate(-titleHint.sz.width() / 2, titleHint.sz.height() / 2);
        painter.drawText(0, 0, title.string);
        painter.resetTransform();
    }

    painter.setFont(axisTickFont);
    for (int i = 0; i < tickHints.count(); i++)
    {
        TickHint &th = tickHints[i];
        if (th.label == nullptr) continue;
        painter.translate(th.mid.x(), h - th.mid.y());
        painter.translate(-th.sz.width() / 2, th.sz.height() / 2);
        painter.drawText(0, 0, *th.label);
        painter.resetTransform();
    }

    QTextDocument td;
    td.setDefaultFont(axisTickFont);
    td.setDocumentMargin(2);
    for (int i = 0; i < 3; i++)
    {
        QMAxis *a = fge[i].a;
        if (a->exp == 0) continue;
        if (fge[i].size < 10) continue;
        td.setHtml(QString("&times;10<sup>%1</sup>").arg(a->exp));
        QSizeF size = td.size();
        Vec2d p = (fge[i].line[0] + fge[i].line[1]) * 0.5;
        p = p - fge[i].lineNormal * (hypot(size.width(), size.height()) / 2 + 1);
        painter.translate(p.x(), h - p.y());
        painter.translate(-size.width() / 2, -size.height() / 2);
        painter.setBrush(QBrush(QColor(Qt::white), Qt::SolidPattern));
        painter.setPen(QColor(Qt::black));
        painter.setPen(QColor(Qt::SolidLine));
        painter.drawRect(QRectF(0, 0, size.width(), size.height()));
        td.drawContents(&painter);
        painter.resetTransform();
    }

    // axes labels
    painter.setFont(axisLabelFont);
    for (int i = 0; i < 3; i++)
    {
        LabelHint &lh = fge[i].lh;
        if (lh.label.isEmpty()) continue;
        painter.translate(lh.mid.x(), h - lh.mid.y());
        painter.translate(-lh.sz.width() / 2, lh.sz.height() / 2);
        if (!lh.hor) painter.rotate(-90);
        painter.drawText(0, 0, lh.label);
        painter.resetTransform();
    }

    for (int i = 0; i < text.count(); i++)
    {
        if (text[i].position.x() < xAxis.lim[0]) continue;
        if (text[i].position.x() > xAxis.lim[1]) continue;
        if (text[i].position.y() < yAxis.lim[0]) continue;
        if (text[i].position.y() > yAxis.lim[1]) continue;
        if (text[i].position.z() < zAxis.lim[0]) continue;
        if (text[i].position.z() > zAxis.lim[1]) continue;
        Vec3d v = text[i].position.project(V * M, P, w, h);
        if (v.x() < 0 || v.y() < 0) continue;
        if (v.x() >= w || v.y() >= h) continue;
        QFontMetrics fm = QFontMetrics(text[i].font);
        QSizeF sz = fm.size(0, text[i].string);
        int tx = lround(v.x() - sz.width() / 2);
        int ty = lround(v.y()) + 5;
        if (text[i].align & Qt::AlignLeft)
            tx = lround(v.x()) + 5;
        if (text[i].align & Qt::AlignRight)
            tx = lround(v.x() - sz.width()) - 5;
        if (text[i].align & Qt::AlignTop)
            ty = lround(v.y() - sz.height()) - 5;
        if (text[i].align & Qt::AlignVCenter)
            ty = lround(v.y() - sz.height() / 2);
        painter.setFont(text[i].font);
        painter.drawText(tx, h - ty, text[i].string);
    }

    painter.end();
}

static QList<QMWidget *> qmwidgets;

QMWidget::QMWidget(QWidget *parent) :
    QOpenGLWidget(parent), QMAxes(), legend(*this)
{
    dc = new QMDefController(this);
    ctl = dc;
    bgColor = QWidget::palette().color(QWidget::backgroundRole());
    setMouseTracking(true);
    qmwidgets.push_front(this);
    legend.setBindingCorner(Qt::TopRightCorner);
    legend.setBindingPosition(QPoint(15, 15));

    QSurfaceFormat format;
    format.setDepthBufferSize(24);
    format.setStencilBufferSize(8);
    format.setSamples(32);
    format.setProfile(QSurfaceFormat::CoreProfile);
    setFormat(format);
}

QMWidget::~QMWidget()
{
    qmwidgets.removeOne(this);
}

void QMWidget::fillLegend(const QStringList &text)
{
    legend.hide();
    legend.clear();
    for (int i = 0; i < graphics.count() && i < text.count(); i++)
    {
        if (text[i].isEmpty()) continue;
        legend.addItem(graphics[i]->linespec, text[i]);
    }
    legend.show();
}

void QMWidget::ctlZoom(const QRect &rect)
{
    const int N = 40;
    Vec3d is[N * 6]; // intersection
    QRectF r(rect);
    int ni = 0;
    for (int i = 0; i < N / 4; i++)
    {
        ni += cubeIntersections(is + ni, QPoint(lround(r.x() + r.width() / 10 * i), lround(r.y())));
        ni += cubeIntersections(is + ni, QPoint(lround(r.x() + r.width() / 10 * i), lround(r.y() + r.height())));
        ni += cubeIntersections(is + ni, QPoint(lround(r.x()), lround(r.y() + r.height() / 10 * i)));
        ni += cubeIntersections(is + ni, QPoint(lround(r.x() + r.width()), lround(r.y() + r.height() / 10 * i)));
    }
    if (ni == 0) return;
    xAxis.manualLim[0] = is[0].x();
    xAxis.manualLim[1] = is[0].x();
    yAxis.manualLim[0] = is[0].y();
    yAxis.manualLim[1] = is[0].y();
    zAxis.manualLim[0] = is[0].z();
    zAxis.manualLim[1] = is[0].z();
    for (int i = 1; i < ni; i++)
    {
        xAxis.manualLim[0] = qMin(xAxis.manualLim[0], is[i].x());
        xAxis.manualLim[1] = qMax(xAxis.manualLim[1], is[i].x());
        yAxis.manualLim[0] = qMin(yAxis.manualLim[0], is[i].y());
        yAxis.manualLim[1] = qMax(yAxis.manualLim[1], is[i].y());
        zAxis.manualLim[0] = qMin(zAxis.manualLim[0], is[i].z());
        zAxis.manualLim[1] = qMax(zAxis.manualLim[1], is[i].z());
    }
}

void QMWidget::ctlZoom(const QPoint &at, float ticks)
{
    Vec3d v0(at.x() + 0, height() - at.y() + 0, 0);
    Vec3d v1(at.x() + 1, height() - at.y() + 0, 0);
    Vec3d v2(at.x() + 0, height() - at.y() + 1, 0);
    v0 = v0.unproject(V * M, P, width(), height());
    v1 = v1.unproject(V * M, P, width(), height());
    v2 = v2.unproject(V * M, P, width(), height());
    v1 = (v1 - v0).normalized();
    v2 = (v2 - v0).normalized();
    Vec3d c1(xAxis.lim[0], yAxis.lim[0], zAxis.lim[0]); // limits corner 1
    Vec3d c2(xAxis.lim[1], yAxis.lim[1], zAxis.lim[1]); // limits corner 2
    float factor = 1.0f - pow(1.1f, ticks);
    Vec3d dx1 = Vec3d::dot(c1 - v0, v1) * factor * v1;
    Vec3d dx2 = Vec3d::dot(c2 - v0, v1) * factor * v1;
    Vec3d dy1 = Vec3d::dot(c1 - v0, v2) * factor * v2;
    Vec3d dy2 = Vec3d::dot(c2 - v0, v2) * factor * v2;
    c1 += dx1 + dy1;
    c2 += dx2 + dy2;
    xAxis.manualLim[0] = c1.x();
    xAxis.manualLim[1] = c2.x();
    yAxis.manualLim[0] = c1.y();
    yAxis.manualLim[1] = c2.y();
    zAxis.manualLim[0] = c1.z();
    zAxis.manualLim[1] = c2.z();
}

void QMWidget::ctlMove(int dx, int dy)
{
    dx = -dx;
    Vec3d v0(width() / 2, height() / 2, 0);
    Vec3d v1(width() / 2 + dx, height() / 2, 0);
    Vec3d v2(width() / 2, height() / 2 + dy, 0);
    v0 = v0.unproject(V * M, P, width(), height());
    v1 = v1.unproject(V * M, P, width(), height());
    v2 = v2.unproject(V * M, P, width(), height());

    Vec3d delta = (v1 - v0) + (v2 - v0);
    xAxis.manualLim[0] = xAxis.lim[0] + delta.x();
    xAxis.manualLim[1] = xAxis.lim[1] + delta.x();
    yAxis.manualLim[0] = yAxis.lim[0] + delta.y();
    yAxis.manualLim[1] = yAxis.lim[1] + delta.y();
    zAxis.manualLim[0] = zAxis.lim[0] + delta.z();
    zAxis.manualLim[1] = zAxis.lim[1] + delta.z();
}

void QMWidget::ctlFocus(const QRect &rect, bool concatenation)
{
    bool emitSignal = false;
    Mat4d mv = V * M;
    QMRange x(rect.left(), rect.right());
    QMRange y(height() - rect.bottom(), height() - rect.top());
    for (int i = 0; i < graphics.count(); i++)
        if (graphics[i]->focusable)
        {
            graphics[i]->setFocus(x, y, mv, P, width(), height(), concatenation);
            emitSignal = true;
        }
    update();
    if (emitSignal)
        emit dataFocused();
}

void QMWidget::unproject(const QPoint &pt, Vec3d *outPoint, Vec3d *outDir)
{
    Vec3d v0(pt.x(), height() - pt.y(), 0);
    Vec3d v1(pt.x(), height() - pt.y(), 1);
    v0 = v0.unproject(V * M, P, width(), height());
    v1 = v1.unproject(V * M, P, width(), height());
    if (outPoint != nullptr) *outPoint = v0;
    if (outDir != nullptr) *outDir = (v1 - v0).normalized();
}

int QMWidget::cubeIntersections(Vec3d *res, const Vec3d &d, const Vec3d &p)
{
    int count = 0;
    if (planeAndLineIntersection(d, p, Vec3d(1, 0, 0), xAxis.lim[0], &res[count]))
        if (inrange2(res[count].y(), res[count].z(), yAxis.lim, zAxis.lim))
            count++;
    if (planeAndLineIntersection(d, p, Vec3d(1, 0, 0), xAxis.lim[1], &res[count]))
        if (inrange2(res[count].y(), res[count].z(), yAxis.lim, zAxis.lim))
            count++;
    if (planeAndLineIntersection(d, p, Vec3d(0, 1, 0), yAxis.lim[0], &res[count]))
        if (inrange2(res[count].x(), res[count].z(), xAxis.lim, zAxis.lim))
            count++;
    if (planeAndLineIntersection(d, p, Vec3d(0, 1, 0), yAxis.lim[1], &res[count]))
        if (inrange2(res[count].x(), res[count].z(), xAxis.lim, zAxis.lim))
            count++;
    if (planeAndLineIntersection(d, p, Vec3d(0, 0, 1), zAxis.lim[0], &res[count]))
        if (inrange2(res[count].x(), res[count].y(), xAxis.lim, yAxis.lim))
            count++;
    if (planeAndLineIntersection(d, p, Vec3d(0, 0, 1), zAxis.lim[1], &res[count]))
        if (inrange2(res[count].x(), res[count].y(), xAxis.lim, yAxis.lim))
            count++;
    return count;
}

int QMWidget::cubeIntersections(Vec3d *res, const QPoint &p)
{
    Vec3d v0(p.x(), height() - p.y(), 0);
    Vec3d v1(p.x(), height() - p.y(), 1);
    v0 = v0.unproject(V * M, P, width(), height());
    v1 = v1.unproject(V * M, P, width(), height());
    Vec3d dir = (v0 - v1).normalized();
    return cubeIntersections(res, dir, v0);
}

void QMWidget::initializeGL()
{
    // Set up the rendering context, load shaders and other resources, etc.:
    QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
    f->glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
}

void QMWidget::resizeGL(int w, int h)
{
    (void)w;
    (void)h;
//  glViewport(w, h, w, h);
}

void QMWidget::paintGL()
{
    // Draw the scene:
//        QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
//        f->glClear(GL_COLOR_BUFFER_BIT);

    int w = width();
    int h = height();

    if (0)
    {
        int ww = w;
        int hh = h;
        if (ww > hh)
            ww = hh; else
            hh = ww;
        glViewport((w - ww) / 2, (h - hh) / 2, ww, hh);
        w = ww;
        h = hh;
    }
    else
    {
        glViewport(0, 0, w, h);
    }


    glClearColor(bgColor.redF(), bgColor.greenF(), bgColor.blueF(), bgColor.alphaF());
    glClear(GL_COLOR_BUFFER_BIT);

    draw(w, h, this);


//    if (pressed)
//        outCamParams(this, width(), height());


/*
    int height = this->height();
    GLdouble textPosX = 0, textPosY = 0, textPosZ = 0;
    //project(x, y, 0f, &textPosX, &textPosY, &textPosZ);
    textPosY = height - textPosY; // y is inverted

    // Render text
    QPainter painter(this);
    painter.setPen(QColor(Qt::black));
    painter.setFont(QFont("Times", 10, QFont::Bold));
    painter.drawText(0, 100, "My text");
    painter.end();
*/
}

void QMWidget::mousePressEvent(QMouseEvent *event)
{
    ctl->mousePressEvent(event);
}

void QMWidget::mouseReleaseEvent(QMouseEvent *event)
{
    ctl->mouseReleaseEvent(event);
}

float angle_norm_360(float angle)
{
    angle -= trunc(angle / 360) * 360;
    return angle < 0 ? angle + 360 : angle;
}

double angle_normd_180(double angle)
{
    angle -= trunc(angle / 360) * 360;
    if (angle <= -180) return angle + 360;
    if (angle > 180) return angle - 360;
    return angle;
}

static inline float nearestAngle(float angle, float step)
{
    return round(angle / step) * step;
}

void QMWidget::mouseMoveEvent(QMouseEvent *event)
{
    ctl->mouseMoveEvent(event);
}

void QMWidget::wheelEvent(QWheelEvent *event)
{
    ctl->wheelEvent(event);
}

QMLines::QMLines(QMAxes &parent) :
    QMGraphic(parent), cache(nullptr), cacheSize(0), focusCache(nullptr), focusCacheSize(0)
{
}

QMLines::~QMLines()
{
    free(cache);
}

void QMLines::dataChanged()
{
    QMGraphic::dataChanged();
    int cx = x.count();
    int cy = y.count();
    int cz = z.count();
    int cf = focused.count();
    cacheSize = static_cast<size_t>(qMax(qMax(cx, cy), cz));
    delete [] cache;
    cache = new double[cacheSize * 3]();
    if (cache == nullptr)
    {
        x.clear();
        y.clear();
        z.clear();
        QMGraphic::dataChanged();
        return;
    }
    for (int i = 0; i < cx; i++)
        cache[i * 3 + 0] = x[i];
    for (int i = 0; i < cy; i++)
        cache[i * 3 + 1] = y[i];
    for (int i = 0; i < cz; i++)
        cache[i * 3 + 2] = z[i];

    focusCacheSize = 0;
    for (int i = 0; i < cf; i++)
        if (focused[i])
            focusCacheSize++;
    delete [] focusCache;
    focusCache = nullptr;
    if (focusCacheSize == 0) return;
    focusCache = new double[cacheSize * 3];
    int n = 0;
    for (int i = 0; i < qMin(cx, cf); i++)
        if (focused[i])
            focusCache[n++ * 3 + 0] = x[i];
    n = 0;
    for (int i = 0; i < qMin(cy, cf); i++)
        if (focused[i])
            focusCache[n++ * 3 + 1] = y[i];

    n = 0;
    for (int i = 0; i < qMin(cz, cf); i++)
        if (focused[i])
            focusCache[n++ * 3 + 2] = z[i];
}

void QMLines::draw(QMAxes &axes)
{
    (void)axes;

    int id = axes.graphics.indexOf(this);
    if (id == -1) id = 0;
    if (linespec.style != QMLineSpec::None)
    {
        glSetLineSpec(linespec);
        glVertexPointer(3, GL_DOUBLE, 0, cache);
        glEnableClientState(GL_VERTEX_ARRAY);
        glDrawArrays(GL_LINE_STRIP, 0, cacheSize);
        glDisableClientState(GL_VERTEX_ARRAY);
        glDisable(GL_LINE_STIPPLE);
    }
    if (linespec.marker != 0)
    {
        glColor4f(linespec.color.redF(),
                  linespec.color.greenF(),
                  linespec.color.blueF(),
                  linespec.color.alphaF());
        glPointSize(3);
        glVertexPointer(3, GL_DOUBLE, 0, cache);
        glEnableClientState(GL_VERTEX_ARRAY);
        glDrawArrays(GL_POINTS, 0, cacheSize);
        glDisableClientState(GL_VERTEX_ARRAY);
        glDisable(GL_LINE_STIPPLE);
    }

    if (focusCacheSize == 0) return;
    glColor3f(1.0f, 0, 0);
    glPointSize(3);
    glVertexPointer(3, GL_DOUBLE, 0, focusCache);
    glEnableClientState(GL_VERTEX_ARRAY);
    glDrawArrays(GL_POINTS, 0, cacheSize);
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisable(GL_LINE_STIPPLE);
}

void QMLines::setFocus(const QMRange &x, const QMRange &y, const Mat4d &modelView, const Mat4d &projection,
                       int w, int h, bool concatenation)
{
    int n = qMax(qMax(this->x.count(), this->y.count()), this->y.count());
    focused.resize(n);
    for (int i = 0; i < n; i++)
    {
        Vec3d v(i < this->x.count() ? this->x[i] : 0,
                i < this->y.count() ? this->y[i] : 0,
                i < this->z.count() ? this->z[i] : 0);
        v = v.project(modelView, projection, w, h);
        bool f = x.in(v.x()) && y.in(v.y());
        focused[i] = concatenation ? focused[i] | f : f;
    }
    dataChanged();
}

QMBar::QMBar(QMAxes &parent) : QMGraphic(parent), minDelta(1)
{
}

void QMBar::dataChanged()
{
    dvec tmp = x;
    std::sort(tmp.begin(), tmp.end());
    minDelta = 1;
    if (tmp.count() > 1)
        minDelta = tmp[1] - tmp[0];
    for (int i = 2; i < tmp.count(); i++)
        if (tmp[i] - tmp[i - 1] < minDelta)
            minDelta = tmp[i] - tmp[i - 1];
    qmwrap::minmax_finit(this->x, xb[0], xb[1]);
    qmwrap::minmax_finit(this->y, yb[0], yb[1]);
    qmwrap::minmax_finit(this->z, zb[0], zb[1]);
    xb[0] -= 0.5 * minDelta;
    xb[1] += 0.5 * minDelta;
    zb[0] -= 0.5;
    zb[1] += 0.5;
}

void QMBar::draw(QMAxes &axes)
{
    (void)axes;
    QColor c = linespec.color;
    if (!linespec.color.isValid())
    {
        int id = axes.graphics.indexOf(this);
        if (id == -1) id = 0;
        c = QColor::fromRgbF(colormapLines[id][0],
                             colormapLines[id][1],
                             colormapLines[id][2]);
    }
    glMatrixMode(GL_MODELVIEW);
    glEnable(GL_DEPTH_TEST);
    int count = qMin(x.count(), y.count());
    for (int i = 0; i < count; i++)
    {
        double z = i < this->z.count() ? this->z[i] : 0.0;
        glPushMatrix();
        glTranslated(x[i], 0, z);
        glScaled(minDelta, y[i], 1);
        glScaled(0.6, 1, 0.6);
        glTranslated(0.0, 0.5, 0.0);

        QColor rgba = i < barColors.count() ? barColors[i] : c;
        glColor4f(static_cast<float>(rgba.redF()),
                  static_cast<float>(rgba.greenF()),
                  static_cast<float>(rgba.blueF()),
                  static_cast<float>(rgba.alphaF()));

        glBegin(GL_QUADS);
        for (int q = 0; q < 6; q++)
        {
            glVertex3dv(cube.v[cube.quad2v[q][0]]);
            glVertex3dv(cube.v[cube.quad2v[q][1]]);
            glVertex3dv(cube.v[cube.quad2v[q][2]]);
            glVertex3dv(cube.v[cube.quad2v[q][3]]);
        }
        glEnd();
        glPopMatrix();
    }
    for (int i = 0; i < count; i++)
    {
        double z = i < this->z.count() ? this->z[i] : 0.0;
        glPushMatrix();
        glTranslated(x[i], 0, z);
        glScaled(minDelta, y[i], 1);
        glScaled(0.6, 1.0, 0.6);
        glTranslated(0, 0.5, 0);

        glColor3f(0, 0, 0);
        glLineWidth(2);
        glBegin(GL_LINES);
        for (int e = 0; e < 12; e++)
        {
            glVertex3dv(cube.v[cube.edge2v[e][0]]);
            glVertex3dv(cube.v[cube.edge2v[e][1]]);
        }
        glEnd();

        glPopMatrix();
    }
    glDisable(GL_DEPTH_TEST);
}

QMAxis::QMAxis() :
    logScale(false),
    autoLim(true),
    autoTickLabels(true),
    grid(true),
    gridMinor(false),
    tickLabelRotation(0),
    exp(0)
{
    manualLim[0] = -1;
    manualLim[1] = +1;
    preferredLim[0] = -1;
    preferredLim[1] = +1;
    lim[0] = -1;
    lim[1] = +1;
}

void QMAxis::adaptate(int nticks, bool roundLimits)
{
    double min = autoLim ? preferredLim[0] : manualLim[0];
    double max = autoLim ? preferredLim[1] : manualLim[1];
    if (nticks < 2)
    {
        ticks.clear();
        tickLabels.clear();
        lim[0] = min;
        lim[1] = max;
        return;
    }
    double amax = qMax(fabs(min), fabs(max));
    int e = static_cast<int>(floor(log10(amax)));
    // normalizing
    double nmin = min * pow(10, -e);
    double nmax = max * pow(10, -e);
    // finding optimal step
    const double steps[] = {0.01, 0.05, 0.1, 0.15, 0.2, 0.25, 0.5, 1, 1.5, 2, 2.5, 5};
    double step = 0.0;
    int a = 0;
    int b = 0;
    for (size_t i = 0; i < sizeof(steps) / sizeof(steps[0]); i++)
    {
        if (roundLimits)
        {
            a = static_cast<int>(floor(nmin / steps[i]));
            b = static_cast<int>(ceil(nmax / steps[i]));
        }
        else
        {
            a = static_cast<int>(ceil(nmin / steps[i]));
            b = static_cast<int>(floor(nmax / steps[i]));
        }
        if (b - a + 1 > nticks) continue;
        step = steps[i];
        break;
    }
    ticks.clear();
    if (step == 0.0)
    {
        lim[0] = min;
        lim[1] = max;
        ticks.push_back(nmin);
        ticks.push_back(nmax);
    }
    else
    {
        for (int i = a; i <= b; i++)
            ticks.push_back(step * i);
        if (roundLimits)
        {
            lim[0] = step * a * pow(10, e);
            lim[1] = step * b * pow(10, e);
        }
        else
        {
            lim[0] = min;
            lim[1] = max;
        }
    }

    // set tick labels
    tickLabels.clear();
    if (e >= -2 && e <= 3)
    {
        // 0.022 0.110 9.99 99.9 999 9999
        const char dig[] = {3, 3, 2, 1, 0, 0};
        for (int i = 0; i < ticks.count(); i++)
        {
            QString s;
            ticks[i] *= pow(10, e);
            s.sprintf("%.*f", dig[e + 2], ticks[i]);
            tickLabels.push_back(s);
        }
        e = 0;
    }
    else
    {
        for (int i = 0; i < ticks.count(); i++)
        {
            QString s;
            s.sprintf("%.3f", ticks[i]);
            tickLabels.push_back(s);
            ticks[i] *= pow(10, e);
        }
    }
    this->exp = e;
}

/*
QVector<double> QMAxis::autoTicks(int nticks, double min, double max, bool roundMinMax)
{
    QVector<double> res;
    if (nticks < 2)
    {
        return res;
    }
    double amax = qMax(fabs(min), fabs(max));
    int e = floor(log10(amax));
    // normalizing
    double nmin = min * pow(10, -e);
    double nmax = max * pow(10, -e);
    // finding optimal step
    const double steps[] = {0.01, 0.05, 0.1, 0.15, 0.2, 0.25, 0.5, 1, 1.5, 2, 2.5, 5};
    double step = 0;
    for (int i = 0; i < (int)(sizeof(steps) / sizeof(steps[0])); i++)
    {
        int a = floor(nmin / steps[i]);
        int b = ceil(nmax / steps[i]);
        if (b - a + 1 > nticks) continue;
        step = steps[i];
        break;
    }
    ticks.clear();
    if (step == 0)
    {
        ticks.push_back(floor(nmin));
        ticks.push_back(ceil(nmax));
        lim[0] = floor(nmin) * pow(10, e);
        lim[1] = ceil(nmax) * pow(10, e);
//        lim[0] = min;
//        lim[1] = max;
    }
    else
    {
        int a = floor(nmin / step);
        int b = ceil(nmax / step);
        for (int i = a; i <= b; i++)
            ticks.push_back(step * i);
        lim[0] = step * a * pow(10, e);
        lim[1] = step * b * pow(10, e);
//        lim[0] = min;
//        lim[1] = max;
    }
    // set tick labels
    tickLabels.clear();
    if (e >= -2 && e <= 3)
    {
        // 0.022 0.110 9.99 99.9 999 9999
        const char *fmt[] = {"%.3f", "%.3f", "%.2f", "%.1f", "%.0f", "%.0f"};
        for (int i = 0; i < ticks.count(); i++)
        {
            QString s;
            ticks[i] *= pow(10, e);
            s.sprintf(fmt[e + 2], ticks[i]);
            tickLabels.push_back(s);
        }
        e = 0;
    }
    else
    {
        for (int i = 0; i < ticks.count(); i++)
        {
            QString s;
            s.sprintf("%.3f", ticks[i]);
            tickLabels.push_back(s);
        }
    }
    this->exp = e;
}
*/

QMText::QMText()
{
    align = Qt::AlignHCenter | Qt::AlignBottom;
    color = QColor(Qt::black);
    visible = true;
    rotation = 0;
}

QMText::QMText(const Vec3d &pos, const QString &str, Qt::Alignment al, const QColor &c)
{
    align = al;
    color = c;
    string = str;
    position = pos;
    visible = true;
    rotation = 0;
}

QMText::QMText(float x, float y, float z, const QString &str, Qt::Alignment al, const QColor &c)
{
    align = al;
    color = c;
    string = str;
    position = Vec3d(x, y, z);
    visible = true;
    rotation = 0;
}

QMController::QMController(QMWidget *parent) :
    QObject(static_cast<QObject *>(parent))
{
}

QMController::~QMController()
{
}

QMDefController::QMDefController(QMWidget *parent) : QMController(parent)
{
    state = None;
}

void QMDefController::mousePressEvent(QMouseEvent *event)
{
    if (parent() == nullptr) return;
    QMWidget *a = static_cast<QMWidget *>(parent());

    if (event->button() == Qt::RightButton)
    {
        state = Rot;
        a->zoomRect.visible = false;
        a->focusRect.visible = false;
        pressedPos = QCursor::pos();
    }
    if (event->button() == Qt::LeftButton)
    {
        if (event->modifiers() & Qt::ControlModifier)
        {
            state = Focus;
            a->focusRect.point[0] = event->pos();
            a->focusRect.point[1] = event->pos();
            a->focusRect.visible = true;
            pressedPos = event->pos();
        }
        else
        {
            state = Zoom;
            a->zoomRect.point[0] = event->pos();
            a->zoomRect.point[1] = event->pos();
            a->zoomRect.visible = true;
            pressedPos = event->pos();
        }
    }
    if (event->button() == Qt::MiddleButton)
    {
        state = MoveStarting;
        a->zoomRect.visible = false;
        a->focusRect.visible = false;
        pressedPos = QCursor::pos();
    }
    a->update();
}

void QMDefController::mouseReleaseEvent(QMouseEvent *event)
{
    (void)event;
    if (parent() == nullptr) return;
    QMWidget *a = static_cast<QMWidget *>(parent());

    if (state == Zoom)
    {
        state = None;
        a->zoomRect.visible = false;
        a->xAxis.autoLim = false;
        a->yAxis.autoLim = false;
        a->zAxis.autoLim = false;

        QRect r = QRect(qMin(pressedPos.x(), event->pos().x()),
                        qMin(pressedPos.y(), event->pos().y()),
                        abs(pressedPos.x() - event->pos().x()),
                        abs(pressedPos.y() - event->pos().y()));
        if (r.width() > 2 && r.height() > 2)
            a->ctlZoom(r);

        a->update();
    }

    if (state == Focus)
    {
        state = None;
        a->focusRect.visible = false;
        QRect r = QRect(qMin(pressedPos.x(), event->pos().x()),
                        qMin(pressedPos.y(), event->pos().y()),
                        abs(pressedPos.x() - event->pos().x()),
                        abs(pressedPos.y() - event->pos().y()));
        a->ctlFocus(r, event->modifiers() & Qt::ShiftModifier);
        a->update();
    }

    if (state == Rot)
    {
        state = None;
        a->update();
    }

    if (state == Move)
        state = None;

    if (state == MoveStarting)
    {
        state = None;
        a->xAxis.autoLim = true;
        a->yAxis.autoLim = true;
        a->zAxis.autoLim = true;
        a->update();
    }
}

void QMDefController::mouseMoveEvent(QMouseEvent *event)
{
    if (state == None)
    {
        emit mouseMoving(event);
        return;
    }
    if (parent() == nullptr) return;
    QMWidget *a = static_cast<QMWidget *>(parent());

    if (state == Rot)
    {
        QPoint p = QCursor::pos();
        int dx = p.x() - pressedPos.x();
        int dy = p.y() - pressedPos.y();
        pressedPos = p;
        a->camera.azimuth -= 0.2f * dx;
        a->camera.elevation += 0.2f * dy;
        a->camera.azimuth = angle_norm_360(a->camera.azimuth);
        if (a->camera.elevation > 90) a->camera.elevation = 90;
        if (a->camera.elevation < -90) a->camera.elevation = -90;

        float nearest = nearestAngle(a->camera.elevation, 90);
        if (abs(a->camera.elevation - nearest) < 0.4)
            a->camera.elevation = nearest;

        nearest = nearestAngle(a->camera.azimuth, 90);
        if (abs(a->camera.azimuth - nearest) < 0.4)
            a->camera.azimuth = nearest;

        a->update();
    }

    if (state == Zoom)
    {
        a->zoomRect.point[0] = pressedPos;
        a->zoomRect.point[1] = event->pos();
        a->update();
    }

    if (state == Focus)
    {
        a->focusRect.point[0] = pressedPos;
        a->focusRect.point[1] = event->pos();
        a->update();
    }

    if (state == MoveStarting)
    {
        if (abs(event->pos().x() - pressedPos.x()) > 1 || abs(event->pos().y() - pressedPos.y()))
            state = Move;
    }

    if (state == Move)
    {
        QPoint p = QCursor::pos();
        if (p == pressedPos) return;
        a->ctlMove(p.x() - pressedPos.x(), p.y() - pressedPos.y());
        pressedPos = p;
        a->xAxis.autoLim = false;
        a->yAxis.autoLim = false;
        a->zAxis.autoLim = false;
        a->update();
    }
}

void QMDefController::wheelEvent(QWheelEvent *event)
{
    if (parent() == nullptr) return;
    QMWidget *a = static_cast<QMWidget *>(parent());
    a->xAxis.autoLim = false;
    a->yAxis.autoLim = false;
    a->zAxis.autoLim = false;
    a->ctlZoom(event->pos(), 1.0f / 120 * event->delta());
    a->update();
}

inline int char2nibble(char c)
{
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 0x0A;
    if (c >= 'A' && c <= 'F') return c - 'A' + 0x0A;
    return -1;
}

int parseColorStr(const char *s, QColor &out)
{
    int n = 0;
    for (int i = 0; n == 0; i++)
        if (char2nibble(s[i]) == -1)
        {
            n = i;
        }
        else
        {
            if (i == 5)
                n = 6;
        }
    int r = 0, g = 0, b = 0;
    switch (n)
    {
    case 0:
    case 2: // #AB     rg?
    case 4: // #ABCD   rrgg?
    case 5: // #ABCDE  rrggb?
        return 0;
    case 1: // #A
        r = char2nibble(s[0]);
        r = (r << 4) | r;
        g = r;
        b = r;
        break;
    case 3:
        r = char2nibble(s[0]);
        g = char2nibble(s[1]);
        b = char2nibble(s[2]);
        r = (r << 4) | r;
        g = (g << 4) | g;
        b = (b << 4) | b;
        break;
    case 6:
        r = (char2nibble(s[0]) << 4) | char2nibble(s[1]);
        g = (char2nibble(s[2]) << 4) | char2nibble(s[3]);
        b = (char2nibble(s[4]) << 4) | char2nibble(s[5]);
        break;
    }
    out = QColor(r, g, b);
    return n;
}

QMLineSpec::QMLineSpec()
{
    color = QColor(QColor::Invalid);
    width = 1;
    style = Solid;
    marker = 0;
}

QMLineSpec::QMLineSpec(const QString &linespec)
{
    color = QColor(QColor::Invalid);
    width = 1;
    style = Solid;
    marker = 0;
    setLineSpec(linespec);
}

void QMLineSpec::setLineSpec(const QString &linespec)
{
    const char *s = linespec.toStdString().c_str();
    style = None;
    switch (*s)
    {
    case ':':
        style = Dotted;
        s++;
        break;
    case '-':
        style = Solid;
        s++;
        switch (*s)
        {
        case '-':
            style = Dashed;
            s++;
            break;
        case '.':
            style = DashDot;
            s++;
            break;
        }
        break;
    }
    if (*s == '+' || *s == 'o' || *s == '*' || *s == '.' || *s == 'x' ||
        *s == 's' || *s == 'd' || *s == '^' || *s == 'v' || *s == '>' ||
        *s == '<' || *s == 'p' || *s == 'h')
    {
        marker = *s;
        s++;
    }
    switch (*s)
    {
    case 'r': color = QColor(Qt::red);     s++; break;
    case 'g': color = QColor(Qt::green);   s++; break;
    case 'b': color = QColor(Qt::blue);    s++; break;
    case 'c': color = QColor(Qt::cyan);    s++; break;
    case 'm': color = QColor(Qt::magenta); s++; break;
    case 'y': color = QColor(Qt::yellow);  s++; break;
    case 'k': color = QColor(Qt::black);   s++; break;
    case 'w': color = QColor(Qt::white);   s++; break;
    case '#':
        color = QColor(Qt::black);
        parseColorStr(s + 1, color);
        break;
    }
}

QMLegendItem::QMLegendItem(QWidget *parent) : QWidget(parent)
{
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);

    _licon = new QLabel(this);
    _licon->setText("?");
    _licon->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);

    _ltext = new QLabel(this);
    _ltext->setText("text");
    _ltext->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);

    setLinespec(QMLineSpec());

    QHBoxLayout *l = new QHBoxLayout(this);
    l->setSizeConstraint(QLayout::SetDefaultConstraint);
    l->setMargin(0);
    setLayout(l);
    l->insertWidget(0, _licon);
    l->insertWidget(1, _ltext);
}

void QMLegendItem::setText(const QString &text)
{
    _ltext->setText(text);
}

void QMLegendItem::setFont(const QFont &font)
{
    _ltext->setFont(font);
}

void QMLegendItem::setLinespec(const QMLineSpec &linespec)
{
    const Qt::PenStyle ls2ps[] =
    {
        Qt::NoPen, // None
        Qt::SolidLine, // Solid
        Qt::DashLine, // Dashed
        Qt::DotLine, // Dotted
        Qt::DashDotLine // DashDot
    };
    _ls = linespec;
    QImage img(40, 10, QImage::Format_ARGB32 );
    QPainter paint(&img);
    paint.eraseRect(img.rect());
    paint.setPen(QPen(_ls.color, 1, ls2ps[_ls.style]));
    paint.setBrush(_ls.color);
    paint.drawLine(0, 5, 39, 5);
    switch (_ls.marker)
    {
    case '.':
        paint.drawEllipse(QPoint(20, 5), 2, 2);
        break;
    case '+':
        paint.drawLine(20-3, 5-0, 20+3, 5+0);
        paint.drawLine(20-0, 5-3, 20+0, 5+3);
        break;
    }
    paint.end();
    _licon->setPixmap(QPixmap::fromImage(img));
}

QMLegendWidget::QMLegendWidget(QWidget &parent) :
    QFrame(&parent),
    bc(Qt::TopLeftCorner),
    bp(0, 0),
    moving(false)
{
    setVisible(false);
    parent.installEventFilter(this);
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    setStyleSheet("background-color:white;");
    setFrameShape(QFrame::Box);
    setLayout(new QVBoxLayout(this));
    layout()->setSizeConstraint(QLayout::SetNoConstraint);
    layout()->setMargin(2);
    layout()->setSpacing(2);
    setMouseTracking(true);
    moveToBindingPos();
}

QMLegendItem *QMLegendWidget::addItem(const QMLineSpec &linespec, const QString &text)
{
    QMLegendItem *item = new QMLegendItem(this);
    item->setLinespec(linespec);
    item->setText(text);
    QVBoxLayout *l = static_cast<QVBoxLayout *>(layout());
    l->addWidget(item);
    _items.push_back(item);
    return item;
}

void QMLegendWidget::removeItem(int index)
{
    delete _items[index];
    _items.remove(index);
}

void QMLegendWidget::clear()
{
    for (int i = 0; i < _items.count(); i++)
        delete _items[i];
    _items.clear();
}

void QMLegendWidget::setBindingCorner(Qt::Corner corner)
{
    bindCurrentPos();
    bc = corner;
    bindCurrentPos();
}

void QMLegendWidget::setBindingPosition(const QPoint &pos)
{
    bp = pos;
    moveToBindingPos();
}

void QMLegendWidget::move(const QPoint &v)
{
    QFrame::move(v);
    bindCurrentPos();
}

void QMLegendWidget::bindCurrentPos()
{
    if (parentWidget() == nullptr) return;
    bp.setX(bc == Qt::TopLeftCorner || bc == Qt::BottomLeftCorner ?
        pos().x() : parentWidget()->width() - width() - pos().x());
    bp.setY(bc == Qt::TopLeftCorner || bc == Qt::TopRightCorner ?
        pos().y() : parentWidget()->height() - height() - pos().y());
}

void QMLegendWidget::moveToBindingPos()
{
    if (parentWidget() == nullptr) return;
    int x = bc == Qt::TopLeftCorner || bc == Qt::BottomLeftCorner ?
        bp.x() : parentWidget()->width() - width() - bp.x();
    int y = bc == Qt::TopLeftCorner || bc == Qt::TopRightCorner ?
        bp.y() : parentWidget()->height() - height() - bp.y();
    QFrame::move(x, y);
}

void QMLegendWidget::mousePressEvent(QMouseEvent *e)
{
    if (e->button() != Qt::LeftButton) return;
    moving = true;
    ppos = QCursor::pos();
    wpos = pos();
    grabMouse();
}

void QMLegendWidget::mouseReleaseEvent(QMouseEvent *e)
{
    (void)e;
    moving = false;
    releaseMouse();
}

void QMLegendWidget::mouseMoveEvent(QMouseEvent *e)
{
    (void)e;
    if (!moving) return;
    move(wpos + QCursor::pos() - ppos);
}

bool QMLegendWidget::eventFilter(QObject *o, QEvent *e)
{
    if (parentWidget() != o)
    {
        o->removeEventFilter(this);
        return false;
    }
    if (parentWidget() == o && e->type() == QEvent::Resize)
    {
        moveToBindingPos();
    }
    return false;
}

void QMLegendWidget::resizeEvent(QResizeEvent *)
{
    moveToBindingPos();
}

//void QMLegendWidget::moveEvent(QMoveEvent *e)
//{
//    QFrame::moveEvent(e);
//    qDebug() << "size" << size() << "now" << pos() << "e.pos" << e->pos() << "e.old" << e->oldPos() << e->isAccepted();
//    bindCurrentPos();
//}

namespace qmplot {

QWidget *gcf()
{
    QMWidget *a = gca();
    if (a == nullptr) return nullptr;
    return a->parentWidget();
}

QMWidget *gca(QMWidget *setTo)
{
    if (setTo)
    {
        Q_ASSERT(qmwidgets.removeOne(setTo));
        qmwidgets.push_front(setTo);
    }
    if (qmwidgets.empty()) return nullptr;
    return qmwidgets[0];
}

QMWidget *axes(QWidget *parent)
{
    QLayout *l = nullptr;
    if (parent == nullptr)
    {
        parent = new QWidget();
        l = new QGridLayout(parent);
        parent->setLayout(l);
        parent->resize(700, 600);
        parent->show();
        parent->setAttribute(Qt::WA_DeleteOnClose);
    }
    QMWidget *res = new QMWidget(parent);
    if (l != nullptr) l->addWidget(res);
    return res;
}

QMWidget *needAxes()
{
    QMWidget *res = gca();
    if (res == nullptr)
        res = axes(nullptr);
    return res;
}

void hold(bool on)
{
    if (gca())
        gca()->hold = on;
}

void cla()
{
    if (gca())
    {
        gca()->clear();
        gca()->text.clear();
    }
}

void xlim(double min, double max, bool automode)
{
    if (gca() == nullptr) return;
    gca()->xAxis.manualLim[0] = min;
    gca()->xAxis.manualLim[1] = max;
    gca()->xAxis.autoLim = automode;
}

void ylim(double min, double max, bool automode)
{
    if (gca() == nullptr) return;
    gca()->yAxis.manualLim[0] = min;
    gca()->yAxis.manualLim[1] = max;
    gca()->yAxis.autoLim = automode;
}

void zlim(double min, double max, bool automode)
{
    if (gca() == nullptr) return;
    gca()->zAxis.manualLim[0] = min;
    gca()->zAxis.manualLim[1] = max;
    gca()->zAxis.autoLim = automode;
}

template <typename T>
QMLines *plot_base(int count, const T *x, const T *y, const T *z, int xstride, int ystride, int zstride, const QString &fmt)
{
    QMWidget *a = needAxes();
    if (a == nullptr) return nullptr;
    if (!a->hold) a->clear();
    QMLines *l = new QMLines(*a);
    l->linespec.setLineSpec(fmt);
    if (x) l->x.resize(count);
    if (y) l->y.resize(count);
    if (z) l->z.resize(count);
    for (int i = 0; i < l->x.count(); i++) l->x[i] = *(T *)((uint8_t *)x + xstride * i);
    for (int i = 0; i < l->y.count(); i++) l->y[i] = *(T *)((uint8_t *)y + ystride * i);
    for (int i = 0; i < l->z.count(); i++) l->z[i] = *(T *)((uint8_t *)z + zstride * i);
    l->dataChanged();
    a->update();
    return l;
}

QMLines *plot(const QString &fmt)
{
    QMWidget *a = needAxes();
    if (a == nullptr) return nullptr;
    if (!a->hold) a->clear();
    QMLines *l = new QMLines(*a);
    l->linespec.setLineSpec(fmt);
    return l;
}

QMLines *plot(int count, const float *x, const float *y, const float *z, const QString &fmt, int xstride, int ystride, int zstride)
{
    return plot_base(count, x, y, z, xstride, ystride, zstride, fmt);
}

QMLines *plot(int count, const double *x, const double *y, const double *z, const QString &fmt, int xstride, int ystride, int zstride)
{
    return plot_base(count, x, y, z, xstride, ystride, zstride, fmt);
}

void view(float az, float el)
{
    QMWidget *a = gca();
    if (a == nullptr) return;
    a->camera.autoMode = true;
    a->camera.azimuth = az;
    a->camera.elevation = el;
    a->update();
}

QMText *text(float x, float y, float z, const QString &string)
{
    QMWidget *a = gca();
    if (a == nullptr) return nullptr;
    QMText text;
    text.position = Vec3d(x, y, z);
    text.rotation = 0;
    text.string = string;
    a->text.push_back(text);
    return &a->text.last();
}

void title(const QString &string, Qt::Alignment alignment)
{
    QMWidget *a = gca();
    if (a == nullptr) return;
    a->title.string = string;
    a->title.align = alignment;
    a->update();
}

QMLegendWidget *legend(const QStringList &items)
{
    QMWidget *a = gca();
    if (a == nullptr) return nullptr;
    a->fillLegend(items);
    return &a->legend;
}

void xlabel(const QString &string)
{
    QMWidget *a = gca();
    if (a == nullptr) return;
    a->xAxis.label = string;
    a->update();
}

void ylabel(const QString &string)
{
    QMWidget *a = gca();
    if (a == nullptr) return;
    a->yAxis.label = string;
    a->update();
}

void zlabel(const QString &string)
{
    QMWidget *a = gca();
    if (a == nullptr) return;
    a->zAxis.label = string;
    a->update();
}


}

//QMMouseTool::QMMouseTool(QMWidget &widget, Qt::MouseButtons btn, Qt::KeyboardModifiers mod, bool enable) :
//    QObject(&widget), enable(enable), active(false), btn(btn), mod(mod)
//{
//    widget.installEventFilter(this);
//}

//QMMouseTool::~QMMouseTool()
//{
//    p()->removeEventFilter(this);
//}

//void QMMouseTool::setEnable(bool enable)
//{
//    this->enable = enable;
//    active = false;
//}

//void QMMouseTool::setCondition(Qt::MouseButtons btn, Qt::KeyboardModifiers mod)
//{
//    this->btn = btn;
//    this->mod = mod;
//}

//bool QMMouseTool::eventFilter(QObject *watched, QEvent *event)
//{
//    (void)watched;
//    if (!enable) return false;
//    QMouseEvent *me;
//    switch (event->type())
//    {
//    case QEvent::MouseButtonPress:
//    case QEvent::MouseButtonRelease:
//    case QEvent::MouseButtonDblClick:
//        me = (QMouseEvent *)event;
//        if (me->button() & btn)
//            return mouseFilteredEvent(event);
//        return false;
//    case QEvent::MouseMove:
//        me = (QMouseEvent *)event;
//        if (me->modifiers() == mod || mod == 0)
//            return moveFilteredEvent(event);
//        return false;
//    case QEvent::Wheel:
//        return wheelFilteredEvent(event);
//    default:
//        return false;
//    }
//}

//QMZoomRectTool::QMZoomRectTool(QMWidget &widget, Qt::MouseButtons btn, Qt::KeyboardModifiers mod, bool enable) :
//    QMMouseTool(widget, btn, mod, enable)
//{
//}

//bool QMZoomRectTool::filtered(QEvent *event)
//{
//    if (!active)
//    {
//        if (event->type() != QEvent::MouseButtonPress)
//    }
//}
