#include "linalgd.h"

#ifdef __cplusplus
extern "C" {
#endif

const vec2d vec2d00 = { 0, 0 };
const vec2d vec2d10 = { 1, 0 };
const vec2d vec2d01 = { 0, 1 };
const vec3d vec3d000 = { 0, 0, 0 };
const vec3d vec3d100 = { 1, 0, 0 };
const vec3d vec3d010 = { 0, 1, 0 };
const vec3d vec3d001 = { 0, 0, 1 };
const mat2d mat2deye =
{
    1, 0,
    0, 1
};
const mat3d mat3deye =
{
    1, 0, 0,
    0, 1, 0,
    0, 0, 1
};
const mat4d mat4deye =
{
    1, 0, 0, 0,
    0, 1, 0, 0,
    0, 0, 1, 0,
    0, 0, 0, 1
};

#include <assert.h>

static void linalgd_mat_testbench(void)
{
    int i;
    mat3d m3, M3;
    mat4d m4, M4;

    /* mat3d */
    MAT3(m3, 0%7, 1%7, 2%7,
              3%7, 4%7, 5%7,
              6%7, 7%7, 8%7);
    /* mat3d transpone */
    m3d_t(m3, m3);
    m3d_t(m3, m3);
    for (i = 0; i < 9; i++)
        assert(m3[i] == (i%7));
    /* mat3d assign, inv, mul */
    m3d_assign(M3, m3);
    m3d_inv(m3, m3);
    m3d_mul_safe(M3, m3, M3);
    for (i = 0; i < 9; i++)
        assert(fabs(M3[i] - mat3deye[i]) < 1e-5);
    /* mat3d rotz, roty, rotz */
    m3d_rotx(m3, 1);
    m3d_roty(M3, 2);
    m3d_mul_safe(m3, M3, m3);
    m3d_rotz(M3, 3);
    m3d_mul_safe(m3, M3, m3);
    m3d_t(M3, m3);
    m3d_mul_safe(M3, m3, M3);
    for (i = 0; i < 9; i++)
        assert(fabs(M3[i] - mat3deye[i]) < 1e-5);

    /* mat4d */
    MAT4(m4, 0%7,  1%7,  2%7,  3%7,
              4%7,  5%7,  6%7,  7%7,
              8%7,  9%7,  10%7, 11%7,
              12%7, 13%7, 14%7, 15%7);
    /* mat3d transpone */
    m4d_t(m4, m4);
    m4d_t(m4, m4);
    for (i = 0; i < 16; i++)
        assert(m4[i] == (i%7));
    /* mat3d assign, inv, mul */
    m4d_assign(M4, m4);
    m4d_inv(m4, m4);
    m4d_mul_safe(M4, m4, M4);
    for (i = 0; i < 16; i++)
        assert(fabs(M4[i] - mat4deye[i]) < 1e-5);
    /* mat4d rotz, roty, rotz */
    m4d_rotx(m4, 1);
    m4d_roty(M4, 2);
    m4d_mul_safe(m4, M4, m4);
    m4d_rotz(M4, 3);
    m4d_mul_safe(m4, M4, m4);
    m4d_t(M4, m4);
    m4d_mul_safe(M4, m4, M4);
    for (i = 0; i < 9; i++)
        assert(fabs(M4[i] - mat4deye[i]) < 1e-5);
}

static void linalgd_projection_testbench(void)
{
    mat4d tmp, modelview, proj;
    vec3d vview, vmodel;
    /* modelview matrix */
    m4d_rotx(modelview, 1);
    m4d_roty(tmp, 2);
    m4d_mul_safe(modelview, tmp, modelview);
    m4d_rotz(tmp, 3);
    m4d_mul_safe(modelview, tmp, modelview);
    m4d_viewport(tmp, 0, 0, 200, 100, 0, 1);
    m4d_mul_safe(modelview, tmp, modelview);
    /* proj matrix */
    m4d_ortho(proj, 0, 200, 100, 0, 0, 1000);
    /* projection/unprojection */
    v3d_set(vmodel, 1,2,3);
    v3d_project(vview, vmodel, modelview, proj, 200, 100);
    v3d_unproject(vmodel, vview, modelview, proj, 200, 100);
    assert(fabs(vmodel[0] - 1) + fabs(vmodel[1] - 2) + fabs(vmodel[2] - 3) < 1e-4);
    /* projection/fast unprojection */
    v3d_set(vmodel, 1,2,3);
    v3d_project(vview, vmodel, modelview, proj, 200, 100);
    m4d_inv(modelview, modelview);
    m4d_inv(proj, proj);
    v3d_unproject_fast(vmodel, vview, modelview, proj, 200, 100);
    assert(fabs(vmodel[0] - 1) + fabs(vmodel[1] - 2) + fabs(vmodel[2] - 3) < 1e-4);
}

void linalgd_testbench(void)
{
    linalgd_mat_testbench();
    linalgd_projection_testbench();
}

void m3d_mul(mat3d res, const mat3d a, const mat3d b)
{
    res[0] = a[0]*b[0] + a[1]*b[3] + a[2]*b[6];
    res[1] = a[0]*b[1] + a[1]*b[4] + a[2]*b[7];
    res[2] = a[0]*b[2] + a[1]*b[5] + a[2]*b[8];

    res[3] = a[3]*b[0] + a[4]*b[3] + a[5]*b[6];
    res[4] = a[3]*b[1] + a[4]*b[4] + a[5]*b[7];
    res[5] = a[3]*b[2] + a[4]*b[5] + a[5]*b[8];

    res[6] = a[6]*b[0] + a[7]*b[3] + a[8]*b[6];
    res[7] = a[6]*b[1] + a[7]*b[4] + a[8]*b[7];
    res[8] = a[6]*b[2] + a[7]*b[5] + a[8]*b[8];
}

void m4d_mul(mat4d res, const mat4d a, const mat4d b)
{
    res[0] = a[0]*b[0] + a[1]*b[4] + a[2]*b[8] + a[3]*b[12];
    res[1] = a[0]*b[1] + a[1]*b[5] + a[2]*b[9] + a[3]*b[13];
    res[2] = a[0]*b[2] + a[1]*b[6] + a[2]*b[10] + a[3]*b[14];
    res[3] = a[0]*b[3] + a[1]*b[7] + a[2]*b[11] + a[3]*b[15];

    res[4] = a[4]*b[0] + a[5]*b[4] + a[6]*b[8] + a[7]*b[12];
    res[5] = a[4]*b[1] + a[5]*b[5] + a[6]*b[9] + a[7]*b[13];
    res[6] = a[4]*b[2] + a[5]*b[6] + a[6]*b[10] + a[7]*b[14];
    res[7] = a[4]*b[3] + a[5]*b[7] + a[6]*b[11] + a[7]*b[15];

    res[8] = a[8]*b[0] + a[9]*b[4] + a[10]*b[8] + a[11]*b[12];
    res[9] = a[8]*b[1] + a[9]*b[5] + a[10]*b[9] + a[11]*b[13];
    res[10] = a[8]*b[2] + a[9]*b[6] + a[10]*b[10] + a[11]*b[14];
    res[11] = a[8]*b[3] + a[9]*b[7] + a[10]*b[11] + a[11]*b[15];

    res[12] = a[12]*b[0] + a[13]*b[4] + a[14]*b[8] + a[15]*b[12];
    res[13] = a[12]*b[1] + a[13]*b[5] + a[14]*b[9] + a[15]*b[13];
    res[14] = a[12]*b[2] + a[13]*b[6] + a[14]*b[10] + a[15]*b[14];
    res[15] = a[12]*b[3] + a[13]*b[7] + a[14]*b[11] + a[15]*b[15];
}

static __inline double det2d(const double *m, int s, int r1, int c1, int r2, int c2)
{
    return m[r1*s + c1] * m[r2*s + c2] - m[r1*s + c2] * m[r2*s + c1];
}

static __inline double det3d(const double *m, int s, int r1, int c1, int r2, int c2, int r3, int c3)
{
    /*
      r1c1 r1c2 r1c3
      r2c1 r2c2 r2c3
      r3c1 r3c2 r3c3
    */
    vec3d v;
    v[0] = +det2d(m, s, r2, c2, r3, c3);
    v[1] = -det2d(m, s, r2, c1, r3, c3);
    v[2] = +det2d(m, s, r2, c1, r3, c2);
    return m[r1*s + c1] * v[0] + m[r1*s + c2] * v[1] + m[r1*s + c3] * v[2];
}

double m3d_det(const mat3d m)
{
    return det3d(m, 3, 0, 0, 1, 1, 2, 2);
}

double m4d_det(const mat4d m)
{
    /*
    00 01 02 03
    10 11 12 13
    20 21 22 23
    30 31 32 33
    */
    return det3d(m, 4,  1, 1,  2, 2,  3, 3) * m[0] -
           det3d(m, 4,  1, 0,  2, 2,  3, 3) * m[1] +
           det3d(m, 4,  1, 0,  2, 1,  3, 3) * m[2] -
           det3d(m, 4,  1, 0,  2, 1,  3, 2) * m[3];
}

bool m2d_inv(mat2d res, const mat2d m)
{
    double tmp;
    double det = m[0] * m[3] - m[1] * m[2];
    if (det == 0) return false; /* FIX! */
    det = 1.0 / det;
    tmp = m[0];
    res[0] = det * m[3];
    res[3] = det * tmp;
    res[1] = -det * m[1];
    res[2] = -det * m[2];
    return true;
}

bool m3d_inv(mat3d res, const mat3d m)
{
    vec3d co;
    double det, s;
    v3d_set(co, det2d(m, 3, 1, 1, 2, 2), det2d(m, 3, 1, 2, 2, 0), det2d(m, 3, 1, 0, 2, 1));
    det = v3d_dot(co, &m[0]);
    if (det == 0) return false; /* FIX! */
    s = 1.0f / det;
    m3d_set(res, co[0] * s, det2d(m, 3, 0, 2, 2, 1) * s, det2d(m, 3, 0, 1, 1, 2) * s,
                 co[1] * s, det2d(m, 3, 0, 0, 2, 2) * s, det2d(m, 3, 0, 2, 1, 0) * s,
                 co[2] * s, det2d(m, 3, 0, 1, 2, 0) * s, det2d(m, 3, 0, 0, 1, 1) * s);
    return true;
}

bool m4d_inv(mat4d res, const mat4d m)
{
    mat4d M;
    double det;
    int i;

    /*
    00 01 02 03
    10 11 12 13
    20 21 22 23
    30 31 32 33
    */
    M[0*4+0] = +det3d(m, 4, 1,1, 2,2, 3,3);
    M[0*4+1] = -det3d(m, 4, 1,0, 2,2, 3,3);
    M[0*4+2] = +det3d(m, 4, 1,0, 2,1, 3,3);
    M[0*4+3] = -det3d(m, 4, 1,0, 2,1, 3,2);

    M[1*4+0] = -det3d(m, 4, 0,1, 2,2, 3,3);
    M[1*4+1] = +det3d(m, 4, 0,0, 2,2, 3,3);
    M[1*4+2] = -det3d(m, 4, 0,0, 2,1, 3,3);
    M[1*4+3] = +det3d(m, 4, 0,0, 2,1, 3,2);

    M[2*4+0] = +det3d(m, 4, 0,1, 1,2, 3,3);
    M[2*4+1] = -det3d(m, 4, 0,0, 1,2, 3,3);
    M[2*4+2] = +det3d(m, 4, 0,0, 1,1, 3,3);
    M[2*4+3] = -det3d(m, 4, 0,0, 1,1, 3,2);

    M[3*4+0] = -det3d(m, 4, 0,1, 1,2, 2,3);
    M[3*4+1] = +det3d(m, 4, 0,0, 1,2, 2,3);
    M[3*4+2] = -det3d(m, 4, 0,0, 1,1, 2,3);
    M[3*4+3] = +det3d(m, 4, 0,0, 1,1, 2,2);

    det =
        M[0*4+0] * m[0*4+0] +
        M[0*4+1] * m[0*4+1] +
        M[0*4+2] * m[0*4+2] +
        M[0*4+3] * m[0*4+3];
    if (det == 0.0f) return false; /* FIX! */
    det = 1.0f / det;

    for (i = 0; i < 16; i++)
    {
        int r = i >> 2;
        int c = i & 3;
        res[i] = M[c*4 + r] * det;
    }

    return true;
}

void m4d_ortho(mat4d res, double left, double right, double bottom, double top, double nearPlane, double farPlane)
{
    double width, invheight, clip;
    width = right - left;
    invheight = top - bottom;
    clip = farPlane - nearPlane;

    #define m(c,r) res[r*4+c]

    m(0, 0) = 2.0f / width;
    m(1, 0) = 0.0f;
    m(2, 0) = 0.0f;
    m(3, 0) = -(left + right) / width;

    m(0, 1) = 0.0f;
    m(1, 1) = 2.0f / invheight;
    m(2, 1) = 0.0f;
    m(3, 1) = -(top + bottom) / invheight;

    m(0, 2) = 0.0f;
    m(1, 2) = 0.0f;
    m(2, 2) = -2.0f / clip;
    m(3, 2) = -(nearPlane + farPlane) / clip;

    m(0, 3) = 0.0f;
    m(1, 3) = 0.0f;
    m(2, 3) = 0.0f;
    m(3, 3) = 1.0f;

    #undef m
}

void m4d_frustum(mat4d res, double left, double right, double bottom, double top, double nearPlane, double farPlane)
{
    double width, invheight, clip;

    width = right - left;
    invheight = top - bottom;
    clip = farPlane - nearPlane;

    #define m(c,r) res[r*4+c]
    m(0, 0) = 2.0f * nearPlane / width;
    m(1, 0) = 0.0f;
    m(2, 0) = (left + right) / width;
    m(3, 0) = 0.0f;

    m(0, 1) = 0.0f;
    m(1, 1) = 2.0f * nearPlane / invheight;
    m(2, 1) = (top + bottom) / invheight;
    m(3, 1) = 0.0f;

    m(0, 2) = 0.0f;
    m(1, 2) = 0.0f;
    m(2, 2) = -(nearPlane + farPlane) / clip;
    m(3, 2) = -2.0f * nearPlane * farPlane / clip;

    m(0, 3) = 0.0f;
    m(1, 3) = 0.0f;
    m(2, 3) = -1.0f;
    m(3, 3) = 0.0f;
    #undef m
}

void m4d_perspective(mat4d res, double verticalAngle, double aspectRatio, double nearPlane, double farPlane)
{
    double radians, sine, cotan, clip;
    radians = verticalAngle / 2.0f / 180.0f * M_PI;
    sine = sin(radians);
    cotan = cos(radians) / sine;
    clip = farPlane - nearPlane;

    #define m(c,r) res[r*4+c]
    m(0, 0) = cotan / aspectRatio;
    m(1, 0) = 0.0f;
    m(2, 0) = 0.0f;
    m(3, 0) = 0.0f;
    m(0, 1) = 0.0f;
    m(1, 1) = cotan;
    m(2, 1) = 0.0f;
    m(3, 1) = 0.0f;
    m(0, 2) = 0.0f;
    m(1, 2) = 0.0f;
    m(2, 2) = -(nearPlane + farPlane) / clip;
    m(3, 2) = -(2.0f * nearPlane * farPlane) / clip;
    m(0, 3) = 0.0f;
    m(1, 3) = 0.0f;
    m(2, 3) = -1.0f;
    m(3, 3) = 0.0f;
    #undef m
}

void m4d_viewport(mat4d res, double left, double bottom, double width, double height, double nearPlane, double farPlane)
{
    double w2, h2;
    w2 = width / 2.0f;
    h2 = height / 2.0f;

    #define m(c,r) res[r*4+c]
    m(0, 0) = w2;
    m(1, 0) = 0.0f;
    m(2, 0) = 0.0f;
    m(3, 0) = left + w2;
    m(0, 1) = 0.0f;
    m(1, 1) = h2;
    m(2, 1) = 0.0f;
    m(3, 1) = bottom + h2;
    m(0, 2) = 0.0f;
    m(1, 2) = 0.0f;
    m(2, 2) = (farPlane - nearPlane) / 2.0f;
    m(3, 2) = (nearPlane + farPlane) / 2.0f;
    m(0, 3) = 0.0f;
    m(1, 3) = 0.0f;
    m(2, 3) = 0.0f;
    m(3, 3) = 1.0f;
    #undef m
}

double v3d_project(vec3d res, const vec3d v, const mat4d modelView, const mat4d projection, int viewport_w, int viewport_h)
{
    mat4d m;
    double w;
    /* projection transformation: */
    /* projection = translate(PROJECTION * MODELVIEW * vector) */
    m4d_mul(m, projection, modelView);
    w = m4d_mul_v3d(res, m, v);
    v3d_scale(res, res, 1.0 / w);
    res[0] = (res[0] * 0.5 + 0.5) * viewport_w;
    res[1] = (res[1] * 0.5 + 0.5) * viewport_h;
    res[2] = res[2] * 0.5 + 0.5;
    return w;
}

void v3d_unproject(vec3d res, const vec3d v, const mat4d modelView, const mat4d projection, int viewport_w, int viewport_h)
{
    vec3d vv;
    mat4d m;
    double w;
    /* unprojection transformation: */
    /* inv(PROJECTION * MODELVIEW) * inv_translate(projection) = vector */
    vv[0] = v[0] / viewport_w * 2 - 1;
    vv[1] = v[1] / viewport_h * 2 - 1;
    vv[2] = v[2] * 2 - 1;
    m4d_mul(m, projection, modelView);
    m4d_inv(m, m);
    w = m4d_mul_v3d(res, m, vv);
    w = 1.0 / w;
    res[0] *= w;
    res[1] *= w;
    res[2] *= w;
}

void v3d_unproject_fast(vec3d res, const vec3d v, const mat4d inv_modelview, const mat4d inv_projection, int viewport_w, int viewport_h)
{
    vec3d vv;
    mat4d m;
    double w;
    /* unprojection transformation: */
    /* inv(PROJECTION * MODELVIEW) * inv_translate(projection) = vector */
    /* inv(MODELVIEW) * inv(PROJECTION) * inv_translate(projection) = vector */
    vv[0] = v[0] / viewport_w * 2 - 1;
    vv[1] = v[1] / viewport_h * 2 - 1;
    vv[2] = v[2] * 2 - 1;
    m4d_mul(m, inv_modelview, inv_projection);
    w = m4d_mul_v3d(res, m, vv);
    w = 1.0 / w;
    res[0] *= w;
    res[1] *= w;
    res[2] *= w;
}

LINALG_TRANSL bool v3d_lnp_intersection(vec3d res, const vec3d d, const vec3d p, const vec4d plane, double *dist)
{
    //  line
    //    p
    //     \ d
    //    __\_____
    //   /   \   / plane
    //  /     \ /
    // /___/___/x
    //    /
    //   / n, D
    //
    // plane: normal vector n, point n*D
    // line: direction vector d, point p
    // intersection: vector x
    //
    // { vec x = vec p + vec d * c, where c - unknown
    // { vec n * (vec x - vec n * D) = 0
    //
    // vec n * (vec p + vec d * c - vec n * D) = 0
    // vec n * vec p + vec n * vec d * c - D = 0
    // c = (D - vec n * vec p) / (vec n * vec d)

    double cc = v3d_dot(&plane[0], d);
    if (fabs(cc) < 1e-40) return false;
    cc = (plane[3] - v3d_dot(&plane[0], p)) / cc;
    if (dist != NULL) *dist = cc;
    if (res != NULL)
    {
        res[0] = p[0] + d[0] * cc;
        res[1] = p[1] + d[1] * cc;
        res[2] = p[2] + d[2] * cc;
    }
    return true;
}

static __inline bool linsolved_exchange_rows(double **A, double *B, int row, int n, double eps)
{
    int i, j;
    double max = fabs(A[row][row]);
    j = row;
    for (i = row + 1; i < n; i++)
    {
        double v = fabs(A[i][row]);
        if (v <= max) continue;
        max = v;
        j = i;
    }
    if (max < eps) return false;
    if (j != row)
    {
        double *t1, t2;
        t1 = A[row];
        A[row] = A[j];
        A[j] = t1;
        t2 = B[row];
        B[row] = B[j];
        B[j] = t2;
    }
    return true;
}

static __inline int linsolved_max(double *v, int n)
{
    int res = 0;
    double max = fabs(v[0]);
    for (int i = 1; i < n; i++)
    {
        double a = fabs(v[i]);
        if (max >= a) continue;
        max = a;
        res = i;
    }
    return res;
}

bool linsolved_base(double **A, double *B, double *S, int size, double eps)
{
    int i, j, k;
    /* normalization */
    for (i = 0; i < size; i++)
    {
        double c;
        int cmax;
        cmax = linsolved_max(A[i], size);
        c = A[i][cmax];
        if (fabs(c) < eps) return false;
        c = 1.0 / c;
        for (j = 0; j < size; j++)
            A[i][j] *= c;
        B[i] *= c;
    }
    /* gauss */
    for (i = 0; i < size; i++)
    {
        if (!linsolved_exchange_rows(A, B, i, size, eps))
            return false;
        double v = A[i][i];
        if (fabs(v) < eps) return false;
        for (j = 0; j < size; j++)
        {
            double c;
            if (i == j) continue;
            c = A[j][i] / v;
            for (k = 0; k < size; k++)
                A[j][k] -= A[i][k] * c;
            A[j][i] = 0;
            B[j] -= B[i] * c;
        }
    }
    /* solve */
    for (i = 0; i < size; i++)
    {
        if (fabs(A[i][i]) < eps) return false;
        S[i] = B[i] / A[i][i];
    }
    return true;
}

bool linsolved(const double *A, const double *B, double *solve, int size, double eps)
{
    int i, j;
    if (size <= 4)
    {
        double abuff[16];
        double bbuff[4];
        double *a[4];
        for (i = 0; i < size; i++)
        {
            a[i] = abuff + i * 4;
            for (j = 0; j < size; j++)
                a[i][j] = A[i * size + j];
            bbuff[i] = B[i];
        }
        return linsolved_base(a, bbuff, solve, size, eps);
    }
    else
    {
        char *p;
        double **a;
        double *b;
        i = (size * size + size /* a & b data */) * sizeof(double) + size /* a pointers */ * sizeof(double *);
        p = (char *)malloc(i);
        if (p == NULL) return false;
        a = (double **)p; p += size * sizeof(double *);
        b = (double *)p; p += size * sizeof(double);
        for (i = 0; i < size; i++)
        {
            a[i] = (double *)p; p += size * sizeof(double);
            for (int j = 0; j < size; j++)
                a[i][j] = A[i * size + j];
            b[i] = B[i];
        }
        i = linsolved_base(a, b, solve, size, eps);
        free(a);
        return i;
    }
}

#ifdef __cplusplus
}
#endif
