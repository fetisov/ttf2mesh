#ifndef LINALGD_H
#define LINALGD_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef double mat2d[4];  /* matrix 2x2 type */
typedef double mat3d[9];  /* matrix 3x3 type */
typedef double mat4d[16]; /* matrix 4x4 type */
typedef double vec2d[2]; /* vector type with x,y components */
typedef double vec3d[3]; /* vector type with x,y,z components */
typedef double vec4d[4]; /* vector type with x,y,z,w components */
typedef vec2d cpxd; /* complex value */

extern const vec2d vec2d00; /* vector [0, 0] */
extern const vec2d vec2d10; /* vector [1, 0] */
extern const vec2d vec2d01; /* vector [0, 1] */
extern const vec3d vec3d000; /* vector [0, 0, 0] */
extern const vec3d vec3d100; /* vector [1, 0, 0] */
extern const vec3d vec3d010; /* vector [0, 1, 0] */
extern const vec3d vec3d001; /* vector [0, 0, 1] */
extern const mat2d mat2deye; /* identity matrix 2x2 */
extern const mat3d mat3deye; /* identity matrix 3x3 */
extern const mat4d mat4deye; /* identity matrix 4x4 */

#ifndef LINALG_MACRO
#   define LINALG_MACRO

#   define LINALG_INLINE static __inline
#   define LINALG_TRANSL
#   define MAT2(m, a0, a1, a2, a3) { \
        (m)[0] = a0; (m)[1] = a1; \
        (m)[2] = a2; (m)[3] = a3; }
#   define MAT3(m, a0, a1, a2, a3, a4, a5, a6, a7, a8) { \
        (m)[0] = a0; (m)[1] = a1; (m)[2] = a2; \
        (m)[3] = a3; (m)[4] = a4; (m)[5] = a5; \
        (m)[6] = a6; (m)[7] = a7; (m)[8] = a8; }
#   define MAT4(m, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15) { \
        (m)[0] = a0;   (m)[1] = a1;   (m)[2] = a2;   (m)[3] = a3;   \
        (m)[4] = a4;   (m)[5] = a5;   (m)[6] = a6;   (m)[7] = a7;   \
        (m)[8] = a8;   (m)[9] = a9;   (m)[10] = a10; (m)[11] = a11; \
        (m)[12] = a12; (m)[13] = a13; (m)[14] = a14; (m)[15] = a15; }
#   define LINALG_ASSIGN_ARRAY(dst, src, size) { \
        struct tmp { char data[size]; }; \
        *(struct tmp *)dst = *(struct tmp *)src; }
#endif

void linalgd_testbench(void);

LINALG_INLINE void m2d_eye(mat2d m); /* set identity matrix */
LINALG_INLINE void m3d_eye(mat3d m);
LINALG_INLINE void m4d_eye(mat4d m);
LINALG_INLINE void m2d_assign(mat2d dest, const mat2d src); /* copy src to dest matrix */
LINALG_INLINE void m3d_assign(mat3d dest, const mat3d src);
LINALG_INLINE void m4d_assign(mat4d dest, const mat4d src);
LINALG_INLINE void m3d_set(mat3d m, double a0, double a1, double a2,
                                    double a3, double a4, double a5,
                                    double a6, double a7, double a8); /* same as macro MAT3 */
