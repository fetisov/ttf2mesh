#ifndef LINALGF_H
#define LINALGF_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef float mat2f[4];  /* matrix 2x2 type */
typedef float mat3f[9];  /* matrix 3x3 type */
typedef float mat4f[16]; /* matrix 4x4 type */
typedef float vec2f[2]; /* vector type with x,y components */
typedef float vec3f[3]; /* vector type with x,y,z components */
typedef float vec4f[4]; /* vector type with x,y,z,w components */
typedef vec2f cpxf; /* complex value */

extern const vec2f vec2f00; /* vector [0, 0] */
extern const vec2f vec2f10; /* vector [1, 0] */
extern const vec2f vec2f01; /* vector [0, 1] */
extern const vec3f vec3f000; /* vector [0, 0, 0] */
extern const vec3f vec3f100; /* vector [1, 0, 0] */
extern const vec3f vec3f010; /* vector [0, 1, 0] */
extern const vec3f vec3f001; /* vector [0, 0, 1] */
extern const mat2f mat2feye; /* identity matrix 2x2 */
extern const mat3f mat3feye; /* identity matrix 3x3 */
extern const mat4f mat4feye; /* identity matrix 4x4 */

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

void linalgf_testbench(void);

LINALG_INLINE void m2f_eye(mat2f m); /* set identity matrix */
LINALG_INLINE void m3f_eye(mat3f m);
LINALG_INLINE void m4f_eye(mat4f m);
LINALG_INLINE void m2f_assign(mat2f dest, const mat2f src); /* copy src to dest matrix */
LINALG_INLINE void m3f_assign(mat3f dest, const mat3f src);
LINALG_INLINE void m4f_assign(mat4f dest, const mat4f src);
LINALG_INLINE void m3f_set(mat3f m, float a0, float a1, float a2,
                                    float a3, float a4, float a5,
                                    float a6, float a7, float a8); /* same as macro MAT3 */
