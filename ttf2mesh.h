/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2020 by Sergey Fetisov <fsenok@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef TTF2MESH_H
#define TTF2MESH_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define TTF2MESH_VERSION   "1.0"  /* current library version */

#define TTF_MAX_FILE       32     /* font file size limit, MB */

/* return codes of ttf_xxx functions */

#define TTF_DONE           0      /* operation successful */
#define TTF_ERR_NOMEM      1      /* not enough memory (malloc failed) */
#define TTF_ERR_SIZE       2      /* file size > TTF_MAX_FILE */
#define TTF_ERR_OPEN       3      /* error opening file */
#define TTF_ERR_VER        4      /* unsupported file version */
#define TTF_ERR_FMT        5      /* invalid file structure */
#define TTF_ERR_NOTAB      6      /* no required tables in file */
#define TTF_ERR_CSUM       7      /* invalid file or table checksum */
#define TTF_ERR_UTAB       8      /* unsupported table format */
#define TTF_ERR_MESHER     9      /* unable to create mesh */
#define TTF_ERR_NO_OUTLINE 10     /* glyph has no outline */
#define TTF_ERR_WRITING    11     /* error writing file */

/* flags and values passing to some function */

#define TTF_QUALITY_LOW    10     /* default quality value for some functions */
#define TTF_QUALITY_NORMAL 20     /* default quality value for some functions */
#define TTF_QUALITY_HIGH   50     /* default quality value for some functions */

#define TTF_FEATURES_DFLT   0     /* default value of ttf_glyph2mesh features parameter */
#define TTF_FEATURE_IGN_ERR 1     /* flag of ttf_glyph2mesh to ignore uncritical mesh errors */

/* all library types */

typedef struct ttf_file    ttf_t;
typedef struct ttf_glyph   ttf_glyph_t;
typedef struct ttf_outline ttf_outline_t;
typedef struct ttf_point   ttf_point_t;
typedef struct ttf_mesh    ttf_mesh_t;

/**
 * @brief Loaded font structure
 *
 * This structure contains information about the loaded
 * font, including the character set and their glyphs.
 */
struct ttf_file
{
    int nchars;                   /* number of the font characters */
    int nglyphs;                  /* number of glyphs (usually less than nchars) */
    uint16_t *chars;              /* utf16 codes array with nchars length */
    uint16_t *char2glyph;         /* glyph indeces array with nchars length */
    ttf_glyph_t *glyphs;          /* array of the font glyphs with nglyphs length */
    const char *filename;         /* full path and file name of the font */
    uint32_t glyf_csum;           /* 'glyf' table checksum (used by ttf_list_fonts) */
    struct
    {
        /* fields of head table */
        /* https://docs.microsoft.com/ru-ru/typography/opentype/spec/head */
        float rev;                /* Font revision, set by manufacturer */
        struct
        {
            uint8_t bold: 1;      /* this font is a bold */
            uint8_t italic: 1;    /* this font is a italic */
            uint8_t underline: 1; /* this font is a underline */
            uint8_t outline: 1;   /* Outline */
            uint8_t shadow: 1;    /* Shadow */
            uint8_t condensed: 1; /* Condensed */
            uint8_t extended: 1;  /* Extended */
        } macStyle;
        /* fields of name table */
        /* https://docs.microsoft.com/ru-ru/typography/opentype/spec/name */
        const char *copyright;    /* Copyright notice */
        const char *family;       /* Font Family name */
        const char *subfamily;    /* Font Subfamily name */
        const char *unique_id;    /* Unique font identifier */
        const char *full_name;    /* Full font name */
        const char *version;      /* Version string */
        const char *ps_name;      /* PostScript name for the font */
        const char *trademark;    /* Trademark */
        const char *manufacturer; /* Manufacturer Name */
        const char *designer;     /* Designer */
        const char *description;  /* Description */
        const char *url_vendor;   /* URL Vendor */
        const char *url_designer; /* URL Designer */
        const char *license_desc; /* License Description */
        const char *locense_url;  /* License Info URL */
        const char *sample_text;  /* Sample text */
    } info;
};

