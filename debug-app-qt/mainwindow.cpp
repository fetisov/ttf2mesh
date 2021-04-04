#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <QDir>
#include <assert.h>
#include <algorithm>
#include <math.h>
#include <time.h>

static const struct
{
    uint32_t start;
    uint32_t stop;
    const char *name;
} uranges[] =
{
    {1, 0, "Unknown"},
    URANGES
};

static const int uranges_len = sizeof(uranges) / sizeof(uranges[0]);

static inline int get_urange_index(uint32_t utf)
{
    for (int i = 1; i < uranges_len; i++)
        if (utf >= uranges[i].start && utf <= uranges[i].stop)
            return i;
    return 0;
}

extern "C" {

typedef struct mesher_vert_to_edge_link v2e_t;
typedef struct mesher_vertex_struct     mvs_t;
typedef struct mesher_circumcircle      mcc_t;
typedef struct mesher_edge_struct       mes_t;
typedef struct mesher_triangle_struct   mts_t;

struct mesher_vert_to_edge_link
{
    v2e_t *next;
    v2e_t *prev;
    mes_t *edge;
};

struct mesher_vertex_struct
{
    float x;
    float y;
    int contour;
    int subglyph;
    bool is_hole;
    int nested_to;
    int object;
    v2e_t edges;
    mvs_t *next_in_contour;
    mvs_t *prev_in_contour;
    int index;
};

struct mesher_circumcircle
{
    float radius;
    float center[2];
};

struct mesher_edge_struct
{
    mes_t *next;
    mes_t *prev;
    mvs_t *v1;
    mvs_t *v2;
    mts_t *tr[2];
    mcc_t alt_cc[2];
    int index;
};

struct mesher_triangle_struct
{
    mts_t *next; /* Следующий треугольник в списке */
    mts_t *prev; /* Предыдущий треугольник в списке */
    mes_t *edge[3]; /* Ссылки на 3 образующих ребра */
    mcc_t cc;
    int helper; /* Поле для разных задач */
};

typedef struct
{
    int maxv;  /* Максимальное число вершин */
    int maxe;  /* Максимальное число рёбер */
    int maxt;  /* Максимальное число треугольников */
    int nv;    /* Фактическое число вершин в v (может быть меньше maxv) */
    mvs_t *v; /* Пул вершин, массив длиной maxv */
    mes_t *e; /* Пул рёбер, массив длиной maxe */
    mts_t *t; /* Пул треугольников, массив длиной maxt */
    v2e_t *l; /* Пул ссылок вершина->ребро, массив длиной (2 * maxe) */
    mes_t efree; /* Корень списка свободных рёбер */
    mes_t eused; /* Корень списка использованных рёбер */
    mts_t tfree; /* Корень списка свободных треугольников */
    mts_t tused; /* Корень списка использованных треугольников */
    v2e_t lfree; /* Корень списка свободных ссылок вершина->ребро */
    mes_t convx; /* Корень списка рёбер в выпуклой оболочке */
    mvs_t vinit[2]; /* Две инициализационные точки по нижней границе глифа */
    mvs_t **s; /* Сортированный по (y) массив вершин */
    struct /* Отладочные поля */
    {
        int curr_step; /* Текущий шаг алгоритма */
        char message[64]; /* Текстовое сообщение при отладке или ошибке */
        int stop_at_step; /* Шаг, на котором следует прервать работу алгоритма */
        bool breakpoint; /* Если заполнено true, то будет сформирован SIGINT */
    } debug;
} mesher_t;

#define MESHER_DONE 0
#define MESHER_WARN 1
#define MESHER_FAIL 2
#define MESHER_TRAP 3

extern mesher_t *create_mesher(const ttf_outline_t *o);
extern int mesher(mesher_t *m, int deep);
extern void free_mesher(mesher_t *m);

static __inline int count_triangles(const mesher_t *m)
{
    int res = 0;
    for (mts_t *t = m->tused.next; t != &m->tused; t = t->next)
        res++;
    return res;
}

}

namespace qmwrap {
extern float randn(float mu, float sigma);
}

using namespace qmwrap;
using namespace qmplot;

uint64_t utime(void)
{
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC, &t);
    return (uint64_t)t.tv_sec * 1000000 + t.tv_nsec / 1000;
}

class FontTest
{
public:
    class Issue
    {
    public:
        Issue() {}
        Issue(const QString &link);
        Issue(int code, const QString &font, int utf, int failed_at_step = -1, const char *message = "", int nvert = 0)
            : code(code), utf(utf), font(font), failed_at_step(failed_at_step), message(message), nvert(nvert) {}
        QString toLink() const;
        int code;
        int utf;
        QString font;
        int failed_at_step;
        QString message;
        int nvert;

        static bool compareByNvert(const Issue &lhs, const Issue &rhs)
        {
          return lhs.nvert < rhs.nvert;
        }
    };

    class TimeRep
    {
    public:
        TimeRep() {}
        TimeRep(int nvert, int elapsed) : nvert(nvert), elapsed(elapsed) {}
        int nvert;
        int elapsed;
    };

public:
    FontTest()
    {
        clearStat();
    }

    void clearStat()
    {
        issue.clear();
        huge10.clear();
        memset(&total, 0, sizeof(total));
        memset(&utf_errors, 0, sizeof(utf_errors));
    }

    void makeFontTest(ttf_t *ttf, int quality, int optim)
    {
        total.fonts++;

        QTime t = QTime::currentTime();
        for (int i = 0; i < ttf->nchars; i++)
        {
            int id = ttf->char2glyph[i];
            ttf_outline_t *o = ttf_linear_outline(ttf->glyphs + id, quality);
            if (o == NULL) continue;
            if (o->total_points < 3)
            {
                ttf_free_outline(o);
                continue;
            }
            mesher_t *m = create_mesher(o);
            uint64_t elapsed = utime();
            int code = mesher(m, optim);
            elapsed = utime() - elapsed;

            if (code == MESHER_DONE)
            {
                total.glyphs++;
                total.done++;
                total.vert += m->nv;
                total.triangles += count_triangles(m);
                timerep << TimeRep(m->nv, (int)elapsed);

                int nvert = huge10.count() == 0 ? 0 : huge10[0].nvert;
                if (m->nv > nvert)
                {
                    Issue I(0, ttf->names.full_name, ttf->chars[i], -1, "", m->nv);
                    if (huge10.count() < 10)
                        huge10 << I; else
                        huge10[0] = I;
                    std::sort(huge10.begin(), huge10.end(), Issue::compareByNvert);
                }
            }

            if (code == MESHER_WARN)
            {
                total.glyphs++;
                total.warn++;
                timerep << TimeRep(m->nv, (int)elapsed);
                issue << Issue(code, ttf->names.full_name, ttf->chars[i]);
            }

            if (code == MESHER_FAIL)
            {
                total.glyphs++;
                total.fail++;
                utf_errors[get_urange_index(ttf->chars[i])]++;
                issue << Issue(code, ttf->names.full_name, ttf->chars[i], m->debug.curr_step, m->debug.message);
            }

            free_mesher(m);
            ttf_free_outline(o);
        }
        total.time += t.elapsed();
    }

