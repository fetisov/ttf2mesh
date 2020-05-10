#!/usr/bin/env python

import sys
import os

#----------------------------------------------

file = open("linalgf.h", "r")
s = file.read()
file.close()

s = s.replace("LINALGF_H", "LINALGD_H");

s = s.replace("float", "double");

s = s.replace("mat2f", "mat2d");
s = s.replace("mat3f", "mat3d");
s = s.replace("mat4f", "mat4d");
s = s.replace("vec2f", "vec2d");
s = s.replace("vec3f", "vec3d");
s = s.replace("vec4f", "vec4d");
s = s.replace("cpxf", "cpxd");

s = s.replace("v2f_", "v2d_");
s = s.replace("v3f_", "v3d_");
s = s.replace("v4f_", "v4d_");
s = s.replace("_v2f", "_v2d");
s = s.replace("_v3f", "_v3d");
s = s.replace("_v4f", "_v4d");
s = s.replace("m2f_", "m2d_");
s = s.replace("m3f_", "m3d_");
s = s.replace("m4f_", "m4d_");
s = s.replace("_m2f", "_m2d");
s = s.replace("_m3f", "_m3d");
s = s.replace("_m4f", "_m4d");

s = s.replace("Vec2f", "Vec2d");
s = s.replace("Vec3f", "Vec3d");
s = s.replace("Mat2f", "Mat2d");
s = s.replace("Mat3f", "Mat3d");
s = s.replace("Mat4f", "Mat4d");

s = s.replace("linalgf_", "linalgd_");
s = s.replace("linsolverf", "linsolverd");

file = open("linalgd.h", "w")
file.write(s)
file.close()

#----------------------------------------------

file = open("linalgf.c", "r")
s = file.read()
file.close()

s = s.replace("linalgf.h", "linalgd.h");

s = s.replace("float", "double");

s = s.replace("mat2f", "mat2d");
s = s.replace("mat3f", "mat3d");
s = s.replace("mat4f", "mat4d");
s = s.replace("vec2f", "vec2d");
s = s.replace("vec3f", "vec3d");
s = s.replace("vec4f", "vec4d");
s = s.replace("cpxf", "cpxd");

s = s.replace("v2f_", "v2d_");
s = s.replace("v3f_", "v3d_");
s = s.replace("v4f_", "v4d_");
s = s.replace("_v2f", "_v2d");
s = s.replace("_v3f", "_v3d");
s = s.replace("_v4f", "_v4d");
s = s.replace("m2f_", "m2d_");
s = s.replace("m3f_", "m3d_");
s = s.replace("m4f_", "m4d_");
s = s.replace("_m2f", "_m2d");
s = s.replace("_m3f", "_m3d");
s = s.replace("_m4f", "_m4d");

s = s.replace("linalgf_", "linalgd_");
s = s.replace("linsolverf", "linsolverd");
s = s.replace("find_leading_order_f", "find_leading_order_d");
s = s.replace("linear_solver_base_f", "linear_solver_base_d");
s = s.replace("det2f", "det2d");
s = s.replace("det3f", "det3d");

file = open("linalgd.c", "w")
file.write(s)
file.close()

#----------------------------------------------

file = open("linalgf.h", "r")
linalgfh = file.read()
file.close()

file = open("linalgd.h", "r")
linalgdh = file.read()
file.close()

file = open("linalgf.c", "r")
linalgfc = file.read()
file.close()

file = open("linalgd.c", "r")
linalgdc = file.read()
file.close()

linalgh = linalgfh + linalgdh

linalgc = linalgfc + linalgdc
linalgc = linalgc.replace("#include \"linalgf.h\"\n", "")
linalgc = linalgc.replace("#include \"linalgd.h\"\n", "")
linalgc = linalgc.replace("#include <assert.h>\n", "")
linalgc = "#include \"linalg.h\"\n#include <assert.h>\n" + linalgc

file = open("linalg.h", "w")
file.write(linalgh)
file.close()

file = open("linalg.c", "w")
file.write(linalgc)
file.close()