/**
 * @brief The glyph struct
 */
struct ttf_glyph
{
    int index;                    /* glyph index in font */
    int symbol;                   /* utf-16 symbol */
    int npoints;                  /* total points within all contours */
    int ncontours;                /* number of contours in outline */
    float xbounds[2];             /* min/max values ​​along the x coordinate */
    float ybounds[2];             /* min/max values ​​along the y coordinate */
    float advance;                /* advance width */
    float lbearing;               /* left side bearing */
    float rbearing;               /* right side bearing = aw - (lsb + xMax - xMin) */
    ttf_outline_t *outline;       /* original outline of the glyph or NULL */
};

/**
 * @brief The outline struct
 */
struct ttf_outline
{
    int total_points;             /* total points within all contours */
    int ncontours;                /* number of contours in outline */
    struct
    {
        int length;               /* number of contour points */
        int subglyph_id;          /* subglyph index for composite glyphs */
        int subglyph_order;       /* subglyph reading order */
        ttf_point_t *pt;          /* contour points */
    } cont[1];
};

/**
 * @brief The outline point struct
 */
struct ttf_point
{
    float x;                      /* point x coordinate in EM */
    float y;                      /* point y coordinate in EM */
    uint32_t spl : 1;             /* point of spliting process */
    uint32_t onc : 1;             /* point on curve */
    uint32_t res: 30;             /* reserved for internal use */
};

/**
 * @brief The mesh struct
 */
struct ttf_mesh
{
    int nvert;                    /* length of vert array */
    int nfaces;                   /* length of faces array */
    struct
    {
        float x;
        float y;
    } *vert;                      /* vertices */
    struct
    {
        int v1;                   /* index of vertex #1 of triangle */
        int v2;                   /* index of vertex #2 of triangle */
        int v3;                   /* index of vertex #3 of triangle */
    } *faces;                     /* triangles */
    ttf_outline_t *outline;       /* see ttf_linear_outline() */
};

/**
 * @brief Load a font from memory
 * @param data Data pointer
 * @param size Data size
 * @param output Pointer to font object or NULL if error was occurred
 * @return Operation result TTF_XXX
 */
int ttf_load_from_mem(const uint8_t *data, int size, ttf_t **output, bool headers_only);

/**
 * @brief Load a font from file
 * @param filename TTF font file name
 * @param output Pointer to font object or NULL if error was occurred
 * @return Operation result TTF_XXX
 */
int ttf_load_from_file(const char *filename, ttf_t **output, bool headers_only);

/**
 * @brief List available fonts in directory
 * @param directories Array of standard C strings
 * @param dir_count Length of \a directories array
 * @return Array of references to ttf_t objects or NULL if no memory
 *
 * Font list is terminated by NULL reference.
 * Any font in list has no loaded glyphs but values of other fields are presented.
 * After using the list, you need to free it with ttf_free_list function.
 */
ttf_t **ttf_list_fonts(const char **directories, int dir_count);

/**
 * @brief List available system fonts
 * @return Array of references to ttf_t objects or NULL if no memory
 *
 * Font list is terminated by NULL reference.
 * Any font in list has no loaded glyphs but values of other fields are presented.
 * After using the list, you need to free it with ttf_free_list function.
 */
ttf_t **ttf_list_system_fonts(void);

/**
 * @brief Translate unicode character to glyph index in font object
 * @param ttf Pointer to font object
 * @param utf16_char Unicode character
 * @return Glyph index in glyphs array or -1
 */
int ttf_find_glyph(const ttf_t *ttf, uint16_t utf16_char);

/**
 * @brief Convert continuous qbezier curves to their three-point variant
 * @param glyph Pointer to glyph object
 * @return Pointer to converted outline or NULL if no memory in system
 */