    QString makeHtmlRep();

public:
    Vec<Issue> issue;
    Vec<TimeRep> timerep;
    Vec<Issue> huge10;
    struct
    {
        int fonts;
        int glyphs;
        int time;
        int vert;
        int triangles;
        int done;
        int warn;
        int fail;
    } total;
    int utf_errors[uranges_len];
};

FontTest::Issue::Issue(const QString &link)
    : code(0), utf(0), failed_at_step(-1), message(""), nvert(0)
{
    if (link.left(12) != "app://debug/") return;
    QStringList sl = link.mid(12).split('/', QString::KeepEmptyParts);
    if (sl.count() != 5) return;
    code = sl[0].toInt();
    font = QByteArray::fromBase64(sl[1].toUtf8());
    utf = sl[2].toInt(NULL, 16);
    failed_at_step = sl[3].toInt();
    message = QByteArray::fromBase64(sl[4].toUtf8());
}

QString FontTest::Issue::toLink() const
{
    QString link = "app://debug/";
    link += QString::number(code) + "/";
    link += font.toUtf8().toBase64() + "/";
    link += QString::number(utf, 16) + "/";
    link += QString::number(failed_at_step) + "/";
    link += message.toUtf8().toBase64();
    return link;
}

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    memset(args, 0, sizeof(args));
    fonts = ttf_list_system_fonts(NULL);
    assert(fonts != NULL);
    assert(fonts[0] != NULL);
    assert(ttf_load_from_file(fonts[0]->filename, &ttf, false) == TTF_DONE);

    g = ttf->glyphs;
    lastSelectedUtf = 'A';
    updating = 1;

    ui->setupUi(this);
    setupFontList();

    ui->curves->setMouseTracking(true);
    ui->bTools->setMenu(ui->menuTools);

    updating = 0;
    updateUi();


//    srand(time(NULL));
//    float r[4] = {fabs(randn(0, 1)), fabs(randn(0, 1)), fabs(randn(0, 1)), fabs(randn(0, 1))};
//    dvt v[4] =
//    {
//        {cos(0.0 / 180 * M_PI) * r[0],  sin(0.0 / 180 * M_PI) * r[0]},
//        {cos(50.00 / 180 * M_PI) * r[1], sin(50.00 / 180 * M_PI) * r[1]},
//        {cos(150.0 / 180 * M_PI) * r[2], sin(150.0 / 180 * M_PI) * r[2]},
//        {cos(200.0 / 180 * M_PI) * r[3], sin(200.0 / 180 * M_PI) * r[3]},
//    };
//    axes();
//    hold(true);
//    plot(dvec() << v[0].x << v[1].x << v[2].x << v[3].x << v[0].x,
//         dvec() << v[0].y << v[1].y << v[2].y << v[3].y << v[0].y, "-k");
//    if (is_convex_quad(v+0, v+1, v+2, v+3))
//        title("CONVEX"); else
//        title("NOT CONVEX");


//    srand(time(NULL));
//    Vec2f v[3] =
//    {
//        Vec2f(randn(0, 1), randn(0, 1)),
//        Vec2f(randn(0, 1), randn(0, 1)),
//        Vec2f(randn(0, 1), randn(0, 1))
//    };
//    Vec2f dir[2] =
//    {
//        v[1] - v[0],
//        v[2] - v[1]
//    };

//    axes();
//    hold(true);
//    plot(dvec() << v[0].x() << v[1].x() << v[2].x(),
//         dvec() << v[0].y() << v[1].y() << v[2].y(), "-k");
//    text(v[0].x(), v[0].y(), 0, "1");
//    text(v[1].x(), v[1].y(), 0, "2");
//    text(v[2].x(), v[2].y(), 0, "3");

//    Vec<Vec2f> done;
//    Vec<Vec2f> error;
//    for (int i = 0; i < 360; i++)
//    {
//        float angle = M_PI * i / 180.0;
//        Vec2f pt(cos(angle), sin(angle));

//        if (is_middle_line(dir[0].v, dir[1].v, pt.v))
//            done.push_back(pt + v[1]); else
//            error.push_back(pt + v[1]);
//    }
//    plot(done.count(), done.data()->v + 0, done.data()->v + 1, NULL, ".b", sizeof(Vec2f), sizeof(Vec2f));
//    plot(error.count(), error.data()->v + 0, error.data()->v + 1, NULL, ".r", sizeof(Vec2f), sizeof(Vec2f));


    //QDir dir("/usr/share/fonts/truetype/liberation");
//    QDir dir("/home/fse/develope/ttf/fonts");
//    QStringList fonts = dir.entryList(QStringList() << "*.ttf", QDir::Files);

//    while (1)
//    for (int i = 0; i < fonts.count(); i++)
//    {
//        QString font = dir.absoluteFilePath(fonts[i]);
//        assert(ttf_load_file(font.toLocal8Bit().data(), &ttf) == 0);
//        ttf_free(ttf);
//    }


//    QString svg;
//    g = ttf->glyphs + ttf_find_glyph(ttf, 'o');

//    svg += "<svg width=\"200\" height=\"200\" xmlns=\"http://www.w3.org/2000/svg\">\n";
//    svg += "<g transform=\"scale(200,200) translate(0,0.5)\">";
//    char *s = ttf_glyph2svgpath(g);
//    svg += s;
//    free(s);
//    svg += "</g>";
//    svg += "</svg>";

//    QFile f("1.svg");
//    f.open(QIODevice::WriteOnly);
//    f.write(svg.toUtf8());
//    f.close();
//exit(0);
}