LINALG_INLINE void m2d_add(mat2d res, const mat2d a, const mat2d b); /* res[2x2] = a[2x2] + b[2x2] */
LINALG_INLINE void m3d_add(mat3d res, const mat3d a, const mat3d b); /* res[3x3] = a[3x3] + b[3x3] */
LINALG_INLINE void m4d_add(mat4d res, const mat4d a, const mat4d b); /* res[4x4] = a[4x4] + b[4x4] */
LINALG_INLINE void m2d_sub(mat2d res, const mat2d a, const mat2d b); /* res[2x2] = a[2x2] - b[2x2] */
LINALG_INLINE void m3d_sub(mat3d res, const mat3d a, const mat3d b); /* res[3x3] = a[3x3] - b[3x3] */
LINALG_INLINE void m4d_sub(mat4d res, const mat4d a, const mat4d b); /* res[4x4] = a[4x4] - b[4x4] */
LINALG_INLINE void m2d_mul(mat2d res, const mat2d a, const mat2d b); /* res[2x2] = a[2x2] * b[2x2] */
LINALG_TRANSL void m3d_mul(mat3d res, const mat3d a, const mat3d b); /* res[3x3] = a[3x3] * b[3x3] */
LINALG_TRANSL void m4d_mul(mat4d res, const mat4d a, const mat4d b); /* res[4x4] = a[4x4] * b[4x4] */
LINALG_INLINE void m2d_mul_safe(mat2d res, const mat2d a, const mat2d b); /* res/a/b[2x2] = a[2x2] * b[2x2] */
LINALG_INLINE void m3d_mul_safe(mat3d res, const mat3d a, const mat3d b); /* res/a/b[3x3] = a[3x3] * b[3x3] */
LINALG_INLINE void m4d_mul_safe(mat4d res, const mat4d a, const mat4d b); /* res/a/b[4x4] = a[4x4] * b[4x4] */
LINALG_INLINE void m2d_t(mat2d res, const mat2d m); /* transposition */
LINALG_INLINE void m3d_t(mat3d res, const mat3d m);
LINALG_INLINE void m4d_t(mat4d res, const mat4d m);
LINALG_INLINE double m2d_det(const mat2d m); /* m[2x2] determinant */
LINALG_TRANSL double m3d_det(const mat3d m); /* m[3x3] determinant */
LINALG_TRANSL double m4d_det(const mat4d m); /* m[4x4] determinant */
LINALG_TRANSL bool m2d_inv(mat2d res, const mat2d m); /* res[2x2] = m[2x2]^(-1) */
LINALG_TRANSL bool m3d_inv(mat3d res, const mat3d m); /* res[3x3] = m[3x3]^(-1) */
LINALG_TRANSL bool m4d_inv(mat4d res, const mat4d m); /* res[4x4] = m[3x3]^(-1) */
LINALG_INLINE void m2d_rot(mat2d m, double angle); /* matrix of rotation to angle */
LINALG_INLINE void m3d_rotv(mat3d m, const vec3d v, double angle);
LINALG_INLINE void m3d_rotx(mat3d m, double angle); /* matrix of rotation to angle about x-axis */
LINALG_INLINE void m4d_rotx(mat4d m, double angle);
LINALG_INLINE void m3d_roty(mat3d m, double angle); /* matrix of rotation to angle about y-axis */
LINALG_INLINE void m4d_roty(mat4d m, double angle);
LINALG_INLINE void m3d_rotz(mat3d m, double angle); /* matrix of rotation to angle about z-axis */
LINALG_INLINE void m4d_rotz(mat4d m, double angle);
LINALG_INLINE void m4d_translate(mat4d m, double x, double y, double z);
LINALG_INLINE void rot_mul_m2d(mat3d m, double angle);
LINALG_INLINE void rotx_mul_m3d(mat3d m, double angle);
LINALG_INLINE void rotx_mul_m4d(mat4d m, double angle);
LINALG_INLINE void roty_mul_m3d(mat3d m, double angle);
LINALG_INLINE void roty_mul_m4d(mat4d m, double angle);
LINALG_INLINE void rotz_mul_m3d(mat3d m, double angle);
LINALG_INLINE void rotz_mul_m4d(mat4d m, double angle);
LINALG_INLINE void translate_mul_m4d(mat4d m, double x, double y, double z);
LINALG_INLINE void m2d_mul_v2d(vec2d res, const mat2d m, const vec2d v);
LINALG_INLINE double m3d_mul_v2d(vec2d res, const mat3d m, const vec2d v);
LINALG_INLINE void m3d_mul_v3d(vec3d res, const mat3d m, const vec3d v);
LINALG_INLINE void m4d_mul_v4d(vec4d res, const mat4d m, const vec4d v);
LINALG_INLINE double m4d_mul_v3d(vec3d res, const mat4d m, const vec3d v);
LINALG_INLINE void v2d_mul_m2d(vec2d res, const vec2d v, const mat2d m);
LINALG_INLINE void v3d_mul_m3d(vec3d res, const vec3d v, const mat3d m);
LINALG_INLINE void v4d_mul_m4d(vec4d res, const vec4d v, const mat4d m);
LINALG_INLINE double v3d_mul_m4d(vec3d res, const vec3d v, const mat4d m);
LINALG_INLINE void v2d_assign(vec2d dst, const vec2d src);
LINALG_INLINE void v3d_assign(vec3d dst, const vec3d src);
LINALG_INLINE void v4d_assign(vec4d dst, const vec4d src);
LINALG_INLINE void v2d_set(vec2d res, double x, double y);
LINALG_INLINE void v3d_set(vec3d res, double x, double y, double z);
LINALG_INLINE void v4d_set(vec4d res, double x, double y, double z, double w);
LINALG_INLINE void v2d_add(vec2d res, const vec2d a, const vec2d b);
LINALG_INLINE void v3d_add(vec3d res, const vec3d a, const vec3d b);
LINALG_INLINE void v4d_add(vec4d res, const vec4d a, const vec4d b);
LINALG_INLINE void v2d_sub(vec2d res, const vec2d a, const vec2d b);
LINALG_INLINE void v3d_sub(vec3d res, const vec3d a, const vec3d b);
LINALG_INLINE void v4d_sub(vec4d res, const vec4d a, const vec4d b);
LINALG_INLINE void v2d_scale(vec2d res, const vec2d v, double factor);
LINALG_INLINE void v3d_scale(vec3d res, const vec3d v, double factor);
LINALG_INLINE void v4d_scale(vec4d res, const vec4d v, double factor);
LINALG_INLINE double v2d_dot(const vec2d a, const vec2d b);
LINALG_INLINE double v3d_dot(const vec3d a, const vec3d b);
LINALG_INLINE double v4d_dot(const vec4d a, const vec4d b);
LINALG_INLINE void v3d_cross(vec3d res, const vec3d a, const vec3d b);
LINALG_INLINE double v2d_len(const vec2d v) { return sqrt(v2d_dot(v, v)); }
LINALG_INLINE double v3d_len(const vec3d v) { return sqrt(v3d_dot(v, v)); }
LINALG_INLINE double v4d_len(const vec4d v) { return sqrt(v4d_dot(v, v)); }
LINALG_INLINE void cpxd_set_re_im(cpxd res, double re, double im);
LINALG_INLINE void cpxd_set_abs_arg(cpxd res, double amp, double phase);
LINALG_INLINE void cpxd_add_cpxd(cpxd res, const cpxd a, const cpxd b);
LINALG_INLINE void cpxd_scale(cpxd a, double factor);
LINALG_INLINE void cpxd_mul_cpxd(cpxd res, const cpxd a, const cpxd b);
LINALG_INLINE double cpxd_abs(const cpxd v);
LINALG_INLINE double cpxd_arg(const cpxd v);
LINALG_TRANSL void m4d_ortho(mat4d res, double left, double right, double bottom, double top, double nearPlane, double farPlane);
LINALG_TRANSL void m4d_frustum(mat4d res, double left, double right, double bottom, double top, double nearPlane, double farPlane);
LINALG_TRANSL void m4d_perspective(mat4d res, double verticalAngle, double aspectRatio, double nearPlane, double farPlane);
LINALG_TRANSL void m4d_viewport(mat4d res, double left, double bottom, double width, double height, double nearPlane, double farPlane);
LINALG_TRANSL double v3d_project(vec3d res, const vec3d v, const mat4d modelView, const mat4d projection, int viewport_w, int viewport_h);
LINALG_TRANSL void v3d_unproject(vec3d res, const vec3d v, const mat4d modelView, const mat4d projection, int viewport_w, int viewport_h);
LINALG_TRANSL void v3d_unproject_fast(vec3d res, const vec3d v, const mat4d inv_modelview, const mat4d inv_projection, int viewport_w, int viewport_h);
LINALG_TRANSL bool v3d_lnp_intersection(vec3d res, const vec3d line_dir, const vec3d line_point, const vec4d plane, double *dist);
LINALG_TRANSL bool linsolved(const double *A, const double *B, double *solve, int size, double eps);

LINALG_INLINE void m2d_eye(mat2d m)
{
    LINALG_ASSIGN_ARRAY(m, mat2deye, sizeof(mat2d));
}

