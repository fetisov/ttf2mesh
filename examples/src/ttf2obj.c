#define _CRT_SECURE_NO_WARNINGS

#include "ttf2mesh.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *ttf_error_str[] =
{
    "",
    "not enough memory (malloc failed)",
    "file size > TTF_MAX_FILE",
    "error opening file",
    "unsupported file version",
    "invalid file structure",
    "no required tables in file",
    "invalid file or table checksum",
    "unsupported table format",
    "unable to create mesh",
    "glyph has no outline",
    "error writing file"
};

static void usage(int ret)
{
    printf("usage: ttf2obj <font-file.ttf> <output-file.obj> [quality]\n");
    fflush(stdout);
    exit(ret);
}

static void print_tail_of_file(const char *file_name)
{
    static char buff[512 + 1];
    FILE *f = fopen(file_name, "rb");
    if (f == NULL) return;
    fseek(f, -512, SEEK_END);
    int readed = fread(buff, 1, 512, f);
    buff[readed] = 0;
    const char *str = strstr(buff, "# export finished");
    if (str != NULL)
    {
        str += 17;
        while (*str == '\r' || *str == '\n') str++;
        printf("Completed. Read the tail of the output file:\n%s", str);
        fflush(stdout);
    }
    fclose(f);
}

int main(int argc, const char **argv)
{
    if (argc != 3 && argc != 4) usage(1);

    int quality = TTF_QUALITY_NORMAL;
    if (argc == 4)
    {
        char *endptr;
        quality = strtoul(argv[3], &endptr, 10);
        if (*endptr != 0) usage(1);
    }

    printf("Loading font \"%s\"...\n", argv[1]);
    fflush(stdout);

    ttf_t *font;
    int error = ttf_load_from_file(argv[1], &font, false);
    if (error != TTF_DONE)
    {
        fprintf(stderr, "Unable to load font: %s\n", ttf_error_str[error]);
        fflush(stderr);
        return 1;
    }

    printf("Export to wavefront \"%s\" with quality=%i...\n", argv[2], quality);
    fflush(stdout);

    error = ttf_export_to_obj(font, argv[2], quality);
    if (error != TTF_DONE)
    {
        fprintf(stderr, "Unable to export font: %s\n", ttf_error_str[error]);
        fflush(stderr);
        ttf_free(font);
        return 1;
    }

    ttf_free(font);

    print_tail_of_file(argv[2]);
    return 0;
}