MainWindow::~MainWindow()
{
    delete ui;
}

static __inline float qbezier(float p0, float p1, float p2, float t)
{
    float tt = 1.0f - t;
    return tt * tt * p0 + 2.0f * t * tt * p1 + t * t * p2;
}

static __inline float qbezier_diff1(float p0, float p1, float p2, float t)
{
    // (1-t)^2 * p0 + 2.0f * t * (1-t) * p1 + t^2 * p2
    // p0 - 2 t p0 + t^2 p0 + 2 t p1 - 2 t^2 p1 + t^2 p2
    // 2 [t (p0 - 2 p1 + p2) - p0 + p1]
    return 2.0f * (t * (p0 - 2.0f * p1 + p2) - p0 + p1);
}

static __inline int square_solve(float a, float b, float c, float *x1, float *x2)
{
    float d = b * b - 4.0f * a * c;
    if (d < 0 || fabs(a) < 1e-3f) return 0;
    d = sqrt(d);
    *x1 = (-b - d) / (2.0f * a);
    if (d < 1e-3) return 1;
    *x2 = (-b + d) / (2.0f * a);
    return 2;
}

void pushArrow(const ttf_point_t p[2], fvec &x, fvec &y)
{
    Vec2f e1 = Vec2f(&p[1].x) - Vec2f(&p[0].x);
    Vec2f e2 = Vec2f(e1.y(), -e1.x());
    Vec2f arrow[7] =
    {
        Vec2f(0, 0),
        Vec2f(1, 0),
        Vec2f(NAN, NAN),
        Vec2f(0.8f, 0.05f),
        Vec2f(1, 0),
        Vec2f(0.8f, -0.05f),
        Vec2f(NAN, NAN)
    };
    for (int i = 0; i < 7; i++)
    {
        Vec2f v = arrow[i].x() * e1 + arrow[i].y() * e2 + Vec2f(&p[0].x);
        x << v.x();
        y << v.y();
    }
}

void MainWindow::updateCurvesPlot()
{
    ttf_outline_t *o, *linear;

    gca(ui->curves);
    cla();
    hold(true);

    if (ttf == NULL || g == NULL) return;
    if (g->outline == NULL) return;

    linear = ttf_linear_outline(g, ui->smooth->isChecked() ? 128 : 0);
    o = linear;
    for (int i = 0; i < o->ncontours; i++)
    {
        fvec x;
        fvec y;
        if (o->cont[i].length > 1)
            pushArrow(o->cont[i].pt, x, y);
        for (int j = 0; j < o->cont[i].length; j++)
        {
            x << o->cont[i].pt[j].x;
            y << o->cont[i].pt[j].y;
        }
        x << o->cont[i].pt[0].x;
        y << o->cont[i].pt[0].y;
        plot(x, y);
    }

    switch (ui->showCurvePoints->currentIndex())
    {
    case 0: o = g->outline; break;
    case 1: o = linear; break;
    default: o = NULL;
    }

    if (o != NULL)
    {
        fvec x, xoc;
        fvec y, yoc;
        for (int i = 0; i < o->ncontours; i++)
            for (int j = 0; j < o->cont[i].length; j++)
                if (o->cont[i].pt[j].onc)
                {
                    xoc << o->cont[i].pt[j].x;
                    yoc << o->cont[i].pt[j].y;
                }
                else
                {
                    x << o->cont[i].pt[j].x;
                    y << o->cont[i].pt[j].y;
                }
        plot(xoc, yoc, ".k");
        plot(x, y, ".#999");
        for (int i = 0; i < o->ncontours; i++)
            for (int j = 0; j < o->cont[i].length; j++)
                text(o->cont[i].pt[j].x, o->cont[i].pt[j].y, 0,
                     QString().sprintf("p%i", j))->font.setPixelSize(10);
    }

    fvec x;
    fvec y;
    x << g->xbounds[0] << g->xbounds[0] << g->xbounds[1] << g->xbounds[1] << g->xbounds[0];
    y << g->ybounds[0] << g->ybounds[1] << g->ybounds[1] << g->ybounds[0] << g->ybounds[0];
    plot(x, y, ":k");

    x.clear();
    y.clear();
    x << g->lbearing << g->lbearing << NAN << g->advance << g->advance;
    y << g->ybounds[0] << g->ybounds[1] << NAN << g->ybounds[0] << g->ybounds[1];
    plot(x, y, "-k");

    ttf_free_outline(linear);
}

bool line_intersect(float p, float p0, float p1, float *t, int *sign)
{
    // linear interpolation
    // y0 + (y1 - y0) * t = y
    // t = (y - y0) / (y1 - y0)
    *t = p1 - p0;
    if (fabs(*t) < 1e-3) return 0;
    *t = (p - p0) / *t;
    if (*t < 0.0f || *t >= 1.0f) return 0;
    *sign = p1 > p0 ? 1 : -1;
    return true;
}

int ttf_point_xcmp(const void *p1, const void *p2)
{
    return ((ttf_point_t *)p1)->x - ((ttf_point_t *)p2)->x;
}

#include "poly2tri/poly2tri.h"

using namespace p2t;

QString mesherDebugStr(int code, const mesher_t *m, int time)
{
    QString res;
    if (code == MESHER_DONE || code == MESHER_WARN)
        res = QString().sprintf(
            "<b>%s</b><br>"
            "Elapsed: %i ms<br>"
            "Output: %i triangles",
            code == MESHER_DONE ? "DONE" : "Mesher WARNS",
            time,
            count_triangles(m));
    if (code == MESHER_TRAP)
        res = QString().sprintf(
            "<b>Trapped</b><br>"
            "At step %i<br>"
            "State: %s",
            m->debug.curr_step,
            m->debug.message);
    if (code == MESHER_FAIL)
        res = QString().sprintf(
            "<b>!!! FAILED !!!</b><br>"
            "At step %i<br>"
            "(%s)",
            m->debug.curr_step,
            m->debug.message);
    return res;
}