LINALG_INLINE void m3d_eye(mat3d m)
{
    LINALG_ASSIGN_ARRAY(m, mat3deye, sizeof(mat3d));
}

LINALG_INLINE void m4d_eye(mat4d m)
{
    LINALG_ASSIGN_ARRAY(m, mat4deye, sizeof(mat4d));
}

LINALG_INLINE void m2d_assign(mat2d dest, const mat2d src)
{
    LINALG_ASSIGN_ARRAY(dest, src, sizeof(mat2d));
}

LINALG_INLINE void m3d_assign(mat3d dest, const mat3d src)
{
    LINALG_ASSIGN_ARRAY(dest, src, sizeof(mat3d));
}

LINALG_INLINE void m4d_assign(mat4d dest, const mat4d src)
{
    LINALG_ASSIGN_ARRAY(dest, src, sizeof(mat4d));
}

LINALG_INLINE void m3d_set(mat3d m, double a0, double a1, double a2,
                                    double a3, double a4, double a5,
                                    double a6, double a7, double a8)
{
    m[0] = a0; m[1] = a1; m[2] = a2;
    m[3] = a3; m[4] = a4; m[5] = a5;
    m[6] = a6; m[7] = a7; m[8] = a8;
}

#define LINALG_MAT2_BINOP(res, a, op, b) { \
    res[0] = a[0] op b[0]; res[1] = a[1] op b[1]; \
    res[2] = a[2] op b[2]; res[3] = a[3] op b[3]; }

#define LINALG_MAT3_BINOP(res, a, op, b) { \
    res[0] = a[0] op b[0]; res[1] = a[1] op b[1]; res[2] = a[2] op b[2]; \
    res[3] = a[3] op b[3]; res[4] = a[4] op b[4]; res[5] = a[5] op b[5]; \
    res[6] = a[6] op b[6]; res[7] = a[7] op b[7]; res[8] = a[8] op b[8]; }

#define LINALG_MAT4_BINOP(res, a, op, b) { \
    res[0] = a[0] op b[0];    res[1] = a[1] op b[1];    res[2] = a[2] op b[2];    res[3] = a[3] op b[3]; \
    res[4] = a[4] op b[4];    res[5] = a[5] op b[5];    res[6] = a[6] op b[6];    res[7] = a[7] op b[7]; \
    res[8] = a[8] op b[8];    res[9] = a[9] op b[9];    res[10] = a[10] op b[10]; res[11] = a[11] op b[11]; \
    res[12] = a[12] op b[12]; res[13] = a[13] op b[13]; res[14] = a[14] op b[14]; res[15] = a[15] op b[15]; }

LINALG_INLINE void m2d_add(mat2d res, const mat2d a, const mat2d b)
{
    LINALG_MAT2_BINOP(res, a, +, b)
}

LINALG_INLINE void m3d_add(mat3d res, const mat3d a, const mat3d b)
{
    LINALG_MAT3_BINOP(res, a, +, b)
}

LINALG_INLINE void m4d_add(mat4d res, const mat4d a, const mat4d b)
{
    LINALG_MAT4_BINOP(res, a, +, b)
}

LINALG_INLINE void m2d_sub(mat2d res, const mat2d a, const mat2d b)
{
    LINALG_MAT2_BINOP(res, a, -, b)
}

LINALG_INLINE void m3d_sub(mat3d res, const mat3d a, const mat3d b)
{
    LINALG_MAT3_BINOP(res, a, -, b)
}

LINALG_INLINE void m4d_sub(mat4d res, const mat4d a, const mat4d b)
{
    LINALG_MAT4_BINOP(res, a, -, b)
}

#undef LINALG_MAT3_BINOP
#undef LINALG_MAT4_BINOP

LINALG_INLINE void m2d_mul(mat2d res, const mat2d a, const mat2d b)
{
    res[0] = a[0] * b[0] + a[1] * b[2];
    res[1] = a[0] * b[1] + a[1] * b[3];
    res[2] = a[2] * b[0] + a[3] * b[2];
    res[3] = a[2] * b[1] + a[3] * b[3];
}

LINALG_INLINE void m2d_mul_safe(mat2d res, const mat2d a, const mat2d b)
{
    mat2d r;
    m2d_mul(r, a, b);
    m2d_assign(res, r);
}

LINALG_INLINE void m3d_mul_safe(mat3d res, const mat3d a, const mat3d b)
{
    mat3d r;
    m3d_mul(r, a, b);
    m3d_assign(res, r);
}

LINALG_INLINE void m4d_mul_safe(mat4d res, const mat4d a, const mat4d b)
{
    mat4d r;
    m4d_mul(r, a, b);
    m4d_assign(res, r);
}

LINALG_INLINE void m2d_t(mat2d res, const mat2d m)
{
    double m1;
    m1 = m[1];
    res[0] = m[0];
    res[3] = m[3];
    res[1] = m[2];
    res[2] = m1;
}

LINALG_INLINE void m3d_t(mat3d res, const mat3d m)
{
    double a1, a2, a5;
    a1 = m[1];
    a2 = m[2];
    a5 = m[5];
    res[0] = m[0];
    res[4] = m[4];
    res[8] = m[8];
    res[1] = m[3];
    res[3] = a1;
    res[2] = m[6];
    res[6] = a2;
    res[5] = m[7];
    res[7] = a5;
}

LINALG_INLINE void m4d_t(mat4d res, const mat4d m)
{
#define a(r, c) res[r*4+c]
#define b(r, c) m[r*4+c]
    double b01, b02, b03, b12, b13, b23;
    b01 = b(0,1);
    b02 = b(0,2);
    b03 = b(0,3);
    b12 = b(1,2);
    b13 = b(1,3);
    b23 = b(2,3);

    a(0,0)=b(0,0);
    a(1,1)=b(1,1);
    a(2,2)=b(2,2);
    a(3,3)=b(3,3);

    a(0,1)=b(1,0);
    a(0,2)=b(2,0);
    a(0,3)=b(3,0);
    a(1,0)=b01;
    a(1,2)=b(2,1);
    a(1,3)=b(3,1);
    a(2,0)=b02;
    a(2,1)=b12;
    a(2,3)=b(3,2);
    a(3,0)=b03;
    a(3,1)=b13;
    a(3,2)=b23;
#undef a
#undef b
}