LINALG_INLINE void m2f_add(mat2f res, const mat2f a, const mat2f b); /* res[2x2] = a[2x2] + b[2x2] */
LINALG_INLINE void m3f_add(mat3f res, const mat3f a, const mat3f b); /* res[3x3] = a[3x3] + b[3x3] */
LINALG_INLINE void m4f_add(mat4f res, const mat4f a, const mat4f b); /* res[4x4] = a[4x4] + b[4x4] */
LINALG_INLINE void m2f_sub(mat2f res, const mat2f a, const mat2f b); /* res[2x2] = a[2x2] - b[2x2] */
LINALG_INLINE void m3f_sub(mat3f res, const mat3f a, const mat3f b); /* res[3x3] = a[3x3] - b[3x3] */
LINALG_INLINE void m4f_sub(mat4f res, const mat4f a, const mat4f b); /* res[4x4] = a[4x4] - b[4x4] */
LINALG_INLINE void m2f_mul(mat2f res, const mat2f a, const mat2f b); /* res[2x2] = a[2x2] * b[2x2] */
LINALG_TRANSL void m3f_mul(mat3f res, const mat3f a, const mat3f b); /* res[3x3] = a[3x3] * b[3x3] */
LINALG_TRANSL void m4f_mul(mat4f res, const mat4f a, const mat4f b); /* res[4x4] = a[4x4] * b[4x4] */
LINALG_INLINE void m2f_mul_safe(mat2f res, const mat2f a, const mat2f b); /* res/a/b[2x2] = a[2x2] * b[2x2] */
LINALG_INLINE void m3f_mul_safe(mat3f res, const mat3f a, const mat3f b); /* res/a/b[3x3] = a[3x3] * b[3x3] */
LINALG_INLINE void m4f_mul_safe(mat4f res, const mat4f a, const mat4f b); /* res/a/b[4x4] = a[4x4] * b[4x4] */
LINALG_INLINE void m2f_t(mat2f res, const mat2f m); /* transposition */
LINALG_INLINE void m3f_t(mat3f res, const mat3f m);
LINALG_INLINE void m4f_t(mat4f res, const mat4f m);
LINALG_INLINE float m2f_det(const mat2f m); /* m[2x2] determinant */
LINALG_TRANSL float m3f_det(const mat3f m); /* m[3x3] determinant */
LINALG_TRANSL float m4f_det(const mat4f m); /* m[4x4] determinant */
LINALG_TRANSL bool m2f_inv(mat2f res, const mat2f m); /* res[2x2] = m[2x2]^(-1) */
LINALG_TRANSL bool m3f_inv(mat3f res, const mat3f m); /* res[3x3] = m[3x3]^(-1) */
LINALG_TRANSL bool m4f_inv(mat4f res, const mat4f m); /* res[4x4] = m[3x3]^(-1) */
LINALG_INLINE void m2f_rot(mat2f m, float angle); /* matrix of rotation to angle */
LINALG_INLINE void m3f_rotv(mat3f m, const vec3f v, float angle);
LINALG_INLINE void m3f_rotx(mat3f m, float angle); /* matrix of rotation to angle about x-axis */
LINALG_INLINE void m4f_rotx(mat4f m, float angle);
LINALG_INLINE void m3f_roty(mat3f m, float angle); /* matrix of rotation to angle about y-axis */
LINALG_INLINE void m4f_roty(mat4f m, float angle);
LINALG_INLINE void m3f_rotz(mat3f m, float angle); /* matrix of rotation to angle about z-axis */
LINALG_INLINE void m4f_rotz(mat4f m, float angle);
LINALG_INLINE void m4f_translate(mat4f m, float x, float y, float z);
LINALG_INLINE void rot_mul_m2f(mat3f m, float angle);
LINALG_INLINE void rotx_mul_m3f(mat3f m, float angle);
LINALG_INLINE void rotx_mul_m4f(mat4f m, float angle);
LINALG_INLINE void roty_mul_m3f(mat3f m, float angle);
LINALG_INLINE void roty_mul_m4f(mat4f m, float angle);
LINALG_INLINE void rotz_mul_m3f(mat3f m, float angle);
LINALG_INLINE void rotz_mul_m4f(mat4f m, float angle);
LINALG_INLINE void translate_mul_m4f(mat4f m, float x, float y, float z);
LINALG_INLINE void m2f_mul_v2f(vec2f res, const mat2f m, const vec2f v);
LINALG_INLINE float m3f_mul_v2f(vec2f res, const mat3f m, const vec2f v);
LINALG_INLINE void m3f_mul_v3f(vec3f res, const mat3f m, const vec3f v);
LINALG_INLINE void m4f_mul_v4f(vec4f res, const mat4f m, const vec4f v);
LINALG_INLINE float m4f_mul_v3f(vec3f res, const mat4f m, const vec3f v);
LINALG_INLINE void v2f_mul_m2f(vec2f res, const vec2f v, const mat2f m);
LINALG_INLINE void v3f_mul_m3f(vec3f res, const vec3f v, const mat3f m);
LINALG_INLINE void v4f_mul_m4f(vec4f res, const vec4f v, const mat4f m);
LINALG_INLINE float v3f_mul_m4f(vec3f res, const vec3f v, const mat4f m);
LINALG_INLINE void v2f_assign(vec2f dst, const vec2f src);
LINALG_INLINE void v3f_assign(vec3f dst, const vec3f src);
LINALG_INLINE void v4f_assign(vec4f dst, const vec4f src);
LINALG_INLINE void v2f_set(vec2f res, float x, float y);
LINALG_INLINE void v3f_set(vec3f res, float x, float y, float z);
LINALG_INLINE void v4f_set(vec4f res, float x, float y, float z, float w);
LINALG_INLINE void v2f_add(vec2f res, const vec2f a, const vec2f b);
LINALG_INLINE void v3f_add(vec3f res, const vec3f a, const vec3f b);
LINALG_INLINE void v4f_add(vec4f res, const vec4f a, const vec4f b);
LINALG_INLINE void v2f_sub(vec2f res, const vec2f a, const vec2f b);
LINALG_INLINE void v3f_sub(vec3f res, const vec3f a, const vec3f b);
LINALG_INLINE void v4f_sub(vec4f res, const vec4f a, const vec4f b);
LINALG_INLINE void v2f_scale(vec2f res, const vec2f v, float factor);
LINALG_INLINE void v3f_scale(vec3f res, const vec3f v, float factor);
LINALG_INLINE void v4f_scale(vec4f res, const vec4f v, float factor);
LINALG_INLINE float v2f_dot(const vec2f a, const vec2f b);
LINALG_INLINE float v3f_dot(const vec3f a, const vec3f b);
LINALG_INLINE float v4f_dot(const vec4f a, const vec4f b);
LINALG_INLINE void v3f_cross(vec3f res, const vec3f a, const vec3f b);
LINALG_INLINE float v2f_len(const vec2f v) { return sqrt(v2f_dot(v, v)); }
LINALG_INLINE float v3f_len(const vec3f v) { return sqrt(v3f_dot(v, v)); }
LINALG_INLINE float v4f_len(const vec4f v) { return sqrt(v4f_dot(v, v)); }
LINALG_INLINE void cpxf_set_re_im(cpxf res, float re, float im);
LINALG_INLINE void cpxf_set_abs_arg(cpxf res, float amp, float phase);
LINALG_INLINE void cpxf_add_cpxf(cpxf res, const cpxf a, const cpxf b);
LINALG_INLINE void cpxf_scale(cpxf a, float factor);
LINALG_INLINE void cpxf_mul_cpxf(cpxf res, const cpxf a, const cpxf b);
LINALG_INLINE float cpxf_abs(const cpxf v);
LINALG_INLINE float cpxf_arg(const cpxf v);
LINALG_TRANSL void m4f_ortho(mat4f res, float left, float right, float bottom, float top, float nearPlane, float farPlane);
LINALG_TRANSL void m4f_frustum(mat4f res, float left, float right, float bottom, float top, float nearPlane, float farPlane);
LINALG_TRANSL void m4f_perspective(mat4f res, float verticalAngle, float aspectRatio, float nearPlane, float farPlane);
LINALG_TRANSL void m4f_viewport(mat4f res, float left, float bottom, float width, float height, float nearPlane, float farPlane);
LINALG_TRANSL float v3f_project(vec3f res, const vec3f v, const mat4f modelView, const mat4f projection, int viewport_w, int viewport_h);
LINALG_TRANSL void v3f_unproject(vec3f res, const vec3f v, const mat4f modelView, const mat4f projection, int viewport_w, int viewport_h);
LINALG_TRANSL void v3f_unproject_fast(vec3f res, const vec3f v, const mat4f inv_modelview, const mat4f inv_projection, int viewport_w, int viewport_h);
LINALG_TRANSL bool v3f_lnp_intersection(vec3f res, const vec3f line_dir, const vec3f line_point, const vec4f plane, float *dist);
LINALG_TRANSL bool linsolvef(const float *A, const float *B, float *solve, int size, float eps);

LINALG_INLINE void m2f_eye(mat2f m)
{
    LINALG_ASSIGN_ARRAY(m, mat2feye, sizeof(mat2f));
}