void MainWindow::updateDebugPage()
{
    gca(ui->temp);
    cla();
    hold(true);
    ui->temp->update();
    ui->debugState->setText("");

    if (ttf == NULL || g == NULL) return;
    if (g->outline == NULL) return;

    QTime time = QTime::currentTime();
    ttf_outline_t *o = ttf_linear_outline(g, ui->quality->value());
    if (o == NULL) return;
    if (o->total_points < 3)
    {
        ttf_free_outline(o);
        return;
    }
    mesher_t *mesh = create_mesher(o);

    mesh->debug.stop_at_step = ui->stopAt->value();
    mesh->debug.breakpoint = sender() == ui->trap;
    int res = mesher(mesh, ui->deep->value());

    int sweeping_point = -1;
    sscanf(mesh->debug.message, "sweeping point %i", &sweeping_point);

    ui->debugState->setText(mesherDebugStr(res, mesh, time.elapsed()));

    fvec x, y;
    for (int i = 0; i < mesh->nv; i++)
    {
        x << mesh->v[i].x << mesh->v[i].prev_in_contour->x << NAN;
        y << mesh->v[i].y << mesh->v[i].prev_in_contour->y << NAN;
    }
    if (ui->drawOutline->isChecked())
        plot(x, y, ":k");

    x.clear();
    y.clear();
    int last_contour = -1;
    for (int i = 0; i < mesh->nv; i++)
    {
        x << mesh->v[i].x;
        y << mesh->v[i].y;
        if (ui->pnums->isChecked())
        {
            QMText *t = text(mesh->v[i].x, mesh->v[i].y, 0, QString("p%1").arg(mesh->v[i].index));
            t->font.setPixelSize(10);
            if (mesh->v[i].index == sweeping_point)
                t->font.setBold(true);
        }

        if (last_contour != mesh->v[i].contour)
        {
            QMText *txt = text(mesh->v[i].x, mesh->v[i].y, 0, QString("%1%2N%3")
                               .arg(mesh->v[i].is_hole ? "H" : "C")
                               .arg(mesh->v[i].contour)
                               .arg(mesh->v[i].nested_to));
            txt->font.setPixelSize(12);
            txt->align = Qt::AlignTop;
        }
        last_contour = mesh->v[i].contour;

    }
    plot(x, y, ".k");

    for (mts_t *ptr = mesh->tused.next; ptr != &mesh->tused; ptr = ptr->next)
    {
        mts_t &t = *ptr;
        x.clear();
        y.clear();

        if (t.edge[0] == NULL) continue;

        x << NAN << t.edge[0]->v1->x << t.edge[0]->v2->x;
        x << NAN << t.edge[1]->v1->x << t.edge[1]->v2->x;
        x << NAN << t.edge[2]->v1->x << t.edge[2]->v2->x;

        y << NAN << t.edge[0]->v1->y << t.edge[0]->v2->y;
        y << NAN << t.edge[1]->v1->y << t.edge[1]->v2->y;
        y << NAN << t.edge[2]->v1->y << t.edge[2]->v2->y;

        plot(x, y, "-k");

        float sumx = t.edge[0]->v1->x + t.edge[0]->v2->x +
                     t.edge[1]->v1->x + t.edge[1]->v2->x +
                     t.edge[2]->v1->x + t.edge[2]->v2->x;
        float sumy = t.edge[0]->v1->y + t.edge[0]->v2->y +
                     t.edge[1]->v1->y + t.edge[1]->v2->y +
                     t.edge[2]->v1->y + t.edge[2]->v2->y;

        if (ui->tnums->isChecked())
        {
            QMText *tt = text(sumx / 6, sumy / 6, 0, QString("t%1").arg(ptr - mesh->t));
            tt->font.setPixelSize(10);
            tt->align = Qt::AlignCenter;
            if (ui->enums->isChecked())
            {
                tt = text(sumx / 6, sumy / 6, 0, QString("(e%1/%2/%3)")
                    .arg(t.edge[0]->index)
                    .arg(t.edge[1]->index)
                    .arg(t.edge[2]->index));
                tt->font.setPixelSize(8);
                tt->align = Qt::AlignTop;
            }
        }
    }

    x.clear();
    y.clear();
    for (mes_t *e = mesh->convx.next; e != &mesh->convx; e = e->next)
    {
        x << NAN << e->v1->x << e->v2->x;
        y << NAN << e->v1->y << e->v2->y;
    }
  //plot(x, y, ":#999");
    const char *style = QString(mesh->debug.message) == "sweep finishing" ? "-b" : "-r";
    plot(x, y, style);

    x.clear();
    y.clear();
    for (mes_t *e = mesh->eused.next; e != &mesh->eused; e = e->next)
    {
        if (e->tr[0] == NULL)
        {
            x << NAN << e->v1->x << e->v2->x;
            y << NAN << e->v1->y << e->v2->y;
        }
        if (ui->enums->isChecked())
        {
            float mid[2] = {(e->v1->x + e->v2->x) / 2, (e->v1->y + e->v2->y) / 2};
            text(mid[0], mid[1], 0, QString("e%1").arg(e->index))->font.setPixelSize(10);
        }
    }
    plot(x, y, "--#999");

    ttf_free_outline(o);
    free_mesher(mesh);
}

void MainWindow::updateTextPage()
{
    ui->renderer->font = ttf;
    ui->renderer->text = ui->text->toPlainText();
    ui->renderer->textHeight = ui->textHeight->value();
    ui->renderer->meshQuality = ui->textQuality->value();
    ui->renderer->update();
    qApp->processEvents();
    ui->numTriangles->setText(QString("%1 triangles output").arg(ui->renderer->numTriangles));
}

MatchingArg::MatchingArg(QWidget *parent) : QWidget(parent)
{
    type = 0;

    hide();
    QGridLayout *l = new QGridLayout(this);
    l->setVerticalSpacing(0);
    setLayout(l);
    title = new QLabel(this);
    title->hide();
    l->addWidget(title, 0, 0, 1, 1);

    weights = new QComboBox(this);
    weights->addItem("TTF_WEIGHT_THIN");
    weights->addItem("TTF_WEIGHT_EXTRALIGHT");
    weights->addItem("TTF_WEIGHT_LIGHT");
    weights->addItem("TTF_WEIGHT_NORMAL");
    weights->addItem("TTF_WEIGHT_MEDIUM");
    weights->addItem("TTF_WEIGHT_DEMIBOLD");
    weights->addItem("TTF_WEIGHT_BOLD");
    weights->addItem("TTF_WEIGHT_EXTRABOLD");
    weights->addItem("TTF_WEIGHT_BLACK");
    weights->hide();
    l->addWidget(weights, 1, 0, 1, 1);

    family = new QLineEdit(this);
    family->hide();
    l->addWidget(family, 2, 0, 1, 1);

    text = new QLineEdit(this);
    text->hide();
    l->addWidget(text, 3, 0, 1, 1);

    connect(weights, SIGNAL(currentIndexChanged(int)), this, SLOT(edited()));
    connect(family, SIGNAL(textChanged(QString)), this, SLOT(edited()));
    connect(text, SIGNAL(textChanged(QString)), this, SLOT(edited()));
}