LINALG_INLINE double m2d_det(const mat2d m)
{
    return m[0] * m[3] - m[1] * m[2];
}

LINALG_INLINE void m2d_rot(mat2d m, double angle)
{
    double s, c;
    s = sin(angle);
    c = cos(angle);
    MAT2(m, c, -s,
            s,  c);
}

LINALG_INLINE void m3d_rotv(mat3d m, const vec3d v, double angle)
{
    double s, c, c1;
    s = sin(angle);
    c = cos(angle);
    c1 = 1.0 - c;
    MAT3(m,
        c+c1*v[0]*v[0],      c1*v[0]*v[1]-s*v[2], c1*v[0]*v[2]+s*v[1],
        c1*v[1]*v[0]+s*v[2], c+c1*v[1]*v[1],      c1*v[1]*v[2]-s*v[0],
        c1*v[2]*v[0]-s*v[1], c1*v[2]*v[1]+s*v[0], c+c1*v[2]*v[2]);
}

LINALG_INLINE void m3d_rotx(mat3d m, double angle)
{
    double s, c;
    s = sin(angle);
    c = cos(angle);
    MAT3(m, 1, 0, 0,
            0, c,-s,
            0, s, c);
}

LINALG_INLINE void m4d_rotx(mat4d m, double angle)
{
    double s, c;
    s = sin(angle);
    c = cos(angle);
    MAT4(m, 1, 0, 0, 0,
            0, c,-s, 0,
            0, s, c, 0,
            0, 0, 0, 1);
}

LINALG_INLINE void m3d_roty(mat3d m, double angle)
{
    double s, c;
    s = sin(angle);
    c = cos(angle);
    MAT3(m, c, 0, s,
            0, 1, 0,
           -s, 0, c);
}

LINALG_INLINE void m4d_roty(mat4d m, double angle)
{
    double s, c;
    s = sin(angle);
    c = cos(angle);
    MAT4(m, c, 0, s, 0,
            0, 1, 0, 0,
           -s, 0, c, 0,
            0, 0, 0, 1);
}

LINALG_INLINE void m3d_rotz(mat3d m, double angle)
{
    double s, c;
    s = sin(angle);
    c = cos(angle);
    MAT3(m, c,-s, 0,
            s, c, 0,
            0, 0, 1);
}

LINALG_INLINE void m4d_rotz(mat4d m, double angle)
{
    double s, c;
    s = sin(angle);
    c = cos(angle);
    MAT4(m, c,-s, 0, 0,
            s, c, 0, 0,
            0, 0, 1, 0,
            0, 0, 0, 1);
}

LINALG_INLINE void m4d_translate(mat4d m, double x, double y, double z)
{
    MAT4(m, 1, 0, 0, x,
            0, 1, 0, y,
            0, 0, 1, z,
            0, 0, 0, 1);
}

LINALG_INLINE void rot_mul_m2d(mat3d m, double angle)
{
    mat2d left, right;
    m2d_rot(left, angle);
    m2d_assign(right, m);
    m2d_mul(m, left, right);
}

LINALG_INLINE void rotx_mul_m3d(mat3d m, double angle)
{
    mat3d left, right;
    m3d_rotx(left, angle);
    m3d_assign(right, m);
    m3d_mul(m, left, right);
}

LINALG_INLINE void rotx_mul_m4d(mat4d m, double angle)
{
    mat4d left, right;
    m4d_rotx(left, angle);
    m4d_assign(right, m);
    m4d_mul(m, left, right);
}

LINALG_INLINE void roty_mul_m3d(mat3d m, double angle)
{
    mat3d left, right;
    m3d_roty(left, angle);
    m3d_assign(right, m);
    m3d_mul(m, left, right);
}

LINALG_INLINE void roty_mul_m4d(mat4d m, double angle)
{
    mat4d left, right;
    m4d_roty(left, angle);
    m4d_assign(right, m);
    m4d_mul(m, left, right);
}

LINALG_INLINE void rotz_mul_m3d(mat3d m, double angle)
{
    mat3d left, right;
    m3d_rotz(left, angle);
    m3d_assign(right, m);
    m3d_mul(m, left, right);
}

LINALG_INLINE void rotz_mul_m4d(mat4d m, double angle)
{
    mat4d left, right;
    m4d_rotz(left, angle);
    m4d_assign(right, m);
    m4d_mul(m, left, right);
}

LINALG_INLINE void translate_mul_m4d(mat4d m, double x, double y, double z)
{
    /*
    [1 0 0 x] [a b c X]   [a+xj, b+xk, c+xl, X+xW]
    [0 1 0 y] [d e f Y] = [d+yj, e+yk, f+yl, Y+yW]
    [0 0 1 z] [g h i Z]   [g+yj, h+yk, i+yl, Z+yW]
    [0 0 0 1] [j k l W]   [j,    k,    l,    W   ]
    */
    m[0*4+0] += x*m[3*4+0];
    m[0*4+1] += x*m[3*4+1];
    m[0*4+2] += x*m[3*4+2];
    m[0*4+3] += x*m[3*4+3];

    m[1*4+0] += y*m[3*4+0];
    m[1*4+1] += y*m[3*4+1];
    m[1*4+2] += y*m[3*4+2];
    m[1*4+3] += y*m[3*4+3];

    m[2*4+0] += z*m[3*4+0];
    m[2*4+1] += z*m[3*4+1];
    m[2*4+2] += z*m[3*4+2];
    m[2*4+3] += z*m[3*4+3];
}

LINALG_INLINE void m2d_mul_v2d(vec2d res, const mat2d m, const vec2d v)
{
    double x, y;
    x = m[0] * v[0] + m[1] * v[1];
    y = m[2] * v[0] + m[3] * v[1];
    res[0] = x;
    res[1] = y;
}

LINALG_INLINE double m3d_mul_v2d(vec2d res, const mat3d m, const vec2d v)
{
    double x, y;
    x = m[0] * v[0] + m[1] * v[1] + m[2];
    y = m[3] * v[0] + m[4] * v[1] + m[5];
    res[0] = x;
    res[1] = y;
    return m[6] * v[0] + m[7] * v[1] + m[8];
}