LINALG_INLINE void m3f_eye(mat3f m)
{
    LINALG_ASSIGN_ARRAY(m, mat3feye, sizeof(mat3f));
}

LINALG_INLINE void m4f_eye(mat4f m)
{
    LINALG_ASSIGN_ARRAY(m, mat4feye, sizeof(mat4f));
}

LINALG_INLINE void m2f_assign(mat2f dest, const mat2f src)
{
    LINALG_ASSIGN_ARRAY(dest, src, sizeof(mat2f));
}

LINALG_INLINE void m3f_assign(mat3f dest, const mat3f src)
{
    LINALG_ASSIGN_ARRAY(dest, src, sizeof(mat3f));
}

LINALG_INLINE void m4f_assign(mat4f dest, const mat4f src)
{
    LINALG_ASSIGN_ARRAY(dest, src, sizeof(mat4f));
}

LINALG_INLINE void m3f_set(mat3f m, float a0, float a1, float a2,
                                    float a3, float a4, float a5,
                                    float a6, float a7, float a8)
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

LINALG_INLINE void m2f_add(mat2f res, const mat2f a, const mat2f b)
{
    LINALG_MAT2_BINOP(res, a, +, b)
}

LINALG_INLINE void m3f_add(mat3f res, const mat3f a, const mat3f b)
{
    LINALG_MAT3_BINOP(res, a, +, b)
}

LINALG_INLINE void m4f_add(mat4f res, const mat4f a, const mat4f b)
{
    LINALG_MAT4_BINOP(res, a, +, b)
}

LINALG_INLINE void m2f_sub(mat2f res, const mat2f a, const mat2f b)
{
    LINALG_MAT2_BINOP(res, a, -, b)
}

LINALG_INLINE void m3f_sub(mat3f res, const mat3f a, const mat3f b)
{
    LINALG_MAT3_BINOP(res, a, -, b)
}

LINALG_INLINE void m4f_sub(mat4f res, const mat4f a, const mat4f b)
{
    LINALG_MAT4_BINOP(res, a, -, b)
}

#undef LINALG_MAT3_BINOP
#undef LINALG_MAT4_BINOP

LINALG_INLINE void m2f_mul(mat2f res, const mat2f a, const mat2f b)
{
    res[0] = a[0] * b[0] + a[1] * b[2];
    res[1] = a[0] * b[1] + a[1] * b[3];
    res[2] = a[2] * b[0] + a[3] * b[2];
    res[3] = a[2] * b[1] + a[3] * b[3];
}

LINALG_INLINE void m2f_mul_safe(mat2f res, const mat2f a, const mat2f b)
{
    mat2f r;
    m2f_mul(r, a, b);
    m2f_assign(res, r);
}

LINALG_INLINE void m3f_mul_safe(mat3f res, const mat3f a, const mat3f b)
{
    mat3f r;
    m3f_mul(r, a, b);
    m3f_assign(res, r);
}

LINALG_INLINE void m4f_mul_safe(mat4f res, const mat4f a, const mat4f b)
{
    mat4f r;
    m4f_mul(r, a, b);
    m4f_assign(res, r);
}

LINALG_INLINE void m2f_t(mat2f res, const mat2f m)
{
    float m1;
    m1 = m[1];
    res[0] = m[0];
    res[3] = m[3];
    res[1] = m[2];
    res[2] = m1;
}

