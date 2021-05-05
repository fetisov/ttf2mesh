CONFIG = c99 utf8_source
QMAKE_CFLAGS = -std=c99

INCLUDEPATH += ../src/common ../..

SOURCES += ../../ttf2mesh.c ../src/common/glwindow.c
HEADERS += ../../ttf2mesh.h ../src/common/glwindow.h

linux* {
    LIBS = -lm -lX11 -lGL
}

win32* {
    LIBS += -lgdi32 -lopengl32 -lglu32
}