LINALG_INLINE void m3d_mul_v3d(vec3d res, const mat3d m, const vec3d v)
{
    double a1, a2, a3;
    a1 = v[0]*m[0] + v[1]*m[1] + v[2]*m[2];
    a2 = v[0]*m[3] + v[1]*m[4] + v[2]*m[5];
    a3 = v[0]*m[6] + v[1]*m[7] + v[2]*m[8];
    res[0] = a1;
    res[1] = a2;
    res[2] = a3;
}

LINALG_INLINE void m4d_mul_v4d(vec4d res, const mat4d m, const vec4d v)
{
    double a1, a2, a3, a4;
    a1 = v[0]*m[0]  + v[1]*m[1]  + v[2]*m[2]  + v[3]*m[3];
    a2 = v[0]*m[4]  + v[1]*m[5]  + v[2]*m[6]  + v[3]*m[7];
    a3 = v[0]*m[8]  + v[1]*m[9]  + v[2]*m[10] + v[3]*m[11];
    a4 = v[0]*m[12] + v[1]*m[13] + v[2]*m[14] + v[3]*m[15];
    res[0] = a1;
    res[1] = a2;
    res[2] = a3;
    res[3] = a4;
}

LINALG_INLINE double m4d_mul_v3d(vec3d res, const mat4d m, const vec3d v)
{
    double a1, a2, a3, a4;
    a1 = v[0]*m[0]  + v[1]*m[1]  + v[2]*m[2]  + m[3];
    a2 = v[0]*m[4]  + v[1]*m[5]  + v[2]*m[6]  + m[7];
    a3 = v[0]*m[8]  + v[1]*m[9]  + v[2]*m[10] + m[11];
    a4 = v[0]*m[12] + v[1]*m[13] + v[2]*m[14] + m[15];
    res[0] = a1;
    res[1] = a2;
    res[2] = a3;
    return a4;
}

LINALG_INLINE void v2d_mul_m2d(vec2d res, const vec2d v, const mat2d m)
{
    double x, y;
    x = m[0] * v[0] + m[2] * v[1];
    y = m[1] * v[0] + m[3] * v[2];
    res[0] = x;
    res[1] = y;
}

LINALG_INLINE void v3d_mul_m3d(vec3d res, const vec3d v, const mat3d m)
{
    double a1, a2, a3;
    a1 = v[0]*m[0] + v[1]*m[3] + v[2]*m[6];
    a2 = v[0]*m[1] + v[1]*m[4] + v[2]*m[7];
    a3 = v[0]*m[2] + v[1]*m[5] + v[2]*m[8];
    res[0] = a1;
    res[1] = a2;
    res[2] = a3;
}

LINALG_INLINE void v4d_mul_m4d(vec4d res, const vec4d v, const mat4d m)
{
    double a1, a2, a3, a4;
    a1 = v[0]*m[0] + v[1]*m[4] + v[2]*m[8]  + v[3]*m[12];
    a2 = v[0]*m[1] + v[1]*m[5] + v[2]*m[9]  + v[3]*m[13];
    a3 = v[0]*m[2] + v[1]*m[6] + v[2]*m[10] + v[3]*m[14];
    a4 = v[0]*m[3] + v[1]*m[7] + v[2]*m[11] + v[3]*m[15];
    res[0] = a1;
    res[1] = a2;
    res[2] = a3;
    res[3] = a4;
}

LINALG_INLINE double v3d_mul_m4d(vec3d res, const vec3d v, const mat4d m)
{
    double a1, a2, a3, a4;
    a1 = v[0]*m[0] + v[1]*m[4] + v[2]*m[8]  + m[12];
    a2 = v[0]*m[1] + v[1]*m[5] + v[2]*m[9]  + m[13];
    a3 = v[0]*m[2] + v[1]*m[6] + v[2]*m[10] + m[14];
    a4 = v[0]*m[3] + v[1]*m[7] + v[2]*m[11] + m[15];
    res[0] = a1;
    res[1] = a2;
    res[2] = a3;
    return a4;
}

LINALG_INLINE void v2d_assign(vec2d dst, const vec2d src)
{
    LINALG_ASSIGN_ARRAY(dst, src, sizeof(vec2d));
}

LINALG_INLINE void v3d_assign(vec3d dst, const vec3d src)
{
    LINALG_ASSIGN_ARRAY(dst, src, sizeof(vec3d));
}

LINALG_INLINE void v4d_assign(vec4d dst, const vec4d src)
{
    LINALG_ASSIGN_ARRAY(dst, src, sizeof(vec4d));
}

LINALG_INLINE void v2d_set(vec2d res, double x, double y)
{
    res[0] = x;
    res[1] = y;
}

LINALG_INLINE void v3d_set(vec3d res, double x, double y, double z)
{
    res[0] = x;
    res[1] = y;
    res[2] = z;
}

LINALG_INLINE void v4d_set(vec4d res, double x, double y, double z, double w)
{
    res[0] = x;
    res[1] = y;
    res[2] = z;
    res[3] = w;
}

LINALG_INLINE void v2d_add(vec2d res, const vec2d a, const vec2d b)
{
    res[0] = a[0] + b[0];
    res[1] = a[1] + b[1];
}

LINALG_INLINE void v3d_add(vec3d res, const vec3d a, const vec3d b)
{
    res[0] = a[0] + b[0];
    res[1] = a[1] + b[1];
    res[2] = a[2] + b[2];
}

LINALG_INLINE void v4d_add(vec4d res, const vec4d a, const vec4d b)
{
    res[0] = a[0] + b[0];
    res[1] = a[1] + b[1];
    res[2] = a[2] + b[2];
    res[3] = a[3] + b[3];
}

LINALG_INLINE void v2d_sub(vec2d res, const vec2d a, const vec2d b)
{
    res[0] = a[0] - b[0];
    res[1] = a[1] - b[1];
}

LINALG_INLINE void v3d_sub(vec3d res, const vec3d a, const vec3d b)
{
    res[0] = a[0] - b[0];
    res[1] = a[1] - b[1];
    res[2] = a[2] - b[2];
}