void MatchingArg::setType(char type)
{
    if (type >= 'A' && type <= 'Z')
        type = type - 'A' + 'a';

    if (type == this->type) return;

    hide();
    title->hide();
    weights->hide();
    family->hide();
    text->hide();

    switch (type)
    {
    case 'b': title->setText("Matching bold..."); title->show(); break;
    case 'i': title->setText("Matching italic..."); title->show(); break;
    case 'h': title->setText("Matching font with hollow glyps..."); title->show(); break;
    case 'o': title->setText("Matching oblique font..."); title->show(); break;
    case 'r': title->setText("Matching regular font..."); title->show(); break;
    case 'w': weights->show(); break;
    case 'f': family->show(); break;
    case 't': text->show(); break;
    default:
        this->type = 0;
        return;
    }

    this->type = type;
    show();
}

bool MatchingArg::has_arg()
{
    return type == 'w' || type == 'f' || type == 't';
}

size_t MatchingArg::get_arg()
{
    uint16_t *t;
    switch (type)
    {
    case 'w': return (weights->currentIndex() + 1) * 100;
    case 'f':
        buff = family->text().toUtf8();
        return (size_t)buff.data();
    case 't':
        buff.resize(text->text().length() * 2 + 2);
        t = (uint16_t *)buff.data();
        for (int i = 0; i < text->text().length(); i++)
            *t++ = text->text().at(i).unicode();
        *t = 0;
        return (size_t)buff.data();
    default: return 0;
    }
}

void MatchingArg::edited()
{
    emit chanded();
}

QString fontInfoStr(const ttf_t *ttf, int index, int count)
{
    QString macStyle = QString("%1%2%3%4%5%6%7")
        .arg(ttf->head.macStyle.bold ? "bold " : "")
        .arg(ttf->head.macStyle.italic ? "italic " : "")
        .arg(ttf->head.macStyle.underline ? "underline " : "")
        .arg(ttf->head.macStyle.outline ? "outline " : "")
        .arg(ttf->head.macStyle.shadow ? "shadow " : "")
        .arg(ttf->head.macStyle.condensed ? "condensed " : "")
        .arg(ttf->head.macStyle.extended ? "extended " : "");

    QString os2 = QString("<b>sFamilyClass</b> %1 <b>usWeightClass</b> %2 <b>usWidthClass</b> %3 <b>panose</b> %4 <b>fsSelection</b> %5")
        .arg(ttf->os2.sFamilyClass)
        .arg(ttf->os2.usWeightClass)
        .arg(ttf->os2.usWidthClass)
        .arg(QString("%1%2%3%4%5%6%7%8%9%10")
            .arg(ttf->os2.panose[0]).arg(ttf->os2.panose[1]).arg(ttf->os2.panose[2]).arg(ttf->os2.panose[3]).arg(ttf->os2.panose[4])
            .arg(ttf->os2.panose[5]).arg(ttf->os2.panose[6]).arg(ttf->os2.panose[7]).arg(ttf->os2.panose[8]).arg(ttf->os2.panose[9]))
        .arg(QString("%1%2%3%4%5%6%7%8%9")
            .arg(ttf->os2.fsSelection.italic ? "italic " : "")
            .arg(ttf->os2.fsSelection.underscore ? "underscore " : "")
            .arg(ttf->os2.fsSelection.negative ? "negative " : "")
            .arg(ttf->os2.fsSelection.outlined ? "outlined " : "")
            .arg(ttf->os2.fsSelection.strikeout ? "strikeout " : "")
            .arg(ttf->os2.fsSelection.bold ? "bold " : "")
            .arg(ttf->os2.fsSelection.regular ? "regular " : "")
            .arg(ttf->os2.fsSelection.utm ? "utm " : "")
            .arg(ttf->os2.fsSelection.oblique ? "oblique " : ""));

    QString res = QString("<b>font file</b> %1<br>"
                          "<b>font</b> %2 from %3, <b>full name</b> %4, <b>version</b> %5, <b>glyphs</b> %6<br>"
                          "<b>family</b> %7, <b>subfamily</b> %8, <b>macStyle</b> %9<br>"
                          "<b>OS/2:</b> %10")
                                .arg(ttf->filename)
                                .arg(index + 1)
                                .arg(count)
                                .arg(ttf->names.full_name)
                                .arg(ttf->names.version)
                                .arg(ttf->nglyphs)
                                .arg(ttf->names.family)
                                .arg(ttf->names.subfamily)
                                .arg(macStyle)
                                .arg(os2);

    return res;
}

void MainWindow::updateMatchingPage()
{
    if (args[0] == NULL)
    {
        ui->arguments->setLayout(new QVBoxLayout(ui->arguments));
        ui->arguments->layout()->setSpacing(2);
        for (int i = 0; i < 32; i++)
        {
            args[i] = new MatchingArg(ui->arguments);
            ui->arguments->layout()->addWidget(args[i]);
            connect(args[i], SIGNAL(chanded()), this, SLOT(updateUi()));
        }
    }


    QByteArray buff = ui->requirements->text().toUtf8();
    const char *s = buff.data();
    size_t a[32] = {0};
    int nargs = 0;
    for (int i = 0; i < 32; i++)
    {
        if (*s == 0)
        {
            for (int j = i; j < 32; j++)
                args[j]->setType(0);
            break;
        }
        char req = *s++;
        bool exact = *s == '!';
        if (exact) s++;
        args[i]->setType(req);
        if (args[i]->has_arg())
            a[nargs++] = args[i]->get_arg();
    }
    ttf_t *m = ttf_list_match(fonts, NULL, buff.data(),
        a[0], a[1], a[2], a[3], a[4], a[5], a[6], a[7],
        a[8], a[9], a[10], a[11], a[12], a[13], a[14], a[15],
        a[16], a[17], a[18], a[19], a[20], a[21], a[22], a[23],
        a[24], a[25], a[26], a[27], a[28], a[29], a[30], a[31]);
    if (m == NULL)
        ui->matched->setText("matched: NULL"); else
        ui->matched->setText(fontInfoStr(m, 0, 0));
}

