![image](https://github.com/fetisov/ttf2mesh/blob/assets/logo.png?raw=true)

The ttf2mesh crossplatform library allows to convert a glyphs of truetype font (ttf) to a mesh objects in 2d and 3d space. The library does not require any graphical context and does not use system dependent functions.

The library consists of two files written in the C language of the **c99 standard**: ttf2mesh.c and ttf2mesh.h.
The ttf2mesh.c code implements a **parsing of ttf-files** and a **tessellation algorithm** that provides the formats conversion. The tessellation algorithm uses the [**Delaunay**](https://en.wikipedia.org/wiki/Delaunay_triangulation) test to form an optimal set of triangles. Tessellation (triangulation) process is described in ["this post"](https://habr.com/post/501268) and is shown in the animation below.

![image](https://github.com/fetisov/ttf2mesh/blob/assets/tessellation.gif?raw=true)

The library has a simple doxygen-documented API for loading TTF files and converting glyphs into mesh objects. Examples of using the library are presented in **examples/src** directory. There are three main examples:

|FILE                           |Description                  |
|-------------------------------|-----------------------------|
|examples/src/simple.c          |The simplest code that shows how a user can load a font from the system directory and convert its glyph to a 2d mesh object. The converted glyph is rendering to an opengl window as a filled mesh, wireframe or the glyph contours.|
||![image](https://raw.githubusercontent.com/fetisov/ttf2mesh/assets/2d.png)|
|examples/src/glyph3d.c         |Same as simple.c example, except that the font glyphs are converted to a 3D mesh object, which is displayed in the opengl window with animation.|
||![image](https://raw.githubusercontent.com/fetisov/ttf2mesh/assets/3d.png)|
|examples/src/ttf2obj.c         |Console application for converting TTF font input file to a Wavefront object file (.obj). Each object in the output file includes the plane geometry of the corresponding glyph and its parameters: Unicode ID, advance and bearing.|
||![image](https://raw.githubusercontent.com/fetisov/ttf2mesh/assets/objfile.png)|

To compile examples on Linux system you can use the GNU make utility: `make -C examples/build-linux-make all`. In the Windows operating system, you can use for compilation the Microsoft Visual Studio C++ project files that are located in the `examples/build-win-msvc` directory. Additionally, the `examples\build-any-qmake` directory contains pro files for building examples using the qtcreator IDE.

You can read information on how the library works at [this link](https://habr.com/post/501268).

[PayPal me](https://www.paypal.me/fetisovs) or:
*MasterCard* 5469 3800 5517 1176
*wmz* Z518568605100 *wmr* R885157851601