LINALG_INLINE void v4d_sub(vec4d res, const vec4d a, const vec4d b)
{
    res[0] = a[0] - b[0];
    res[1] = a[1] - b[1];
    res[2] = a[2] - b[2];
    res[3] = a[3] - b[3];
}

LINALG_INLINE void v2d_scale(vec2d res, const vec2d v, double factor)
{
    res[0] = v[0] * factor;
    res[1] = v[1] * factor;
}

LINALG_INLINE void v3d_scale(vec3d res, const vec3d v, double factor)
{
    res[0] = v[0] * factor;
    res[1] = v[1] * factor;
    res[2] = v[2] * factor;
}

LINALG_INLINE void v4d_scale(vec4d res, const vec4d v, double factor)
{
    res[0] = v[0] * factor;
    res[1] = v[1] * factor;
    res[2] = v[2] * factor;
    res[3] = v[3] * factor;
}

LINALG_INLINE double v2d_dot(const vec2d a, const vec2d b)
{
    return a[0] * b[0] + a[1] * b[1];
}

LINALG_INLINE double v3d_dot(const vec3d a, const vec3d b)
{
    return a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
}

LINALG_INLINE double v4d_dot(const vec4d a, const vec4d b)
{
    return a[0] * b[0] + a[1] * b[1] + a[2] * b[2] + a[3] * b[3];
}

LINALG_INLINE void v3d_cross(vec3d res, const vec3d a, const vec3d b)
{
    vec3d r;
    r[0] = a[1] * b[2] - a[2] * b[1];
    r[1] = a[2] * b[0] - a[0] * b[2];
    r[2] = a[0] * b[1] - a[1] * b[0];
    LINALG_ASSIGN_ARRAY(res, r, sizeof(vec3d));
}

LINALG_INLINE void cpxd_set_re_im(cpxd res, double re, double im)
{
    res[0] = re;
    res[1] = im;
}

LINALG_INLINE void cpxd_set_abs_arg(cpxd res, double amp, double phase)
{
    res[0] = amp * cos(phase);
    res[1] = amp * sin(phase);
}

LINALG_INLINE void cpxd_add_cpxd(cpxd res, const cpxd a, const cpxd b)
{
    res[0] = a[0] + b[0];
    res[1] = a[1] + b[1];
}

LINALG_INLINE void cpxd_scale(cpxd a, double factor)
{
    a[0] *= factor;
    a[1] *= factor;
}

LINALG_INLINE void cpxd_mul_cpxd(cpxd res, const cpxd a, const cpxd b)
{
    double re;
    re = a[0] * b[0] - a[1] * b[1];
    res[1] = a[0] * b[1] + a[1] * b[0];
    res[0] = re;
}

LINALG_INLINE double cpxd_abs(const cpxd v)
{
    return sqrt(v[0] * v[0] + v[1] * v[1]);
}

LINALG_INLINE double cpxd_arg(const cpxd v)
{
    return atan2(v[1], v[0]);
}

#ifdef __cplusplus
}

class Vec2d;
class Vec3d;
class Mat2d;
class Mat3d;
class Mat4d;

class Vec2d
{
public:
    inline Vec2d(double x = 0, double y = 0)
    {
        v[0] = x;
        v[1] = y;
    }
    inline Vec2d(const vec2d vec)
    {
        v[0] = vec[0];
        v[1] = vec[1];
    }
    inline Vec3d toVec3d(double third = 1.0) const;
    inline void passto(vec2d vec) const { v2d_assign(vec, v); }
    inline double &operator [](int index) { return v[index]; }
    inline const double &operator [](int index) const { return v[index]; }
    inline void operator +=(const Vec2d &src) { v2d_add(v, v, src.v); }
    inline void operator -=(const Vec2d &src) { v2d_sub(v, v, src.v); }
    inline double &x() { return v[0]; }
    inline double &y() { return v[1]; }
    inline const double &x() const { return v[0]; }
    inline const double &y() const { return v[1]; }
    inline double len() const { return v2d_len(v); }
    inline Vec2d scaled(double x, double y) const
    {
        return Vec2d(v[0] * x, v[1] * y);
    }
    inline Vec2d scaled(double factor) const
    {
        return Vec2d(v[0] * factor, v[1] * factor);
    }
    inline const Vec2d normalized() const
    {
        double c = 1.0 / v2d_len(v);
        return scaled(c, c);
    }
    static inline Vec2d i() { return Vec2d(1, 0); }
    static inline Vec2d j() { return Vec2d(0, 1); }
    static inline double dot(const Vec2d &a, const Vec2d &b)
    {
        return v2d_dot(a.v, b.v);
    }
    vec2d v;
};

class Vec3d
{
public:
    vec3d v;
    inline Vec3d(double x = 0, double y = 0, double z = 0)
    {
        v[0] = x;
        v[1] = y;
        v[2] = z;
    }
    inline Vec3d(const vec3d vec)
    {
        v[0] = vec[0];
        v[1] = vec[1];
        v[2] = vec[2];
    }
    inline Vec2d toVec2d() const;
    inline void passto(vec3d vec) const { v3d_assign(vec, v); }
    inline double &operator [](int index) { return v[index]; }
    inline const double &operator [](int index) const { return v[index]; }
    inline void operator +=(const Vec3d &src) { v3d_add(v, v, src.v); }
    inline void operator -=(const Vec3d &src) { v3d_sub(v, v, src.v); }
    inline double &x() { return v[0]; }
    inline double &y() { return v[1]; }
    inline double &z() { return v[2]; }
    inline const double &x() const { return v[0]; }
    inline const double &y() const { return v[1]; }
    inline const double &z() const { return v[2]; }
    inline double len() const { return v3d_len(v); }
    inline Vec3d scaled(double x, double y, double z) const
    {
        return Vec3d(v[0] * x, v[1] * y, v[2] * z);
    }
    inline Vec3d scaled(double factor) const
    {
        return Vec3d(v[0] * factor, v[1] * factor, v[2] * factor);
    }
    inline const Vec3d normalized() const
    {
        double c = 1.0 / v3d_len(v);
        return scaled(c, c, c);
    }
    inline Vec3d project(const Mat4d &modelView, const Mat4d &proj, int vpWidth, int vpHeight);
    inline Vec3d unproject(const Mat4d &modelView, const Mat4d &proj, int vpWidth, int vpHeight);
    static inline Vec3d i() { return Vec3d(1, 0, 0); }
    static inline Vec3d j() { return Vec3d(0, 1, 0); }
    static inline Vec3d k() { return Vec3d(0, 0, 1); }
    static inline Vec3d cross(const Vec3d &a, const Vec3d &b)
    {
        Vec3d res;
        v3d_cross(res.v, a.v, b.v);
        return res;
    }
    static inline double dot(const Vec3d &a, const Vec3d &b)
    {
        return v3d_dot(a.v, b.v);
    }
};