void MainWindow::applyTestResults(FontTest *t)
{
    QWidget *tab = new QWidget(this);
    tab->setLayout(new QVBoxLayout());
    QLabel *l1 = new QLabel("<a href=\"app://close_stat_tab\">Close this tab</a> or <a href=\"app://save_stat_to\">Save report to...</a>", tab);
    QScrollArea *sa = new QScrollArea(tab);
    QLabel *l2 = new QLabel(t->makeHtmlRep(), sa);
    l2->setTextInteractionFlags(Qt::TextBrowserInteraction);
    QMWidget *qm = new QMWidget(tab);
    qm->setFixedHeight(250);
    sa->setWidget(l2);
    tab->layout()->addWidget(l1);
    tab->layout()->addWidget(sa);
    tab->layout()->addWidget(qm);

    struct
    {
        QWidget *tab;
        FontTest *t;
    } userdata = { tab, t };
    l1->setProperty("data", QVariant(QByteArray((char *)&userdata, sizeof(userdata))));

    connect(l1, SIGNAL(linkActivated(QString)), this, SLOT(onAppLink(QString)));
    connect(l2, SIGNAL(linkActivated(QString)), this, SLOT(onAppLink(QString)));

    dvec x, y;
    for (int i = 0; i < t->timerep.count(); i++)
    {
        x << t->timerep[i].nvert;
        y << t->timerep[i].elapsed;
    }
    gca(qm);
    plot(x, y, ".");
    xlabel("Number of vertices");
    ylabel("Time (usec)");

    ui->tabWidget->addTab(tab, "Stat");
    ui->tabWidget->setCurrentWidget(tab);
}

void MainWindow::updateFontInfo()
{
    if (ttf == NULL)
    {
        ui->fontInfo->setText("");
        return;
    }
    int index = ui->fonts->currentIndex();
    int count = ui->fonts->count();
    ui->fontInfo->setText(fontInfoStr(ttf, index, count));
}

void MainWindow::updateUi()
{
    if (updating) return;
    updateGlyphsList();
    updateFontInfo();
    if (ui->tabWidget->currentIndex() == 0)
        updateCurvesPlot();
    if (ui->tabWidget->currentIndex() == 1)
        updateDebugPage();
    if (ui->tabWidget->currentIndex() == 2)
        updateTextPage();
    if (ui->tabWidget->currentIndex() == 3)
        updateMatchingPage();
}

void MainWindow::setupFontList()
{
    ui->fonts->clear();
    for (int i = 0; fonts[i] != NULL; i++)
        ui->fonts->addItem(QString("%1").arg(fonts[i]->names.full_name));
}

void MainWindow::updateGlyphsList()
{
    if (updating) return;
    updating++;
    if (ttf == NULL)
    {
        ui->glyph->setProperty("csum", 0);
        ui->glyph->clear();
        updating--;
        return;
    }

    if (ui->glyph->property("csum").toUInt() != (uint)ttf->glyf_csum ||
        ui->glyph->count() != ttf->nchars)
    {
        QStringList sl;
        sl.reserve(ttf->nchars);
        for (int i = 0; i < ttf->nchars; i++)
        {
            QString s;
            s += QString("[%1] glyph %2, u+%3 / %4")
                .arg(ttf->chars[i] < ' ' ? "" : QString::fromUtf16(ttf->chars + i, 1))
                .arg(ttf->char2glyph[i])
                .arg(QString::number(ttf->chars[i], 16))
                .arg((unsigned)ttf->chars[i]);
            sl.push_back(s);
        }
        ui->glyph->clear();
        ui->glyph->addItems(sl);
        ui->glyph->setProperty("csum", (uint)ttf->glyf_csum);
    }

    if (g != NULL)
        for (int i = 0; i < ttf->nchars; i++)
            if (g == ttf->glyphs + ttf->char2glyph[i])
            {
                ui->glyph->setCurrentIndex(i);
                break;
            }

    updating--;
}

//void MainWindow::onTestFont()
//{
//    if (ttf == NULL) return;

//    struct
//    {
//        int nchars;
//        int nvert;
//        int time;
//        int ne;
//        int nt;
//        int fails;
//    } stat = {0, 0, 0, 0, 0, 0};
//    stat.nchars = ttf->nchars;
//    int Q = ui->quality->value();
//    int O = ui->deep->value();

//    QTime t = QTime::currentTime();
//    for (int i = 0; i < ttf->nchars; i++)
//    {
//        int id = ttf->char2glyph[i];
//        ttf_outline_t *o = ttf_linear_outline(ttf->glyphs + id, Q, 0);
//        if (o == NULL) continue;
//        QTime t1 = QTime::currentTime();
//        mesher_t *m = create_mesher(o);
//        if (mesher(m, O) != 0)
//        {
//            stat.fails++;
//            fprintf(stderr, "mesher failed at glyph %i symbol #%i (U+%04X)\n", i, ttf->chars[i], ttf->chars[i]);
//        }
//        stat.nvert += o->total_points;
//        stat.ne += m->ne;
//        stat.nt += m->nt;
//        free_mesher(m);
//        ttf_free_outline(o);
//        if (t1.elapsed() > 10)
//        {
//            fprintf(stderr, "mesher timeout %ims at glyph %i symbol #%i (U+%04X)\n",
//                    t1.elapsed(), i, ttf->chars[i], ttf->chars[i]);
//        }
//    }
//    int elapsed = t.elapsed();
//    fflush(stderr);

//    QString s;
//    s = "<table>"
//        "<tr><td>Данные</td><td>%1 вершин в %2 символах</td></tr>"
//        "<tr><td>Производительность</td><td>%3 мс/символ</td></tr>"
//        "<tr><td>Создано</td><td>%4 тр-в на %5 рёбрах</td></tr>"
//        "<tr><td>Неудач</td><td>%6</td></tr>"
//        "</tr>";
//    s = s
//        .arg(stat.nvert).arg(stat.nchars)
//        .arg(0.01 * (elapsed * 100 / stat.nchars))
//        .arg(stat.nt).arg(stat.ne)
//        .arg(stat.fails);

//    QMessageBox::information(NULL, "Статистика", s);
//}