ttf_outline_t *ttf_splitted_outline(const ttf_glyph_t *glyph);

/**
 * @brief Convert glyph outline to sequence of line objects
 * @param glyph Pointer to glyph object
 * @param quality Number of points to circle (see TTF_QUALITY_XXX)
 * @return Pointer to converted outline or NULL if no memory in system or if glyph have no outline
 */
ttf_outline_t *ttf_linear_outline(const ttf_glyph_t *glyph, uint8_t quality);

/**
 * @brief Base implementation of Even-odd algorithm
 * @param outline Outline which prepared by ttf_linear_outline function
 * @param point Point coordinates
 * @param contour Contour index
 * @param dist Distance at left of the point to a most closer point of contour, can be NULL if does not matter
 * @return The number of intersections with the contour to the left of the point
 */
int ttf_outline_evenodd_base(const ttf_outline_t *outline, const float point[2], int contour, float *dist);

/**
 * @brief Even-odd algorithm for point
 * @param outline Outline which prepared by ttf_linear_outline function
 * @param point Point coordinates
 * @param subglyph Subglyph of interest or -1 if does not matter
 * @return true if point is on filled glyph region
 */
bool ttf_outline_evenodd(const ttf_outline_t *outline, const float point[2], int subglyph);

/**
 * @brief Even-odd algorithm for retriving the contour information
 * @param outline Outline which prepared by ttf_linear_outline function
 * @param subglyph Subglyph of interest or -1 if does not matter
 * @param contour Contour index in outline (0...ncontours-1)
 * @param test_point Point index in it \a contour
 * @param nested_to The index of parent contour if not NULL
 * @return true if contour is not a hole
 */
bool ttf_outline_contour_info(const ttf_outline_t *outline, int subglyph, int contour, int test_point, int *nested_to);

/**
 * @brief Convert glyph to <path/> object of svg document
 * @param glyph Pointer to glyph object
 * @return Null terminated string or NULL if no memory in system
 */
char *ttf_glyph2svgpath(ttf_glyph_t *glyph);

/**
 * @brief Convert glyph to mesh-object
 * @param glyph Pointer to glyph object
 * @param output Pointer to mesh object or NULL if error occurred
 * @param quality Number of points to circle (see TTF_QUALITY_XXX)
 * @param features Process features and tricks (see TTF_FEATURES_DFLT, TTF_FEATURE_XXX)
 * @return Operation result TTF_XXX
 */
int ttf_glyph2mesh(ttf_glyph_t *glyph, ttf_mesh_t **output, uint8_t quality, int features);

/**
 * @brief Export ttf font to Wavefront .obj file
 * @param ttf Pointer to font object
 * @param file_name File name of output file
 * @param quality Number of points to circle (see TTF_QUALITY_XXX)
 * @return Operation result TTF_DONE, TTF_ERR_OPEN or TTF_ERR_WRITING
 */
int ttf_export_to_obj(ttf_t *ttf, const char *file_name, uint8_t quality);

/**
 * @brief Free the outline object
 * @param outline Pointer to outline object
 *
 * It is necessary to call after working with outline object which was
 * allocated by ttf_splitted_outline or ttf_linear_outline function
 */
void ttf_free_outline(ttf_outline_t *outline);

/**
 * @brief Free the mesh object
 * @param mesh Pointer to mesh object
 *
 * It is necessary to call after working with mesh object
 * which was allocated by ttf_glyph2mesh function
 */
void ttf_free_mesh(ttf_mesh_t *mesh);

/**
 * @brief Free the font list created with ttf_list_fonts()
 * @param list font list
 */
void ttf_free_list(ttf_t **list);

/**
 * @brief Free the font object
 * @param ttf Pointer to font object
 *
 * It is necessary to call after working with font object which was
 * allocated by ttf_load_from_mem or ttf_load_from_file function
 */
void ttf_free(ttf_t *ttf);

#ifdef __cplusplus
}
#endif

#endif