class Mat2d
{
public:
    inline Mat2d() {}

    inline Mat2d(double a11, double a12,
                 double a21, double a22)
    {
        m[0] = a11; m[1] = a12;
        m[2] = a21; m[3] = a22;
    }

    inline Mat2d(double e)
    {
        m[0] = e; m[1] = 0;
        m[2] = 0; m[3] = e;
    }

    inline Mat2d(const Vec2d &e1, const Vec2d &e2)
    {
        m[0] = e1[0]; m[1] = e1[1];
        m[2] = e2[0]; m[3] = e2[1];
    }

    inline Mat2d(const mat2d a)
    {
        m2d_assign(m, a);
    }

    inline Vec2d e1() const { return Vec2d(m + 0); }
    inline Vec2d e2() const { return Vec2d(m + 2); }
    inline Mat2d t() const { Mat2d res; m2d_t(res.m, m); return res; }

    inline Mat2d inv()
    {
        Mat2d res;
        m2d_inv(res.m, m);
        return res;
    }

    static inline Mat2d rot(double angle)
    {
        Mat2d m;
        m2d_rot(m.m, angle);
        return m;
    }

    static inline Mat2d scale(double x, double y)
    {
        return Mat2d(x, 0,
                     0, y);
    }

    mat2d m;
};

class Mat3d
{
public:
    inline Mat3d() {}

    inline Mat3d(double a11, double a12, double a13,
          double a21, double a22, double a23,
          double a31, double a32, double a33)
    {
        m[0] = a11; m[1] = a12; m[2] = a13;
        m[3] = a21; m[4] = a22; m[5] = a23;
        m[6] = a31; m[7] = a32; m[8] = a33;
    }

    inline Mat3d(double e)
    {
        m[0] = e; m[1] = 0; m[2] = 0;
        m[3] = 0; m[4] = e; m[5] = 0;
        m[6] = 0; m[7] = 0; m[8] = e;
    }

    inline Mat3d(const Vec3d &e1, const Vec3d &e2, const Vec3d &e3)
    {
        m[0] = e1[0]; m[1] = e1[1]; m[2] = e1[2];
        m[3] = e2[0]; m[4] = e2[1]; m[5] = e2[2];
        m[6] = e3[0]; m[7] = e3[1]; m[8] = e3[2];
    }

    inline Mat3d(const mat3d a)
    {
        m3d_assign(m, a);
    }

    inline Vec3d e1() const { return Vec3d(m + 0); }
    inline Vec3d e2() const { return Vec3d(m + 3); }
    inline Vec3d e3() const { return Vec3d(m + 6); }
    inline Mat3d t() const { Mat3d res; m3d_t(res.m, m); return res; }

    inline Mat3d inv()
    {
        Mat3d res;
        m3d_inv(res.m, m);
        return res;
    }

    static inline Mat3d rotv(const Vec3d &v, double angle)
    {
        Mat3d m;
        m3d_rotv(m.m, v.v, angle);
        return m;
    }

    static inline Mat3d rotx(double angle)
    {
        Mat3d m;
        m3d_rotx(m.m, angle);
        return m;
    }

    static inline Mat3d roty(double angle)
    {
        Mat3d m;
        m3d_roty(m.m, angle);
        return m;
    }

    static inline Mat3d rotz(double angle)
    {
        Mat3d m;
        m3d_rotz(m.m, angle);
        return m;
    }

    static inline Mat3d scale(double x, double y, double z)
    {
        return Mat3d(x, 0, 0,
                     0, y, 0,
                     0, 0, z);
    }

    static inline Mat3d translate(double x, double y, double w = 1)
    {
        return Mat3d(1, 0, x,
                     0, 1, y,
                     0, 0, w);
    }

    static inline Mat3d translate(const Vec2d &v, double w = 1)
    {
        return Mat3d(1, 0, v.v[0],
                     0, 0, v.v[1],
                     0, 0, w);
    }

    mat3d m;
};

class Mat4d
{
public:
    inline Mat4d() {}

    inline Mat4d(double a11, double a12, double a13, double a14,
          double a21, double a22, double a23, double a24,
          double a31, double a32, double a33, double a34,
          double a41 = 0, double a42 = 0, double a43 = 0, double a44 = 1)
    {
        MAT4(m, a11, a12, a13, a14,
                a21, a22, a23, a24,
                a31, a32, a33, a34,
                a41, a42, a43, a44);
    }

    inline Mat4d(double e, double w = 1)
    {
        MAT4(m, e, 0, 0, 0,
                0, e, 0, 0,
                0, 0, e, 0,
                0, 0, 0, w);
    }

    inline Mat4d(const Vec3d &e1, const Vec3d &e2, const Vec3d &e3, double w = 1)
    {
        MAT4(m, e1[0], e1[1], e1[2], 0,
                e2[0], e2[1], e2[2], 0,
                e3[0], e3[1], e3[2], 0,
                0,     0,     0,     w);
    }

    inline Mat4d(const mat4d a)
    {
        m4d_assign(m, a);
    }

    inline Vec3d e1() const { return Vec3d(m + 0); }
    inline Vec3d e2() const { return Vec3d(m + 4); }
    inline Vec3d e3() const { return Vec3d(m + 8); }
    inline Mat4d t() const { Mat4d res; m4d_t(res.m, m); return res; }

    inline Mat4d inv()
    {
        Mat4d res;
        m4d_inv(res.m, m);
        return res;
    }

    static inline Mat4d rotx(double angle)
    {
        Mat4d m;
        m4d_rotx(m.m, angle);
        return m;
    }