void MainWindow::onTestFont()
{
    if (ttf == NULL) return;
    FontTest *test = new FontTest();
    test->makeFontTest(ttf, ui->quality->value(), ui->deep->value());
    applyTestResults(test);
}

void MainWindow::onTestAllFonts()
{
    FontTest *test = new FontTest();
    for (int i = 0; fonts[i] != NULL; i++)
    {
        ttf_t *font;
        assert(ttf_load_from_file(fonts[i]->filename, &font, false) == TTF_DONE);
        test->makeFontTest(font, ui->quality->value(), ui->deep->value());
        ttf_free(font);
    }
    applyTestResults(test);
}

void MainWindow::onFontSelect()
{
    if (updating) return;
    int index = ui->fonts->currentIndex();
    ttf_free(ttf);
    if (ttf_load_from_file(fonts[index]->filename, &ttf, false) != TTF_DONE)
    {
        ttf = NULL;
        g = NULL;
        updateUi();
        QMessageBox::information(NULL, "Error", "Unable to load font!");
        return;
    }
    if (ttf_find_glyph(ttf, lastSelectedUtf) != -1)
        g = ttf->glyphs + ttf_find_glyph(ttf, lastSelectedUtf); else
        g = ttf->glyphs;
    updateUi();
}

void MainWindow::onGlyphSelect()
{
    if (updating) return;
    if (ttf == NULL) return;
    int index = ui->glyph->currentIndex();
    if (index < 0 || index >= ttf->nchars)
    {
        g = ttf->glyphs;
    }
    else
    {
        g = ttf->glyphs + ttf->char2glyph[index];
        lastSelectedUtf = ttf->chars[index];
    }
    updateUi();
}

void MainWindow::onLoadDir()
{
    QString dir = QFileDialog::getExistingDirectory(NULL);
    if (dir.isEmpty()) return;
    QByteArray array = dir.toLocal8Bit();

    const char *dir_list[1] = {array.data()};

    ttf_t **list = ttf_list_fonts(dir_list, 1, NULL);
    if (list == NULL) return;
    if (list[0] == NULL)
    {
        ttf_free_list(list);
        QMessageBox::information(NULL, "Loading fonts", "There are no fonts in the selected directory");
        return;
    }
    if (ttf_load_from_file(list[0]->filename, &ttf, false) != TTF_DONE)
    {
        ttf_free_list(list);
        return;
    }
    ttf_free_list(fonts);
    fonts = list;
    g = ttf->glyphs;
    updating = 1;
    setupFontList();
    updating = 0;
    updateUi();

}

void MainWindow::onLoadSys()
{
    ttf_t **list = ttf_list_system_fonts(NULL);
    if (list == NULL) return;
    if (list[0] == NULL)
    {
        ttf_free_list(list);
        return;
    }
    if (ttf_load_from_file(list[0]->filename, &ttf, false) != TTF_DONE)
    {
        ttf_free_list(list);
        return;
    }
    ttf_free_list(fonts);
    fonts = list;
    g = ttf->glyphs;
    updating = 1;
    setupFontList();
    updating = 0;
    updateUi();
}

void make_poly2tri_data(ttf_point_t *p, int count, std::vector<Point> &points, std::vector<Point *> &ppoints)
{
    points.resize(count);
    ppoints.resize(count);
    for (int i = 0; i < count; i++)
    {
        points[i].x = p[i].x;
        points[i].y = p[i].y;
        ppoints[i] = &points[i];
    }
}

void MainWindow::onPoly2tri()
{
    if (g == NULL) return;
    ttf_outline_t *o = ttf_linear_outline(g, ui->quality->value());
    if (o == NULL) return;
    QTime t = QTime::currentTime();
    std::vector<Point> points;
    std::vector<Point *> ppoints;
    make_poly2tri_data(o->cont[0].pt, o->cont[0].length, points, ppoints);
    CDT cdt(ppoints);
    std::vector<Point> hpoints;
    std::vector<Point *> phpoints;
    if (o->ncontours > 1)
    {
        make_poly2tri_data(o->cont[1].pt, o->cont[1].length, hpoints, phpoints);
        cdt.AddHole(phpoints);
    }
    cdt.Triangulate();
    std::vector<Triangle *> tri = cdt.GetTriangles();
    int dt = t.elapsed();
    axes();
    hold(true);
    for (size_t i = 0; i < tri.size(); i++)
    {
        Point *p1 = tri[i]->GetPoint(0);
        Point *p2 = tri[i]->GetPoint(1);
        Point *p3 = tri[i]->GetPoint(2);
        plot(fvec() << p1->x << p2->x << p3->x << p1->x,
             fvec() << p1->y << p2->y << p3->y << p1->y,
             "-k");
    }
    title(QString("Time %1 ms").arg(dt));
}

void MainWindow::onGlyph2Obj()
{
    if (g == NULL) return;
    QString fn = QFileDialog::getSaveFileName(NULL, "Save to ASE", "", "OBJ file (*.obj)");
    ttf_mesh_t *m;
    if (ttf_glyph2mesh(g, &m, TTF_QUALITY_NORMAL, 0) != TTF_DONE) return;
    QString s;
    for (int i = 0; i < m->nvert; i++)
        s += QString().sprintf("v %.5f %.5f 0.0\n", m->vert[i].x, m->vert[i].y);
    for (int i = 0; i < m->nfaces; i++)
        s += QString().sprintf("f %i %i %i\n",
                               m->faces[i].v1 + 1,
                               m->faces[i].v2 + 1,
                               m->faces[i].v3 + 1);
    ttf_free_mesh(m);
    QFile file(fn);
    if (!file.open(QIODevice::WriteOnly))
    {
        QMessageBox::information(NULL, "Error saving", "Unable to save data to file");
        return;
    }
    file.write(s.toUtf8());
    file.close();
}

void MainWindow::onFont2Obj()
{
    if (g == NULL) return;
    QString fn = QFileDialog::getSaveFileName(NULL, "Save to OBJ", "", "OBJ file (*.obj)");
    ttf_export_to_obj(ttf, fn.toUtf8().data(), TTF_QUALITY_NORMAL);
}