LINALG_INLINE void m3f_t(mat3f res, const mat3f m)
{
    float a1, a2, a5;
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

LINALG_INLINE void m4f_t(mat4f res, const mat4f m)
{
#define a(r, c) res[r*4+c]
#define b(r, c) m[r*4+c]
    float b01, b02, b03, b12, b13, b23;
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

LINALG_INLINE float m2f_det(const mat2f m)
{
    return m[0] * m[3] - m[1] * m[2];
}

LINALG_INLINE void m2f_rot(mat2f m, float angle)
{
    float s, c;
    s = sin(angle);
    c = cos(angle);
    MAT2(m, c, -s,
            s,  c);
}

LINALG_INLINE void m3f_rotv(mat3f m, const vec3f v, float angle)
{
    float s, c, c1;
    s = sin(angle);
    c = cos(angle);
    c1 = 1.0 - c;
    MAT3(m,
        c+c1*v[0]*v[0],      c1*v[0]*v[1]-s*v[2], c1*v[0]*v[2]+s*v[1],
        c1*v[1]*v[0]+s*v[2], c+c1*v[1]*v[1],      c1*v[1]*v[2]-s*v[0],
        c1*v[2]*v[0]-s*v[1], c1*v[2]*v[1]+s*v[0], c+c1*v[2]*v[2]);
}

LINALG_INLINE void m3f_rotx(mat3f m, float angle)
{
    float s, c;
    s = sin(angle);
    c = cos(angle);
    MAT3(m, 1, 0, 0,
            0, c,-s,
            0, s, c);
}

LINALG_INLINE void m4f_rotx(mat4f m, float angle)
{
    float s, c;
    s = sin(angle);
    c = cos(angle);
    MAT4(m, 1, 0, 0, 0,
            0, c,-s, 0,
            0, s, c, 0,
            0, 0, 0, 1);
}

LINALG_INLINE void m3f_roty(mat3f m, float angle)
{
    float s, c;
    s = sin(angle);
    c = cos(angle);
    MAT3(m, c, 0, s,
            0, 1, 0,
           -s, 0, c);
}

LINALG_INLINE void m4f_roty(mat4f m, float angle)
{
    float s, c;
    s = sin(angle);
    c = cos(angle);
    MAT4(m, c, 0, s, 0,
            0, 1, 0, 0,
           -s, 0, c, 0,
            0, 0, 0, 1);
}

LINALG_INLINE void m3f_rotz(mat3f m, float angle)
{
    float s, c;
    s = sin(angle);
    c = cos(angle);
    MAT3(m, c,-s, 0,
            s, c, 0,
            0, 0, 1);
}

LINALG_INLINE void m4f_rotz(mat4f m, float angle)
{
    float s, c;
    s = sin(angle);
    c = cos(angle);
    MAT4(m, c,-s, 0, 0,
            s, c, 0, 0,
            0, 0, 1, 0,
            0, 0, 0, 1);
}

LINALG_INLINE void m4f_translate(mat4f m, float x, float y, float z)
{
    MAT4(m, 1, 0, 0, x,
            0, 1, 0, y,
            0, 0, 1, z,
            0, 0, 0, 1);
}

LINALG_INLINE void rot_mul_m2f(mat3f m, float angle)
{
    mat2f left, right;
    m2f_rot(left, angle);
    m2f_assign(right, m);
    m2f_mul(m, left, right);
}

LINALG_INLINE void rotx_mul_m3f(mat3f m, float angle)
{
    mat3f left, right;
    m3f_rotx(left, angle);
    m3f_assign(right, m);
    m3f_mul(m, left, right);
}

LINALG_INLINE void rotx_mul_m4f(mat4f m, float angle)
{
    mat4f left, right;
    m4f_rotx(left, angle);
    m4f_assign(right, m);
    m4f_mul(m, left, right);
}

LINALG_INLINE void roty_mul_m3f(mat3f m, float angle)
{
    mat3f left, right;
    m3f_roty(left, angle);
    m3f_assign(right, m);
    m3f_mul(m, left, right);
}

LINALG_INLINE void roty_mul_m4f(mat4f m, float angle)
{
    mat4f left, right;
    m4f_roty(left, angle);
    m4f_assign(right, m);
    m4f_mul(m, left, right);
}

LINALG_INLINE void rotz_mul_m3f(mat3f m, float angle)
{
    mat3f left, right;
    m3f_rotz(left, angle);
    m3f_assign(right, m);
    m3f_mul(m, left, right);
}

LINALG_INLINE void rotz_mul_m4f(mat4f m, float angle)
{
    mat4f left, right;
    m4f_rotz(left, angle);
    m4f_assign(right, m);
    m4f_mul(m, left, right);
}

LINALG_INLINE void translate_mul_m4f(mat4f m, float x, float y, float z)
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

LINALG_INLINE void m2f_mul_v2f(vec2f res, const mat2f m, const vec2f v)
{
    float x, y;
    x = m[0] * v[0] + m[1] * v[1];
    y = m[2] * v[0] + m[3] * v[1];
    res[0] = x;
    res[1] = y;
}

LINALG_INLINE float m3f_mul_v2f(vec2f res, const mat3f m, const vec2f v)
{
    float x, y;
    x = m[0] * v[0] + m[1] * v[1] + m[2];
    y = m[3] * v[0] + m[4] * v[1] + m[5];
    res[0] = x;
    res[1] = y;
    return m[6] * v[0] + m[7] * v[1] + m[8];
}

LINALG_INLINE void m3f_mul_v3f(vec3f res, const mat3f m, const vec3f v)
{
    float a1, a2, a3;
    a1 = v[0]*m[0] + v[1]*m[1] + v[2]*m[2];
    a2 = v[0]*m[3] + v[1]*m[4] + v[2]*m[5];
    a3 = v[0]*m[6] + v[1]*m[7] + v[2]*m[8];
    res[0] = a1;
    res[1] = a2;
    res[2] = a3;
}

LINALG_INLINE void m4f_mul_v4f(vec4f res, const mat4f m, const vec4f v)
{
    float a1, a2, a3, a4;
    a1 = v[0]*m[0]  + v[1]*m[1]  + v[2]*m[2]  + v[3]*m[3];
    a2 = v[0]*m[4]  + v[1]*m[5]  + v[2]*m[6]  + v[3]*m[7];
    a3 = v[0]*m[8]  + v[1]*m[9]  + v[2]*m[10] + v[3]*m[11];
    a4 = v[0]*m[12] + v[1]*m[13] + v[2]*m[14] + v[3]*m[15];
    res[0] = a1;
    res[1] = a2;
    res[2] = a3;
    res[3] = a4;
}

LINALG_INLINE float m4f_mul_v3f(vec3f res, const mat4f m, const vec3f v)
{
    float a1, a2, a3, a4;
    a1 = v[0]*m[0]  + v[1]*m[1]  + v[2]*m[2]  + m[3];
    a2 = v[0]*m[4]  + v[1]*m[5]  + v[2]*m[6]  + m[7];
    a3 = v[0]*m[8]  + v[1]*m[9]  + v[2]*m[10] + m[11];
    a4 = v[0]*m[12] + v[1]*m[13] + v[2]*m[14] + m[15];
    res[0] = a1;
    res[1] = a2;
    res[2] = a3;
    return a4;
}

LINALG_INLINE void v2f_mul_m2f(vec2f res, const vec2f v, const mat2f m)
{
    float x, y;
    x = m[0] * v[0] + m[2] * v[1];
    y = m[1] * v[0] + m[3] * v[2];
    res[0] = x;
    res[1] = y;
}

LINALG_INLINE void v3f_mul_m3f(vec3f res, const vec3f v, const mat3f m)
{
    float a1, a2, a3;
    a1 = v[0]*m[0] + v[1]*m[3] + v[2]*m[6];
    a2 = v[0]*m[1] + v[1]*m[4] + v[2]*m[7];
    a3 = v[0]*m[2] + v[1]*m[5] + v[2]*m[8];
    res[0] = a1;
    res[1] = a2;
    res[2] = a3;
}

LINALG_INLINE void v4f_mul_m4f(vec4f res, const vec4f v, const mat4f m)
{
    float a1, a2, a3, a4;
    a1 = v[0]*m[0] + v[1]*m[4] + v[2]*m[8]  + v[3]*m[12];
    a2 = v[0]*m[1] + v[1]*m[5] + v[2]*m[9]  + v[3]*m[13];
    a3 = v[0]*m[2] + v[1]*m[6] + v[2]*m[10] + v[3]*m[14];
    a4 = v[0]*m[3] + v[1]*m[7] + v[2]*m[11] + v[3]*m[15];
    res[0] = a1;
    res[1] = a2;
    res[2] = a3;
    res[3] = a4;
}

LINALG_INLINE float v3f_mul_m4f(vec3f res, const vec3f v, const mat4f m)
{
    float a1, a2, a3, a4;
    a1 = v[0]*m[0] + v[1]*m[4] + v[2]*m[8]  + m[12];
    a2 = v[0]*m[1] + v[1]*m[5] + v[2]*m[9]  + m[13];
    a3 = v[0]*m[2] + v[1]*m[6] + v[2]*m[10] + m[14];
    a4 = v[0]*m[3] + v[1]*m[7] + v[2]*m[11] + m[15];
    res[0] = a1;
    res[1] = a2;
    res[2] = a3;
    return a4;
}

LINALG_INLINE void v2f_assign(vec2f dst, const vec2f src)
{
    LINALG_ASSIGN_ARRAY(dst, src, sizeof(vec2f));
}

LINALG_INLINE void v3f_assign(vec3f dst, const vec3f src)
{
    LINALG_ASSIGN_ARRAY(dst, src, sizeof(vec3f));
}

LINALG_INLINE void v4f_assign(vec4f dst, const vec4f src)
{
    LINALG_ASSIGN_ARRAY(dst, src, sizeof(vec4f));
}

LINALG_INLINE void v2f_set(vec2f res, float x, float y)
{
    res[0] = x;
    res[1] = y;
}

LINALG_INLINE void v3f_set(vec3f res, float x, float y, float z)
{
    res[0] = x;
    res[1] = y;
    res[2] = z;
}

LINALG_INLINE void v4f_set(vec4f res, float x, float y, float z, float w)
{
    res[0] = x;
    res[1] = y;
    res[2] = z;
    res[3] = w;
}

LINALG_INLINE void v2f_add(vec2f res, const vec2f a, const vec2f b)
{
    res[0] = a[0] + b[0];
    res[1] = a[1] + b[1];
}

LINALG_INLINE void v3f_add(vec3f res, const vec3f a, const vec3f b)
{
    res[0] = a[0] + b[0];
    res[1] = a[1] + b[1];
    res[2] = a[2] + b[2];
}

LINALG_INLINE void v4f_add(vec4f res, const vec4f a, const vec4f b)
{
    res[0] = a[0] + b[0];
    res[1] = a[1] + b[1];
    res[2] = a[2] + b[2];
    res[3] = a[3] + b[3];
}

LINALG_INLINE void v2f_sub(vec2f res, const vec2f a, const vec2f b)
{
    res[0] = a[0] - b[0];
    res[1] = a[1] - b[1];
}

LINALG_INLINE void v3f_sub(vec3f res, const vec3f a, const vec3f b)
{
    res[0] = a[0] - b[0];
    res[1] = a[1] - b[1];
    res[2] = a[2] - b[2];
}

LINALG_INLINE void v4f_sub(vec4f res, const vec4f a, const vec4f b)
{
    res[0] = a[0] - b[0];
    res[1] = a[1] - b[1];
    res[2] = a[2] - b[2];
    res[3] = a[3] - b[3];
}

LINALG_INLINE void v2f_scale(vec2f res, const vec2f v, float factor)
{
    res[0] = v[0] * factor;
    res[1] = v[1] * factor;
}

LINALG_INLINE void v3f_scale(vec3f res, const vec3f v, float factor)
{
    res[0] = v[0] * factor;
    res[1] = v[1] * factor;
    res[2] = v[2] * factor;
}

LINALG_INLINE void v4f_scale(vec4f res, const vec4f v, float factor)
{
    res[0] = v[0] * factor;
    res[1] = v[1] * factor;
    res[2] = v[2] * factor;
    res[3] = v[3] * factor;
}

LINALG_INLINE float v2f_dot(const vec2f a, const vec2f b)
{
    return a[0] * b[0] + a[1] * b[1];
}

LINALG_INLINE float v3f_dot(const vec3f a, const vec3f b)
{
    return a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
}

LINALG_INLINE float v4f_dot(const vec4f a, const vec4f b)
{
    return a[0] * b[0] + a[1] * b[1] + a[2] * b[2] + a[3] * b[3];
}

LINALG_INLINE void v3f_cross(vec3f res, const vec3f a, const vec3f b)
{
    vec3f r;
    r[0] = a[1] * b[2] - a[2] * b[1];
    r[1] = a[2] * b[0] - a[0] * b[2];
    r[2] = a[0] * b[1] - a[1] * b[0];
    LINALG_ASSIGN_ARRAY(res, r, sizeof(vec3f));
}

LINALG_INLINE void cpxf_set_re_im(cpxf res, float re, float im)
{
    res[0] = re;
    res[1] = im;
}

LINALG_INLINE void cpxf_set_abs_arg(cpxf res, float amp, float phase)
{
    res[0] = amp * cos(phase);
    res[1] = amp * sin(phase);
}

LINALG_INLINE void cpxf_add_cpxf(cpxf res, const cpxf a, const cpxf b)
{
    res[0] = a[0] + b[0];
    res[1] = a[1] + b[1];
}

LINALG_INLINE void cpxf_scale(cpxf a, float factor)
{
    a[0] *= factor;
    a[1] *= factor;
}

LINALG_INLINE void cpxf_mul_cpxf(cpxf res, const cpxf a, const cpxf b)
{
    float re;
    re = a[0] * b[0] - a[1] * b[1];
    res[1] = a[0] * b[1] + a[1] * b[0];
    res[0] = re;
}

LINALG_INLINE float cpxf_abs(const cpxf v)
{
    return sqrt(v[0] * v[0] + v[1] * v[1]);
}

LINALG_INLINE float cpxf_arg(const cpxf v)
{
    return atan2(v[1], v[0]);
}

#ifdef __cplusplus
}

class Vec2f;
class Vec3f;
class Mat2f;
class Mat3f;
class Mat4f;

class Vec2f
{
public:
    inline Vec2f(float x = 0, float y = 0)
    {
        v[0] = x;
        v[1] = y;
    }
    inline Vec2f(const vec2f vec)
    {
        v[0] = vec[0];
        v[1] = vec[1];
    }
    inline Vec3f toVec3f(float third = 1.0) const;
    inline void passto(vec2f vec) const { v2f_assign(vec, v); }
    inline float &operator [](int index) { return v[index]; }
    inline const float &operator [](int index) const { return v[index]; }
    inline void operator +=(const Vec2f &src) { v2f_add(v, v, src.v); }
    inline void operator -=(const Vec2f &src) { v2f_sub(v, v, src.v); }
    inline float &x() { return v[0]; }
    inline float &y() { return v[1]; }
    inline const float &x() const { return v[0]; }
    inline const float &y() const { return v[1]; }
    inline float len() const { return v2f_len(v); }
    inline Vec2f scaled(float x, float y) const
    {
        return Vec2f(v[0] * x, v[1] * y);
    }
    inline Vec2f scaled(float factor) const
    {
        return Vec2f(v[0] * factor, v[1] * factor);
    }
    inline const Vec2f normalized() const
    {
        float c = 1.0 / v2f_len(v);
        return scaled(c, c);
    }
    static inline Vec2f i() { return Vec2f(1, 0); }
    static inline Vec2f j() { return Vec2f(0, 1); }
    static inline float dot(const Vec2f &a, const Vec2f &b)
    {
        return v2f_dot(a.v, b.v);
    }
    vec2f v;
};

class Vec3f
{
public:
    vec3f v;
    inline Vec3f(float x = 0, float y = 0, float z = 0)
    {
        v[0] = x;
        v[1] = y;
        v[2] = z;
    }
    inline Vec3f(const vec3f vec)
    {
        v[0] = vec[0];
        v[1] = vec[1];
        v[2] = vec[2];
    }
    inline Vec2f toVec2f() const;
    inline void passto(vec3f vec) const { v3f_assign(vec, v); }
    inline float &operator [](int index) { return v[index]; }
    inline const float &operator [](int index) const { return v[index]; }
    inline void operator +=(const Vec3f &src) { v3f_add(v, v, src.v); }
    inline void operator -=(const Vec3f &src) { v3f_sub(v, v, src.v); }
    inline float &x() { return v[0]; }
    inline float &y() { return v[1]; }
    inline float &z() { return v[2]; }
    inline const float &x() const { return v[0]; }
    inline const float &y() const { return v[1]; }
    inline const float &z() const { return v[2]; }
    inline float len() const { return v3f_len(v); }
    inline Vec3f scaled(float x, float y, float z) const
    {
        return Vec3f(v[0] * x, v[1] * y, v[2] * z);
    }
    inline Vec3f scaled(float factor) const
    {
        return Vec3f(v[0] * factor, v[1] * factor, v[2] * factor);
    }
    inline const Vec3f normalized() const
    {
        float c = 1.0 / v3f_len(v);
        return scaled(c, c, c);
    }
    inline Vec3f project(const Mat4f &modelView, const Mat4f &proj, int vpWidth, int vpHeight);
    inline Vec3f unproject(const Mat4f &modelView, const Mat4f &proj, int vpWidth, int vpHeight);
    static inline Vec3f i() { return Vec3f(1, 0, 0); }
    static inline Vec3f j() { return Vec3f(0, 1, 0); }
    static inline Vec3f k() { return Vec3f(0, 0, 1); }
    static inline Vec3f cross(const Vec3f &a, const Vec3f &b)
    {
        Vec3f res;
        v3f_cross(res.v, a.v, b.v);
        return res;
    }
    static inline float dot(const Vec3f &a, const Vec3f &b)
    {
        return v3f_dot(a.v, b.v);
    }
};

class Mat2f
{
public:
    inline Mat2f() {}

    inline Mat2f(float a11, float a12,
                 float a21, float a22)
    {
        m[0] = a11; m[1] = a12;
        m[2] = a21; m[3] = a22;
    }

    inline Mat2f(float e)
    {
        m[0] = e; m[1] = 0;
        m[2] = 0; m[3] = e;
    }

    inline Mat2f(const Vec2f &e1, const Vec2f &e2)
    {
        m[0] = e1[0]; m[1] = e1[1];
        m[2] = e2[0]; m[3] = e2[1];
    }

    inline Mat2f(const mat2f a)
    {
        m2f_assign(m, a);
    }

    inline Vec2f e1() const { return Vec2f(m + 0); }
    inline Vec2f e2() const { return Vec2f(m + 2); }
    inline Mat2f t() const { Mat2f res; m2f_t(res.m, m); return res; }

    inline Mat2f inv()
    {
        Mat2f res;
        m2f_inv(res.m, m);
        return res;
    }

    static inline Mat2f rot(float angle)
    {
        Mat2f m;
        m2f_rot(m.m, angle);
        return m;
    }

    static inline Mat2f scale(float x, float y)
    {
        return Mat2f(x, 0,
                     0, y);
    }

    mat2f m;
};

class Mat3f
{
public:
    inline Mat3f() {}

    inline Mat3f(float a11, float a12, float a13,
          float a21, float a22, float a23,
          float a31, float a32, float a33)
    {
        m[0] = a11; m[1] = a12; m[2] = a13;
        m[3] = a21; m[4] = a22; m[5] = a23;
        m[6] = a31; m[7] = a32; m[8] = a33;
    }

    inline Mat3f(float e)
    {
        m[0] = e; m[1] = 0; m[2] = 0;
        m[3] = 0; m[4] = e; m[5] = 0;
        m[6] = 0; m[7] = 0; m[8] = e;
    }

    inline Mat3f(const Vec3f &e1, const Vec3f &e2, const Vec3f &e3)
    {
        m[0] = e1[0]; m[1] = e1[1]; m[2] = e1[2];
        m[3] = e2[0]; m[4] = e2[1]; m[5] = e2[2];
        m[6] = e3[0]; m[7] = e3[1]; m[8] = e3[2];
    }

    inline Mat3f(const mat3f a)
    {
        m3f_assign(m, a);
    }

    inline Vec3f e1() const { return Vec3f(m + 0); }
    inline Vec3f e2() const { return Vec3f(m + 3); }
    inline Vec3f e3() const { return Vec3f(m + 6); }
    inline Mat3f t() const { Mat3f res; m3f_t(res.m, m); return res; }

    inline Mat3f inv()
    {
        Mat3f res;
        m3f_inv(res.m, m);
        return res;
    }

    static inline Mat3f rotv(const Vec3f &v, float angle)
    {
        Mat3f m;
        m3f_rotv(m.m, v.v, angle);
        return m;
    }

    static inline Mat3f rotx(float angle)
    {
        Mat3f m;
        m3f_rotx(m.m, angle);
        return m;
    }

    static inline Mat3f roty(float angle)
    {
        Mat3f m;
        m3f_roty(m.m, angle);
        return m;
    }

    static inline Mat3f rotz(float angle)
    {
        Mat3f m;
        m3f_rotz(m.m, angle);
        return m;
    }

    static inline Mat3f scale(float x, float y, float z)
    {
        return Mat3f(x, 0, 0,
                     0, y, 0,
                     0, 0, z);
    }

    static inline Mat3f translate(float x, float y, float w = 1)
    {
        return Mat3f(1, 0, x,
                     0, 1, y,
                     0, 0, w);
    }

    static inline Mat3f translate(const Vec2f &v, float w = 1)
    {
        return Mat3f(1, 0, v.v[0],
                     0, 0, v.v[1],
                     0, 0, w);
    }

    mat3f m;
};

class Mat4f
{
public:
    inline Mat4f() {}

    inline Mat4f(float a11, float a12, float a13, float a14,
          float a21, float a22, float a23, float a24,
          float a31, float a32, float a33, float a34,
          float a41 = 0, float a42 = 0, float a43 = 0, float a44 = 1)
    {
        MAT4(m, a11, a12, a13, a14,
                a21, a22, a23, a24,
                a31, a32, a33, a34,
                a41, a42, a43, a44);
    }

    inline Mat4f(float e, float w = 1)
    {
        MAT4(m, e, 0, 0, 0,
                0, e, 0, 0,
                0, 0, e, 0,
                0, 0, 0, w);
    }

    inline Mat4f(const Vec3f &e1, const Vec3f &e2, const Vec3f &e3, float w = 1)
    {
        MAT4(m, e1[0], e1[1], e1[2], 0,
                e2[0], e2[1], e2[2], 0,
                e3[0], e3[1], e3[2], 0,
                0,     0,     0,     w);
    }

    inline Mat4f(const mat4f a)
    {
        m4f_assign(m, a);
    }

    inline Vec3f e1() const { return Vec3f(m + 0); }
    inline Vec3f e2() const { return Vec3f(m + 4); }
    inline Vec3f e3() const { return Vec3f(m + 8); }
    inline Mat4f t() const { Mat4f res; m4f_t(res.m, m); return res; }

    inline Mat4f inv()
    {
        Mat4f res;
        m4f_inv(res.m, m);
        return res;
    }

    static inline Mat4f rotx(float angle)
    {
        Mat4f m;
        m4f_rotx(m.m, angle);
        return m;
    }

    static inline Mat4f roty(float angle)
    {
        Mat4f m;
        m4f_roty(m.m, angle);
        return m;
    }

    static inline Mat4f rotz(float angle)
    {
        Mat4f m;
        m4f_rotz(m.m, angle);
        return m;
    }

    static inline Mat4f scale(float x, float y, float z, float w = 1)
    {
        return Mat4f(x, 0, 0, 0,
                     0, y, 0, 0,
                     0, 0, z, 0,
                     0, 0, 0, w);
    }

    static inline Mat4f translate(float x, float y, float z, float w = 1)
    {
        return Mat4f(1, 0, 0, x,
                     0, 1, 0, y,
                     0, 0, 1, z,
                     0, 0, 0, w);
    }

    static inline Mat4f translate(const Vec3f &v, float w = 1)
    {
        return Mat4f(1, 0, 0, v.v[0],
                     0, 1, 0, v.v[1],
                     0, 0, 1, v.v[2],
                     0, 0, 0, w);
    }

    static inline Mat4f ortho(float left, float right, float bottom, float top, float nearPlane, float farPlane)
    {
        Mat4f res;
        m4f_ortho(res.m, left, right, bottom, top, nearPlane, farPlane);
        return res;
    }

    static inline Mat4f perspective(double verticalAngle, double aspectRatio, double nearPlane, double farPlane)
    {
        Mat4f res;
        m4f_perspective(res.m, verticalAngle, aspectRatio, nearPlane, farPlane);
        return res;
    }

    mat4f m;
};

/* Vec2 class implementation */

inline Vec3f Vec2f::toVec3f(float third) const
{
    return Vec3f(v[0], v[1], third);
}

inline Vec2f operator +(const Vec2f &a, const Vec2f &b)
{
    return Vec2f(a.v[0] + b.v[0], a.v[1] + b.v[1]);
}

inline Vec2f operator -(const Vec2f &a, const Vec2f &b)
{
    return Vec2f(a.v[0] - b.v[0], a.v[1] - b.v[1]);
}

inline Vec2f operator -(const Vec2f &a)
{
    return Vec2f(-a.v[0], -a.v[1]);
}

inline Vec2f operator *(const Vec2f &a, float s)
{
    return Vec2f(a.v[0] * s, a.v[1] * s);
}

inline Vec2f operator /(const Vec2f &a, float s)
{
    s = 1.0 / s;
    return Vec2f(a.v[0] * s, a.v[1] * s);
}

inline Vec2f operator *(float s, const Vec2f &a)
{
    return Vec2f(a.v[0] * s, a.v[1] * s);
}

inline Vec2f operator *(const Vec2f &a, const Mat2f &m)
{
    Vec2f res;
    v2f_mul_m2f(res.v, a.v, m.m);
    return res;
}

inline Vec2f operator *(const Mat2f &m, const Vec2f &a)
{
    Vec2f res;
    m2f_mul_v2f(res.v, m.m, a.v);
    return res;
}

inline Vec2f operator *(const Mat3f &m, const Vec2f &a)
{
    Vec2f res;
    m3f_mul_v2f(res.v, m.m, a.v);
    return res;
}

/* Vec3f class implementation */

inline Vec2f Vec3f::toVec2f() const
{
    return Vec2f(v[0], v[1]);
}

inline Vec3f Vec3f::project(const Mat4f &modelView, const Mat4f &proj, int vpWidth, int vpHeight)
{
    Vec3f res;
    v3f_project(res.v, v, modelView.m, proj.m, vpWidth, vpHeight);
    return res;
}

inline Vec3f Vec3f::unproject(const Mat4f &modelView, const Mat4f &proj, int vpWidth, int vpHeight)
{
    Vec3f res;
    v3f_unproject(res.v, v, modelView.m, proj.m, vpWidth, vpHeight);
    return res;
}

inline Vec3f operator +(const Vec3f &a, const Vec3f &b)
{
    return Vec3f(a.v[0] + b.v[0], a.v[1] + b.v[1], a.v[2] + b.v[2]);
}

inline Vec3f operator -(const Vec3f &a, const Vec3f &b)
{
    return Vec3f(a.v[0] - b.v[0], a.v[1] - b.v[1], a.v[2] - b.v[2]);
}

inline Vec3f operator -(const Vec3f &a)
{
    return Vec3f(-a.v[0], -a.v[1], -a.v[2]);
}

inline Vec3f operator *(const Vec3f &a, float s)
{
    return Vec3f(a.v[0] * s, a.v[1] * s, a.v[2] * s);
}

inline Vec3f operator /(const Vec3f &a, float s)
{
    s = 1.0 / s;
    return Vec3f(a.v[0] * s, a.v[1] * s, a.v[2] * s);
}

inline Vec3f operator *(float s, const Vec3f &a)
{
    return Vec3f(a.v[0] * s, a.v[1] * s, a.v[2] * s);
}

inline Vec3f operator *(const Vec3f &a, const Mat3f &m)
{
    Vec3f res;
    v3f_mul_m3f(res.v, a.v, m.m);
    return res;
}

inline Vec3f operator *(const Mat3f &m, const Vec3f &a)
{
    Vec3f res;
    m3f_mul_v3f(res.v, m.m, a.v);
    return res;
}

inline Vec3f operator *(const Vec3f &a, const Mat4f &m)
{
    Vec3f res;
    v3f_mul_m4f(res.v, a.v, m.m);
    return res;
}

inline Vec3f operator *(const Mat4f &m, const Vec3f &a)
{
    Vec3f res;
    m4f_mul_v3f(res.v, m.m, a.v);
    return res;
}

/* Mat2f class implementation */

inline Mat2f operator *(const Mat2f &a, const Mat2f &b)
{
    Mat2f res;
    m2f_mul(res.m, a.m, b.m);
    return res;
}

/* Mat3f class implementation */

inline Mat3f operator *(const Mat3f &a, const Mat3f &b)
{
    Mat3f res;
    m3f_mul(res.m, a.m, b.m);
    return res;
}

/* Mat4f class implementation */

inline Mat4f operator *(const Mat4f &a, const Mat4f &b)
{
    Mat4f res;
    m4f_mul(res.m, a.m, b.m);
    return res;
}

#endif

#endif /* LINALGF_H */