    static inline Mat4d roty(double angle)
    {
        Mat4d m;
        m4d_roty(m.m, angle);
        return m;
    }

    static inline Mat4d rotz(double angle)
    {
        Mat4d m;
        m4d_rotz(m.m, angle);
        return m;
    }

    static inline Mat4d scale(double x, double y, double z, double w = 1)
    {
        return Mat4d(x, 0, 0, 0,
                     0, y, 0, 0,
                     0, 0, z, 0,
                     0, 0, 0, w);
    }

    static inline Mat4d translate(double x, double y, double z, double w = 1)
    {
        return Mat4d(1, 0, 0, x,
                     0, 1, 0, y,
                     0, 0, 1, z,
                     0, 0, 0, w);
    }

    static inline Mat4d translate(const Vec3d &v, double w = 1)
    {
        return Mat4d(1, 0, 0, v.v[0],
                     0, 1, 0, v.v[1],
                     0, 0, 1, v.v[2],
                     0, 0, 0, w);
    }

    static inline Mat4d ortho(double left, double right, double bottom, double top, double nearPlane, double farPlane)
    {
        Mat4d res;
        m4d_ortho(res.m, left, right, bottom, top, nearPlane, farPlane);
        return res;
    }

    static inline Mat4d perspective(double verticalAngle, double aspectRatio, double nearPlane, double farPlane)
    {
        Mat4d res;
        m4d_perspective(res.m, verticalAngle, aspectRatio, nearPlane, farPlane);
        return res;
    }

    mat4d m;
};

/* Vec2 class implementation */

inline Vec3d Vec2d::toVec3d(double third) const
{
    return Vec3d(v[0], v[1], third);
}

inline Vec2d operator +(const Vec2d &a, const Vec2d &b)
{
    return Vec2d(a.v[0] + b.v[0], a.v[1] + b.v[1]);
}

inline Vec2d operator -(const Vec2d &a, const Vec2d &b)
{
    return Vec2d(a.v[0] - b.v[0], a.v[1] - b.v[1]);
}

inline Vec2d operator -(const Vec2d &a)
{
    return Vec2d(-a.v[0], -a.v[1]);
}

inline Vec2d operator *(const Vec2d &a, double s)
{
    return Vec2d(a.v[0] * s, a.v[1] * s);
}

inline Vec2d operator /(const Vec2d &a, double s)
{
    s = 1.0 / s;
    return Vec2d(a.v[0] * s, a.v[1] * s);
}

inline Vec2d operator *(double s, const Vec2d &a)
{
    return Vec2d(a.v[0] * s, a.v[1] * s);
}

inline Vec2d operator *(const Vec2d &a, const Mat2d &m)
{
    Vec2d res;
    v2d_mul_m2d(res.v, a.v, m.m);
    return res;
}

inline Vec2d operator *(const Mat2d &m, const Vec2d &a)
{
    Vec2d res;
    m2d_mul_v2d(res.v, m.m, a.v);
    return res;
}

inline Vec2d operator *(const Mat3d &m, const Vec2d &a)
{
    Vec2d res;
    m3d_mul_v2d(res.v, m.m, a.v);
    return res;
}

/* Vec3d class implementation */

inline Vec2d Vec3d::toVec2d() const
{
    return Vec2d(v[0], v[1]);
}

inline Vec3d Vec3d::project(const Mat4d &modelView, const Mat4d &proj, int vpWidth, int vpHeight)
{
    Vec3d res;
    v3d_project(res.v, v, modelView.m, proj.m, vpWidth, vpHeight);
    return res;
}

inline Vec3d Vec3d::unproject(const Mat4d &modelView, const Mat4d &proj, int vpWidth, int vpHeight)
{
    Vec3d res;
    v3d_unproject(res.v, v, modelView.m, proj.m, vpWidth, vpHeight);
    return res;
}

inline Vec3d operator +(const Vec3d &a, const Vec3d &b)
{
    return Vec3d(a.v[0] + b.v[0], a.v[1] + b.v[1], a.v[2] + b.v[2]);
}

inline Vec3d operator -(const Vec3d &a, const Vec3d &b)
{
    return Vec3d(a.v[0] - b.v[0], a.v[1] - b.v[1], a.v[2] - b.v[2]);
}

inline Vec3d operator -(const Vec3d &a)
{
    return Vec3d(-a.v[0], -a.v[1], -a.v[2]);
}

inline Vec3d operator *(const Vec3d &a, double s)
{
    return Vec3d(a.v[0] * s, a.v[1] * s, a.v[2] * s);
}

inline Vec3d operator /(const Vec3d &a, double s)
{
    s = 1.0 / s;
    return Vec3d(a.v[0] * s, a.v[1] * s, a.v[2] * s);
}

inline Vec3d operator *(double s, const Vec3d &a)
{
    return Vec3d(a.v[0] * s, a.v[1] * s, a.v[2] * s);
}

inline Vec3d operator *(const Vec3d &a, const Mat3d &m)
{
    Vec3d res;
    v3d_mul_m3d(res.v, a.v, m.m);
    return res;
}

inline Vec3d operator *(const Mat3d &m, const Vec3d &a)
{
    Vec3d res;
    m3d_mul_v3d(res.v, m.m, a.v);
    return res;
}

inline Vec3d operator *(const Vec3d &a, const Mat4d &m)
{
    Vec3d res;
    v3d_mul_m4d(res.v, a.v, m.m);
    return res;
}

inline Vec3d operator *(const Mat4d &m, const Vec3d &a)
{
    Vec3d res;
    m4d_mul_v3d(res.v, m.m, a.v);
    return res;
}

/* Mat2d class implementation */

inline Mat2d operator *(const Mat2d &a, const Mat2d &b)
{
    Mat2d res;
    m2d_mul(res.m, a.m, b.m);
    return res;
}

/* Mat3d class implementation */

inline Mat3d operator *(const Mat3d &a, const Mat3d &b)
{
    Mat3d res;
    m3d_mul(res.m, a.m, b.m);
    return res;
}

/* Mat4d class implementation */

inline Mat4d operator *(const Mat4d &a, const Mat4d &b)
{
    Mat4d res;
    m4d_mul(res.m, a.m, b.m);
    return res;
}

#endif

#endif /* LINALGD_H */