void MainWindow::onAppLink(const QString &link)
{
    if (link.left(12) == "app://debug/")
    {
        FontTest::Issue issue(link);
        for (int i = 0; fonts[i] != NULL; i++)
            if (fonts[i]->names.full_name == issue.font)
            {
                ttf_t *f;
                if (ttf_load_from_file(fonts[i]->filename, &f, false) != TTF_DONE)
                {
                    QMessageBox::information(NULL, "ERROR", QString("Unable to load font \"%1\"").arg(issue.font));
                    return;
                }
                int id = ttf_find_glyph(f, issue.utf);
                if (id == -1)
                {
                    ttf_free(f);
                    return;
                }
                ttf_free(ttf);
                ttf = f;
                g = f->glyphs + id;

                updating++;
                ui->fonts->setCurrentIndex(i);
                ui->tabWidget->setCurrentWidget(ui->debugTab);
                updating--;
                updateUi();
                return;
            }
    }
    if (link == "app://close_stat_tab")
    {
        struct
        {
            QWidget *tab;
            FontTest *t;
        } userdata;
        QByteArray a = sender()->property("data").toByteArray();
        if (a.size() != sizeof(userdata)) return;
        memcpy(&userdata, a.data(), a.size());
        if (userdata.t != NULL) delete userdata.t;
        delete userdata.tab;
    }
    if (link == "app://save_stat_to")
    {
        struct
        {
            QWidget *tab;
            FontTest *t;
        } userdata;
        QByteArray a = sender()->property("data").toByteArray();
        if (a.size() != sizeof(userdata)) return;
        memcpy(&userdata, a.data(), a.size());
        if (userdata.t == NULL) return;
        QString fn = QString("TTF debug ") + QDateTime::currentDateTime().toString("dd.MM.yyyy hh:mm") + ".html";
        fn = QDir(qApp->applicationDirPath()).absoluteFilePath(fn);
        fn = QFileDialog::getSaveFileName(NULL, "Save report as...", fn, "HTML file (*.html)");
        if (fn.isEmpty()) return;
        QFile f(fn);
        if (!f.open(QIODevice::WriteOnly))
        {
            QMessageBox::information(NULL, "Error", "Unable to save file");
            return;
        }
        f.write(userdata.t->makeHtmlRep().toUtf8());
        f.close();
        return;
    }

}

QString FontTest::makeHtmlRep()
{
    QString utf_errors_str;
    for (int i = 0; i < uranges_len; i++)
        if (utf_errors[i] != 0)
            utf_errors_str += QString("<br>%1 - %2 errors").arg(uranges[i].name).arg(utf_errors[i]);
    QString s;
    s +=
        "<html>"
        "<style>"
            "table"
            "{"
            "width: 780px;"
            "border: 1px solid black;"
            "border-spacing: 0px 0px;"
            "border-collapse: collapse;"
            "}"
        "</style>"
        "<body>";
    s += "<b>Total statistics</b>:";
    s += QString().sprintf(
                "<table border=\"1\">"
                "<tr><td>Total time</td><td>%i</td></tr>"
                "<tr><td>Fonts tested</td><td>%i</td></tr>"
                "<tr><td>Glyphs</td><td>%i</td></tr>"
                "<tr><td>DONE</td><td>%i</td></tr>"
                "<tr><td>WARN</td><td>%i (%.1f%%)</td></tr>"
                "<tr><td>FAIL</td><td>%i (%.1f%%)%s</td></tr>"
                "<tr><td>Performance</td><td>%.0f glyph/sec</td></tr>"
                "<tr><td>Time per glyph</td><td>%.3f ms</td></tr>"
                "</table>",
                total.time,
                total.fonts,
                total.glyphs,
                total.done,
                total.warn, 100.0 * total.warn / total.glyphs,
                total.fail, 100.0 * total.fail / total.glyphs, utf_errors_str.toUtf8().data(),
                1000.0 * total.glyphs / total.time,
                1.0 * total.time / total.glyphs);
    if (huge10.count() > 0)
    {
        s += "<b>Top10 Huge glyphs</b>:";
        s += "<table border=\"1\">"
             "<tr>"
             "<td>Font</td><td>Symbol</td><td>UTF Range</td><td>Vertices</td><td></td>"
             "</tr>";
        for (int i = 0; i < huge10.count(); i++)
        {
            s += QString().sprintf(
                        "<tr>"
                        "<td>%s</td>"
                        "<td>U+%04X</td>"
                        "<td>%s</td>"
                        "<td>%i</td>"
                        "<td><a href=\"%s\">View</a></td>"
                        "</tr>",
                        huge10[i].font.toUtf8().data(),
                        huge10[i].utf,
                        uranges[get_urange_index(huge10[i].utf)].name,
                        huge10[i].nvert,
                        huge10[i].toLink().toUtf8().data());
        }
        s += "</table>";
    }
    if (total.fail > 0)
    {
        s += "<b>Errors</b>:";
        s += "<table border=\"1\">"
             "<tr>"
             "<td>Font</td><td>Symbol</td><td>UTF Range</td><td>State</td><td>Message</td><td></td>"
             "</tr>";
        for (int i = 0; i < issue.count() && i < 1000; i++)
        {
            if (issue[i].code != MESHER_FAIL) continue;
            s += QString().sprintf(
                        "<tr>"
                        "<td>%s</td>"
                        "<td>U+%04X</td>"
                        "<td>%s</td>"
                        "<td>%i</td>"
                        "<td>%s</td>"
                        "<td><a href=\"%s\">Debug</a></td>"
                        "</tr>",
                        issue[i].font.toUtf8().data(),
                        issue[i].utf,
                        uranges[get_urange_index(issue[i].utf)].name,
                        issue[i].failed_at_step,
                        issue[i].message.toUtf8().data(),
                        issue[i].toLink().toUtf8().data());
        }
        s += "</table>";
    }
    if (total.warn > 0)
    {
        s += "<b>Warnings</b>:";
        s += "<table border=\"1\">"
             "<tr>"
             "<td>Font</td><td>Symbol</td><td></td>"
             "</tr>";
        for (int i = 0; i < issue.count() && i < 1000; i++)
        {
            if (issue[i].code != MESHER_WARN) continue;
            s += QString().sprintf(
                        "<tr>"
                        "<td>%s</td>"
                        "<td>U+%04X</td>"
                        "<td><a href=\"%s\">View glyph</a></td>"
                        "</tr>",
                        issue[i].font.toUtf8().data(),
                        issue[i].utf,
                        issue[i].toLink().toUtf8().data());
        }
        s += "</table>";
    }
    s += "</html>";
    return s;
}
