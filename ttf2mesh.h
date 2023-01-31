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

/*

    Release 1.5 (January 31, 2023)
        New Features and Improvements:
            - added support for supplementary-plane characters (U+10000 to U+10FFFF)
              Thanks to Fadis for this improvement
            - added userdata[] arrays to ttf_t and ttf_glyph_t structures
        Non-Backwards Compatible Changes:
            -
        Bug fixes:
            -

    Release 1.4 (August 8, 2021)
        New Features and Improvements:
            -
        Non-Backwards Compatible Changes:
            -
        Bug fixes:
            - Additional checking of read values in the parser
              Thanks to Mika blaind for the improvements

    Release 1.3 (May 5, 2021)
        New Features and Improvements:
            - ttf_glyph2mesh3d and ttf_free_mesh3d functions added for working with 3d-mesh
        Non-Backwards Compatible Changes:
            -
        Bug fixes:
            -

    Release 1.2 (April 4, 2021)
        New Features and Improvements:
            - ttf_list_fonts and ttf_list_system_fonts now accept the specified filename mask
            - ttf_glyph2svgpath now accept the scaling parameters
            - ttf_list_match_id function was added
        Non-Backwards Compatible Changes:
            - in the ttf_list_fonts and ttf_list_system_fonts mask parameter was added,
              it can be set to NULL for backwards compatible
        Bug fixes:
            -

    Release 1.1 (May 18, 2020)
        New Features and Improvements:
            - ubranges variable that lists the unicode BMP ranges was added
            - ttf_t::ubranges bit map lists the presented utf16 ranges in font
            - ttf_list_match function was added for fonts matching
            - reading of OS/2 table
        Non-Backwards Compatible Changes:
            - ttf_t::info struct was removed, see ttf_t::names and ttf_t::head instead
        Bug fixes:
            - The ttf_t::info::macStyle structure was not filled before

    Initial release 1.0 (May 16, 2020)
        New Features and Improvements:
            - TrueType fonts support. No OpenType. No vertical fonts support.
            - Tessellator uses the V. Domiter & B. Žalik Sweep‐line algorithm modification
        Non-Backwards Compatible Changes:
            -
        Bug fixes:
            -
*/

#ifndef TTF2MESH_H
#define TTF2MESH_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif

#define TTF2MESH_VERSION   "1.5"  /* current library version */

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

/* definitions for ttf_list_match function */

#define TTF_WEIGHT_THIN       100 /*  */
#define TTF_WEIGHT_EXTRALIGHT 200
#define TTF_WEIGHT_LIGHT      300
#define TTF_WEIGHT_NORMAL     400
#define TTF_WEIGHT_MEDIUM     500
#define TTF_WEIGHT_DEMIBOLD   600
#define TTF_WEIGHT_BOLD       700
#define TTF_WEIGHT_EXTRABOLD  800
#define TTF_WEIGHT_BLACK      900

/* flags and values passing to some function */

#define TTF_QUALITY_LOW    10     /* default quality value for some functions */
#define TTF_QUALITY_NORMAL 20     /* default quality value for some functions */
#define TTF_QUALITY_HIGH   50     /* default quality value for some functions */

#define TTF_FEATURES_DFLT   0     /* default value of ttf_glyph2mesh features parameter */
#define TTF_FEATURE_IGN_ERR 1     /* flag of ttf_glyph2mesh to ignore uncritical mesh errors */

/* lenght of userdata array in ttf_t and ttf_glyph_t structures */

#define TTF_GLYPH_USERDATA 4 /* lenght of userdata array in ttf_t */
#define TTF_FILE_USERDATA  4 /* lenght of userdata array ttf_glyph_t */

/* all library types */

