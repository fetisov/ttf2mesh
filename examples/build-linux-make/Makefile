TARGETS := simple glyph3d ttf2obj

CFLAGS  := -ggdb -Wall -Wextra -pedantic -std=c99 -D_POSIX_C_SOURCE=199309L
LDFLAGS := -lm -lX11 -lGL
IFLAGS  := -I ../src -I ../src/common -I ../..

COMMON_SRC := ../src/common/glwindow.c ../src/common/glwindow.h ../../ttf2mesh.c ../../ttf2mesh.h

simple: ../src/simple.c $(COMMON_SRC)
	gcc $(IFLAGS) $(CFLAGS) $(filter %.c,$^) -o $@ $(LDFLAGS)

glyph3d: ../src/glyph3d.c $(COMMON_SRC)
	gcc $(IFLAGS) $(CFLAGS) $(filter %.c,$^) -o $@ $(LDFLAGS)

ttf2obj: ../src/ttf2obj.c ../../ttf2mesh.c ../../ttf2mesh.h
	gcc $(IFLAGS) $(CFLAGS) $(filter %.c,$^) -o $@ $(LDFLAGS)

all: $(TARGETS)

clean:
	rm -rf $(TARGETS)