typedef struct ttf_file          ttf_t;
typedef struct ttf_glyph         ttf_glyph_t;
typedef struct ttf_outline       ttf_outline_t;
typedef struct ttf_point         ttf_point_t;
typedef struct ttf_mesh          ttf_mesh_t;
typedef struct ttf_mesh3d        ttf_mesh3d_t;
typedef struct unicode_bmp_range ubrange_t;

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
    uint32_t *chars;              /* utf32 codes array with nchars length */
    uint32_t *char2glyph;         /* glyph indeces array with nchars length */
    ttf_glyph_t *glyphs;          /* array of the font glyphs with nglyphs length */
    const char *filename;         /* full path and file name of the font */
    uint32_t glyf_csum;           /* 'glyf' table checksum (used by ttf_list_fonts) */
    uint32_t ubranges[6];         /* bit map of presented utf16 ranges in font. LSB of word 0
                                   * means that font has a symbols in unicode range #0. All
                                   * ranges are listed in global variable ubranges[] */

    /* unpacked fields of "head" table */
    struct
    {
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
    } head;

    /* unpacked fields of "OS/2" table (OS/2 and Windows Metrics Table) */
    /* https://docs.microsoft.com/en-us/typography/opentype/spec/os2 */
    struct
    {
        float xAvgCharWidth;        /* Average weighted escapement */
        uint16_t usWeightClass;     /* Weight class, see TTF_WEIGHT_XXX */
        uint16_t usWidthClass;      /* 1 Ultra-condensed; 2 Extra-condensed; Condensed; Semi-condensed; Medium (normal); Semi-expanded; Expanded; Extra-expanded; Ultra-expanded */
        float yStrikeoutSize;       /* Thickness of the strikeout stroke */
        float yStrikeoutPos;        /* The position of the top of the strikeout stroke relative to the baseline */
        int16_t sFamilyClass;       /* Font-family class and subclass. Classification of font-family design. https://docs.microsoft.com/en-us/typography/opentype/spec/ibmfc */
        uint8_t panose[10];         /* PANOSE classification number, https://monotype.github.io/panose/ */
        struct
        {
            uint16_t italic : 1;    /* Font contains italic or oblique glyphs, otherwise they are upright */
            uint16_t underscore: 1; /* glyphs are underscored */
            uint16_t negative : 1;  /* glyphs have their foreground and background reversed */
            uint16_t outlined : 1;  /* Outline (hollow) glyphs, otherwise they are solid */
            uint16_t strikeout: 1;  /* glyphs are overstruck */
            uint16_t bold: 1;       /* glyphs are emboldened */
            uint16_t regular: 1;    /* glyphs are in the standard weight/style for the font */
            uint16_t utm: 1;        /* USE_TYPO_METRICS, If set, it is strongly recommended to use OS/2.sTypoAscender - OS/2.sTypoDescender + OS/2.sTypoLineGap as the default line spacing */
            uint16_t oblique: 1;    /* Font contains oblique glyphs */
        } fsSelection;              /* Font selection flags */
        float sTypoAscender;        /* The typographic ascender for this font */
        float sTypoDescender;       /* The typographic descender for this font */
        float sTypoLineGap;         /* The typographic line gap for this font */
        float usWinAscent;          /* The “Windows ascender” metric. This should be used to specify the height above the baseline for a clipping region */
        float usWinDescent;         /* The “Windows descender” metric. This should be used to specify the vertical extent below the baseline for a clipping region */
    } os2;

    struct
    {
        /* unpacked fields of "name" table */
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
    } names;

    /* unpacked fields of "hhea" table (information for horizontal layout) */
    /* https://docs.microsoft.com/en-us/typography/opentype/spec/hhea */
    struct
    {
        float ascender;           /* Typographic ascent (Distance from baseline of highest ascender) */
        float descender;          /* Typographic descent (Distance from baseline of lowest descender) */
        float lineGap;            /* Typographic line gap (Distance from line1 descender to line2 ascender) */
        float advanceWidthMax;    /* Maximum advance width value */
        float minLSideBearing;    /* Minimum left sidebearing value */
        float minRSideBearing;    /* Minimum right sidebearing value; calculated as Min(aw - lsb - (xMax - xMin)). */
        float xMaxExtent;         /* Max(lsb + (xMax - xMin)) */
        float caretSlope;         /* The slope of the cursor in radians, ~0 for horizontal not italic font and ~0.2 for italic font */
    } hhea;

    /* for external use */

    void *userdata[TTF_FILE_USERDATA];
};

/**
 * @brief The glyph struct
 */
struct ttf_glyph
{
    /* general fields */

    int index;                    /* glyph index in font */
    int symbol;                   /* utf-16 symbol */
    int npoints;                  /* total points within all contours */
    int ncontours;                /* number of contours in outline */
    uint32_t composite : 1;       /* it is composite glyph */
    uint32_t : 31;                /* reserved flags */

    /* horizontal glyph metrics */
    /* see https://docs.microsoft.com/en-us/typography/opentype/spec/hmtx */

    float xbounds[2];             /* min/max values ​​along the x coordinate */
    float ybounds[2];             /* min/max values ​​along the y coordinate */
    float advance;                /* advance width */
    float lbearing;               /* left side bearing */
    float rbearing;               /* right side bearing = aw - (lsb + xMax - xMin) */

    /* glyph outline */

    ttf_outline_t *outline;       /* original outline of the glyph or NULL */

    /* for external use */

    void *userdata[TTF_GLYPH_USERDATA];
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
    uint32_t res : 30;            /* reserved for internal use */
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
 * @brief The mesh struct
 */
struct ttf_mesh3d
{
    int nvert;                    /* length of vert array */
    int nfaces;                   /* length of faces array */
    struct
    {
        float x;
        float y;
        float z;
    } *vert;                      /* vertices */
    struct
    {
        int v1;                   /* index of vertex #1 of triangle */
        int v2;                   /* index of vertex #2 of triangle */
        int v3;                   /* index of vertex #3 of triangle */
    } *faces;                     /* triangles */
    struct
    {
        float x;
        float y;
        float z;
    } *normals;                   /* normals array with 3*nfaces length */
    ttf_outline_t *outline;       /* see ttf_linear_outline() */
};

/**
 * @brief The Unicode Basic Multilingual Plane range struct
 */
struct unicode_bmp_range
{
    uint16_t first;   /* first code in range */
    uint16_t last;    /* last code in range */
    const char *name; /* range name */
};

extern const ubrange_t ubranges[163];

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
 * @param mask Font file name pattern (can be NULL)
 * @return Array of references to ttf_t objects or NULL if no memory
 *
 * The list of fonts returned is terminated with a NULL reference.
 * Any font in list has no loaded glyphs but values of other fields are presented.
 * After using the list, you need to free it with ttf_free_list function.
 * The \a mask parameter specifies a file name pattern or a series of patterns separated 
 * by the "|" character. At the end of the pattern string the "*" character is allowed.
 * If all characters in the file name string match before the "*", the font with that name 
 * is accepted. The text matching process is case insensitive.
 */
ttf_t **ttf_list_fonts(const char **directories, int dir_count, const char *mask);

/**
 * @brief List available system fonts
 * @param mask Font file name pattern (can be NULL)
 * @return Array of references to ttf_t objects or NULL if no memory
 *
 * This function is similar to ttf_list_fonts, but lists the fonts in the system font
 * directories, which are different for different operating systems. 
 */
ttf_t **ttf_list_system_fonts(const char *mask);

/**
 * @brief Matching font from list
 * @param list NULL-terminated array of references to ttf_t objects
 * @param deflt Default font
 * @param requirements Requirements string
 * @return Matched font from \a list or \a deflt value if no font matched
 *
 * The function allows to find a font from a list with arbitrary requirements.
 * The \a requirements parameter sets the criteria and priority of the search.
 * Depending on the value of the \a requirements parameter, the function takes
 * a different number of arguments.
 *
 * For example, the following call allows you to find the bold version of
 * "Courier" or "Courier New" font:
 *
 *     ttf_list_match(list, "fb", "Courier")
 *
 * The following call forces to search any italic font that guarantees the
 * representation of the specified utf16-string. In addition, two preferred
 * fonts are specified (The "Times" has a higher priority than the "Courier"):
 *
 *     ttf_list_match(list, "t!i!ff", L"The number π", "Times", "Courier")
 *
 * The following table lists the modifiers that passing in \a requirements parameter.
 *
 * |modifier | argument type  |  description                                                    |
 * |---------|----------------|-----------------------------------------------------------------|
 * | b       | -              | forces to search the bold font                                  |
 * | i       | -              | forces to search the italic font                                |
 * | h       | -              | forces to search the font with hollow glyps                     |
 * | o       | -              | forces to search the oblique font                               |
 * | r       | -              | forces to search the regular font (default modifier)            |
 * | w       | TTF_WEIGHT_XXX | forces to search the font with specified weight                 |
 * | f       | char *family   | forces to search the font with specified family name            |
 * | t       | uint16_t *text | forces to search the font that can represent the specified text |
 * | !       | -              | forces to search for an exact match by the previous modifier    |
 *
 * A few notes on the proper use of ttf_list_match:
 * - No more than 32 modifiers are allowed.
 * - Position of any modifier with the '!' suffix in string does not matter.
 * - The "r!" and "b!" requirements are incompatible.
 */
ttf_t *ttf_list_match(ttf_t **list, ttf_t *deflt, const char *requirements, ...);

/**
 * @brief Same as ttf_list_match.
 * @return Index of matched font from \a list or -1 value if no font matched
 */
int ttf_list_match_id(ttf_t **list, const char *requirements, ...);

/**
 * @brief Translate unicode character to glyph index in font object
 * @param ttf Pointer to font object
 * @param utf32_char Unicode character (utf16 or utf32)
 * @return Glyph index in glyphs array or -1
 */
int ttf_find_glyph(const ttf_t *ttf, uint32_t utf32_char);

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
 * @param xscale Scaling the glyph horizontally
 * @param yscale Scaling the glyph vertically
 * @return Null terminated string or NULL if no memory in system
 */
char *ttf_glyph2svgpath(ttf_glyph_t *glyph, float xscale, float yscale);

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
 * @brief Convert glyph to mesh3d-object
 * @param glyph Pointer to glyph object
 * @param output Pointer to mesh3d object or NULL if error occurred
 * @param quality Number of points to circle (see TTF_QUALITY_XXX)
 * @param features Process features and tricks (see TTF_FEATURES_DFLT, TTF_FEATURE_XXX)
 * @param depth Depth of the object
 * @return Operation result TTF_XXX
 */
int ttf_glyph2mesh3d(ttf_glyph_t *glyph, ttf_mesh3d_t **output, uint8_t quality, int features, float depth);

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
 * @brief Free the mesh3d object
 * @param mesh Pointer to mesh object
 *
 * It is necessary to call after working with mesh object
 * which was allocated by ttf_glyph2mesh function
 */
void ttf_free_mesh3d(ttf_mesh3d_t *mesh);

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
