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

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
/*********************************** MODULE ***********************************/
/********************************* DEFINITIONS ********************************/
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

/* --------------- System dependent definitions and includes ---------------- */

/* General OS selection definition */
#if defined(__linux) || defined(__linux__)
#   define TTF_LINUX
#   define _DEFAULT_SOURCE 1
#   define PATH_SEP '/'
#   include <dirent.h>
#elif defined(__WINNT__) || defined(_WIN32) || defined(_WIN64)
#   define TTF_WINDOWS
#   define _CRT_SECURE_NO_WARNINGS
#   define PATH_SEP '\\'
#   define PATH_MAX MAX_PATH
#   include <windows.h>
#endif

#include "ttf2mesh.h"
#include <string.h>
#include <malloc.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>

/* Big/little endian definitions */
#if defined(__BYTE_ORDER__) && defined(__ORDER_LITTLE_ENDIAN__)
#   if (__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__)
#       define HOST_IS_LITTLE_ENDIAN
#   endif
#elif defined(REG_DWORD)
#   if (REG_DWORD == REG_DWORD_LITTLE_ENDIAN)
#       define HOST_IS_LITTLE_ENDIAN
#   endif
#else
#   error NO __BYTE_ORDER__ DEFINITION
#endif

#define LINUX_FONTS_PATH      \
    "/usr/share/fonts",       \
    "/usr/local/share/fonts", \
    "~/.fonts"

#define WINDOWS_FONTS_PATH    \
    "C:\\Windows\\Fonts"

/* ---------------------- Different common definitions ---------------------- */

/* Minimal single precision value */
#define EPSILON 1e-7f

/* Pi. It specified more decimal places than it necessary for */
/* single precision floating numbers. Let the preprocessor do */
/* the optimization accurately. */
#ifdef M_PI
#   define pi M_PI
#else
#   define pi 3.14159265358979323846
#endif

/* Swapping of two values */
#define SWAP(type, A, B) { type tmp = A; A = B; B = tmp; }

/* -------------------- Working with vectors on xy-plane -------------------- */

#define VECLEN2(a)     ((a)[0] * (a)[0] + (a)[1] * (a)[1]) /* Squared length vector */
#define VECLEN(a)      (sqrtf(VECLEN2(a)))                 /* Vector length */
#define VECDOT(a, b)   (a[0] * b[0] + a[1] * b[1])         /* Dot product of two vectors */
#define VECCROSS(a, b) (a[0] * b[1] - a[1] * b[0])         /* Third component of cross product of two vectors on xy-plane */
/* Subtraction of vectors */
#define VECSUB(res, a, b) { \
    (res)[0] = (a)[0] - (b)[0]; \
    (res)[1] = (a)[1] - (b)[1]; \
}
/* Sum of vectors */
#define VECADD(res, a, b) { \
    (res)[0] = (a)[0] + (b)[0]; \
    (res)[1] = (a)[1] + (b)[1]; \
}
/* Vector scaling */
#define VECSCALE(res, v, scale) { \
    float S = (scale); \
    (res)[0] = (v)[0] * S; \
    (res)[1] = (v)[1] * S; \
}
/* Projecting a vector onto the basis vectors e1 and e2 */
#define VECPROJ(res, vec, e1, e2) { \
    float t; \
    t = VECDOT(vec, e1); \
    res[1] = VECDOT(vec, e2); \
    res[0] = t; \
}
/* Cramer's rule for 2x2 system */
#define CRAMER2x2(x, y, a11, a12, c1, a21, a22, c2) { \
    float det = (a11) * (a22) - (a12) * (a21); \
    x = ((c1) * (a22) - (c2) * (a12)) / det; \
    y = ((c2) * (a11) - (c1) * (a21)) / det; \
}

/* --------------------- Working with doubly linked list --------------------- */

#define LIST_DETACH(element) { \
    (element)->prev->next = (element)->next; \
    (element)->next->prev = (element)->prev; }

#define LIST_INS_AFTER(what, where) { \
    (what)->prev = (where); \
    (what)->next = (where)->next; \
    (what)->prev->next = (what); \
    (what)->next->prev = (what); } \

#define LIST_ATTACH(root, element) { \
    (element)->prev = (root); \
    (element)->next = (root)->next; \
    (root)->next->prev = (element); \
    (root)->next = (element); }

#define LIST_REATTACH(new_root, element) { LIST_DETACH(element); LIST_ATTACH(new_root, element); }

#define LIST_INS_LAST(root, element) LIST_INS_AFTER(element, (root)->prev)

#define LIST_EMPTY(root) ((root)->next == (root))
#define LIST_FIRST(root) ((root)->next)
#define LIST_INIT(root)  { (root)->next = (root); (root)->prev = (root); }

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
/******************************* TRUETYPE FORMAT ******************************/
/******************************** SPECIFICATION *******************************/
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

#pragma pack(push, 1)

typedef struct ttf_header
{
    uint32_t sfntVersion; /* 0x00010000 or 0x4F54544F ('OTTO') */
    uint16_t numTables; /* Number of tables */
    uint16_t searchRange; /* (Maximum power of 2 <= numTables) x 16 */
    uint16_t entrySelector; /* Log2(maximum power of 2 <= numTables) */
    uint16_t rangeShift; /* NumTables x 16-searchRange */
} ttf_file_hdr_t;

/* https://docs.microsoft.com/typography/opentype/spec/cmap */
typedef struct ttf_cmap_table
{
    uint16_t version; /* Table version number (0) */
    uint16_t numTables; /* Number of encoding tables that follow */
    struct
    {
        uint16_t platformID; /* Platform ID */
        uint16_t encodingID; /* Platform-specific encoding ID */
        uint32_t offset; /* Byte offset from beginning of table to the subtable for this encoding */
    } encRecs[1];
} ttf_cmap_t;

typedef struct ttf_cmap_format4_table
{
    uint16_t format; /* Format number is set to 4 */
    uint16_t length; /* This is the length in bytes of the subtable */
    uint16_t language; /* see https://docs.microsoft.com/typography/opentype/spec/cmap#language */
    uint16_t segCountX2; /* 2 × segCount */
    uint16_t searchRange; /* 2 × (2**floor(log2(segCount))) */
    uint16_t entrySelector; /* log2(searchRange/2) */
    uint16_t rangeShift; /* 2 × segCount - searchRange */
} ttf_fmt4_t;

typedef struct ttf_glyf_table_header
{
    int16_t numberOfContours; /* simple glyph if >= 0. composite glyph if < 0 */
    int16_t xMin; /* Minimum x for coordinate data */
    int16_t yMin; /* Minimum y for coordinate data */
    int16_t xMax; /* Maximum x for coordinate data */
    int16_t yMax; /* Maximum y for coordinate data */
} ttf_glyfh_t;

typedef struct ttf_table_record
{
    char tableTag[4]; /* Table identifier */
    uint32_t checkSum; /* CheckSum for this table */
    uint32_t offset; /* Offset from beginning of TrueType font file */
    uint32_t length; /* Length of this table */
} ttf_tab_rec_t;

/* https://docs.microsoft.com/typography/opentype/spec/head */
typedef struct ttf_head_table
{
    uint16_t majorVersion;       /* Major version number of the font header table — set to 1 */
    uint16_t minorVersion;       /* Minor version number of the font header table — set to 0 */
    int16_t  fontRevisionI;      /* Set by font manufacturer. Integer part of revision value */
    uint16_t fontRevisionF;      /* Set by font manufacturer. Fractional part of revision value */
    uint32_t checkSumAdjustment; /* see https://docs.microsoft.com/typography/opentype/spec/head */
    uint32_t magicNumber;        /* Set to 0x5F0F3CF5 */
    uint16_t flags;              /* see https://docs.microsoft.com/typography/opentype/spec/head */
    uint16_t unitsPerEm;         /* see https://docs.microsoft.com/typography/opentype/spec/head */
    uint64_t created;            /* Number of seconds since 24:00 that started 01.06.1904 in GMT/UTC time zone */
    uint64_t modified;           /* Number of seconds since 24:00 that started 01.06.1904 in GMT/UTC time zone */
    int16_t  xMin;               /* For all glyph bounding boxes */
    int16_t  yMin;               /* For all glyph bounding boxes */
    int16_t  xMax;               /* For all glyph bounding boxes */
    int16_t  yMax;               /* For all glyph bounding boxes */
    struct
    {
        uint8_t bold : 1;
        uint8_t italic : 1;
        uint8_t underline : 1;
        uint8_t outline : 1;
        uint8_t shadow : 1;
        uint8_t condensed : 1;
        uint8_t extended : 1;
        uint8_t : 1;
        uint8_t : 8;
    } macStyle;
    uint16_t lowestRecPPEM;      /* Smallest readable size in pixels */
    int16_t fontDirectionHint;   /* Deprecated (Set to 2) */
    int16_t indexToLocFormat;    /* 0 for short offsets (Offset16), 1 for long (Offset32) */
    int16_t glyphDataFormat;     /* 0 for current format */
} ttf_head_t;

/* https://docs.microsoft.com/ru-ru/typography/opentype/spec/maxp */
typedef struct ttf_maxp_table
{
    uint16_t verMaj;    /* 0x0001 for version 1.0 */
    uint16_t verMin;
    uint16_t numGlyphs; /* The number of glyphs in the font */

    /* fields below are actual for version 1.0 or greater */
    uint16_t maxPoints; /* Maximum points in a non-composite glyph */
    uint16_t maxContours; /* Maximum contours in a non-composite glyph */
    uint16_t maxCompositePoints; /* Maximum points in a composite glyph */
    uint16_t maxCompositeContours; /* Maximum contours in a composite glyph */
    uint16_t maxZones; /* 1 if instructions do not use the twilight zone (Z0), or 2 if instructions do use Z0; should be set to 2 in most cases */
    uint16_t maxTwilightPoints; /* Maximum points used in Z0 */
    uint16_t maxStorage; /* Number of Storage Area locations */
    uint16_t maxFunctionDefs; /* Number of FDEFs, equal to the highest function number + 1 */
    uint16_t maxInstructionDefs; /* Number of IDEFs */
    uint16_t maxStackElements; /* Maximum stack depth across Font Program ('fpgm' table), CVT Program ('prep' table) and all glyph instructions (in the 'glyf' table) */
    uint16_t maxSizeOfInstructions; /* Maximum byte count for glyph instructions */
    uint16_t maxComponentElements; /* Maximum number of components referenced at “top level” for any composite glyph */
    uint16_t maxComponentDepth; /* Maximum levels of recursion; 1 for simple components */
} ttf_maxp_t;

/* https://docs.microsoft.com/ru-ru/typography/opentype/spec/name */
typedef struct ttf_namerecord
{
    uint16_t platformID; /* Platform ID */
    uint16_t encodingID; /* Platform-specific encoding ID */
    uint16_t languageID; /* Language ID */
    uint16_t nameID;     /* Name ID */
    uint16_t length;     /* String length (in bytes) */
    uint16_t offset;     /* String offset from start of storage area (in bytes) */
} ttf_namerec_t;

/* https://docs.microsoft.com/ru-ru/typography/opentype/spec/name */
typedef struct ttf_name_table
{
    uint16_t format; /* Format selector (=0/1) */
    uint16_t count; /* Number of name records */
    uint16_t stringOffset; /* Offset to start of string storage (from start of table) */
    ttf_namerec_t nameRecord[1]; /* nameRecord[count] The name records where count is the number of records */
    /*
        extension for format 1:
        uint16        langTagCount                      Number of language-tag records.
        LangTagRecord langTagRecord[langTagCount]       The language-tag records where langTagCount is the number of records.
    */
} ttf_name_t;

/* https://docs.microsoft.com/ru-ru/typography/opentype/spec/hhea */
typedef struct ttf_hhea_table
{
    uint16_t majorVersion;        /* Major version number of the horizontal header table — set to 1 */
    uint16_t minorVersion;        /* Minor version number of the horizontal header table — set to 0 */
    int16_t  ascender;            /* Typographic ascent (Distance from baseline of highest ascender) */
    int16_t  descender;           /* Typographic descent (Distance from baseline of lowest descender) */
    int16_t  lineGap;             /* Typographic line gap. Negative LineGap values are treated as zero in some legacy platform implementations */
    uint16_t advanceWidthMax;     /* Maximum advance width value in 'hmtx' table */
    int16_t  minLeftSideBearing;  /* Minimum left sidebearing value in 'hmtx' table */
    int16_t  minRightSideBearing; /* Minimum right sidebearing value; calculated as Min(aw - lsb - (xMax - xMin)) */
    int16_t  xMaxExtent;          /* Max(lsb + (xMax - xMin)) */
    int16_t  caretSlopeRise;      /* Used to calculate the slope of the cursor (rise/run); 1 for vertical */
    int16_t  caretSlopeRun;       /* 0 for vertical */
    int16_t  caretOffset;         /* The amount by which a slanted highlight on a glyph needs to be shifted to produce the best appearance. Set to 0 for non-slanted fonts */
    int16_t  reserved1;           /* set to 0 */
    int16_t  reserved2;           /* set to 0 */
    int16_t  reserved3;           /* set to 0 */
    int16_t  reserved4;           /* set to 0 */
    int16_t  metricDataFormat;    /* 0 for current format */
    uint16_t numberOfHMetrics;    /* Number of hMetric entries in 'hmtx' table */
} ttf_hhea_t;

#pragma pack(pop)

/* ------------------- Parser private structure ------------------ */

typedef struct ttf_parser_private_struct
{
    ttf_file_hdr_t *hdr;
    ttf_head_t *phead;
    ttf_maxp_t *pmaxp;
    ttf_cmap_t *pcmap;
    ttf_name_t *pname;
    ttf_hhea_t *phhea;
    uint16_t *phmtx;
    uint8_t *pglyf;
    uint8_t *ploca;
    uint16_t *ploca16;
    uint32_t *ploca32;
    int shead;
    int smaxp;
    int sloca;
    int scmap;
    int sname;
    int shhea;
    int shmtx;
    int sglyf;
    uint32_t glyf_csum;
} pps_t;

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
/*************************** TRUETYPE FORMAT PARSER ***************************/
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

/* Big/little endian conversion helpers */

#if defined(HOST_IS_LITTLE_ENDIAN)
static __inline uint16_t big16toh(uint16_t x)
{
    return ((unsigned short int) ((((x) >> 8) & 0xff) | (((x) & 0xff) << 8)));
}

static __inline uint32_t big32toh(uint32_t x)
{
    return ((((x) & 0xff000000) >> 24) | (((x) & 0x00ff0000) >>  8) |
           (((x) & 0x0000ff00) <<  8) | (((x) & 0x000000ff) << 24));
}

static __inline uint64_t big64toh(uint64_t x)
{
    return ((uint64_t)big32toh((uint32_t)x) << 32) | big32toh(x >> 32);
}

#   define conv16(v) v = big16toh(v)
#   define conv32(v) v = big32toh(v)
#   define conv64(v) v = big64toh(v)
#else
static __inline uint16_t big16toh(uint16_t x)
{
    return x;
}

static __inline uint32_t big32toh(uint32_t x)
{
    return x;
}

static __inline uint64_t big64toh(uint64_t x)
{
    return x;
}

#   define conv16(v)
#   define conv32(v)
#   define conv64(v)
#endif

static uint32_t ttf_checksum(const void *data, int size)
{
    uint32_t *ptr;
    uint32_t sum;
    sum = 0;
    ptr = (uint32_t *)data;
    while (size > 0)
    {
        sum += big32toh(*ptr++);
        size -= 4;
    }
    return sum;
}

#define check(cond, code) \
if (!(cond)) { result = code; goto error; }

ttf_outline_t *allocate_ttf_outline(int ncontours, int npoints)
{
    ttf_outline_t *res;
    int n = sizeof(ttf_outline_t);
    n += npoints * sizeof(ttf_point_t); /* points buffer size */
    n += (ncontours - 1) * sizeof(res->cont); /* contours buffer size */
    res = (ttf_outline_t *)calloc(n, 1);
    if (res == NULL) return NULL;
    res->ncontours = ncontours;
    res->total_points = npoints;
    res->cont[0].pt = (ttf_point_t *)&res->cont[ncontours];
    return res;
}

int parse_simple_glyph(ttf_glyph_t *glyph, int glyph_index, uint8_t *p, int avail)
{
    uint16_t *endPtsOfContours;
    uint8_t flag;
    uint8_t rep;
    ttf_glyfh_t hdr;
    int32_t x, y;
    int i, j, n;

    /* read and store glyph header */
    if (avail < (int)sizeof(ttf_glyfh_t)) return TTF_ERR_FMT;
    hdr = *(ttf_glyfh_t *)p;
    conv16(hdr.numberOfContours);
    conv16(hdr.xMin);
    conv16(hdr.yMin);
    conv16(hdr.xMax);
    conv16(hdr.yMax);
    p += sizeof(ttf_glyfh_t);
    avail -= sizeof(ttf_glyfh_t);

    /* read endPtsOfContours */
    if (hdr.numberOfContours == 0) return TTF_ERR_FMT;
    if (avail < hdr.numberOfContours * 2) return TTF_ERR_FMT;
    endPtsOfContours = (uint16_t *)p;
    p += hdr.numberOfContours * 2;
    avail -= hdr.numberOfContours * 2;

    /* fill glyph structure and allocate points */
    glyph->ncontours = hdr.numberOfContours;
    glyph->npoints = big16toh(endPtsOfContours[glyph->ncontours - 1]) + 1;
    glyph->xbounds[0] = hdr.xMin;
    glyph->xbounds[1] = hdr.xMax;
    glyph->ybounds[0] = hdr.yMin;
    glyph->ybounds[1] = hdr.yMax;

    /* initialize outline */
    glyph->outline = allocate_ttf_outline(glyph->ncontours, glyph->npoints);
    if (glyph->outline == NULL) return TTF_ERR_NOMEM;

    j = 0;
    n = -1;
    for (i = 0; i < glyph->ncontours; i++)
    {
        j = (int)big16toh(endPtsOfContours[i]);
        glyph->outline->cont[i].length = j - n;
        glyph->outline->cont[i].subglyph_id = glyph_index;
        glyph->outline->cont[i].subglyph_order = 0;
        if (i != glyph->ncontours - 1)
            glyph->outline->cont[i + 1].pt = glyph->outline->cont[i].pt + j - n;
        n = j;
    }

    /* read instructionLength */
    if (avail < 2) return TTF_ERR_FMT;
    n = big16toh(*(uint16_t *)p);
    p += 2;
    avail -= 2;

    /* skip instructions */
    if (avail < n) return TTF_ERR_FMT;
    p += n;
    avail -= n;

    /* read flags */
    #define ON_CURVE_POINT 0x01
    #define X_SHORT_VECTOR 0x02
    #define Y_SHORT_VECTOR 0x04
    #define REPEAT_FLAG    0x08
    #define X_IS_SAME_OR_POSITIVE_X_SHORT_VECTOR 0x10
    #define Y_IS_SAME_OR_POSITIVE_Y_SHORT_VECTOR 0x20
    #define OVERLAP_SIMPLE 0x40
    flag = 0;
    rep = 0;
    for (i = 0; i < glyph->ncontours; i++)
        for (j = 0; j < glyph->outline->cont[i].length; j++)
        {
            if (rep == 0)
            {
                if (avail < 1) return TTF_ERR_FMT;
                flag = *p;
                p++;
                avail--;
                if (flag & REPEAT_FLAG)
                {
                    if (avail < 1) return TTF_ERR_FMT;
                    rep = *p;
                    p++;
                    avail--;
                }
            }
            else
            {
                rep--;
            }
            glyph->outline->cont[i].pt[j].onc = flag & ON_CURVE_POINT;
            glyph->outline->cont[i].pt[j].res = flag;
        }

    /* read x coordinate */
    x = 0;
    for (i = 0; i < glyph->ncontours; i++)
        for (j = 0; j < glyph->outline->cont[i].length; j++)
        {
            /*
             * X_SHORT   X_IS_SAME   description
             *    0         0        int16 value
             *    0         1        apply previous value
             *    1         0        uint8, sign -
             *    1         1        uint8, sign +
            */
            flag = glyph->outline->cont[i].pt[j].res;
            switch (flag & (X_SHORT_VECTOR | X_IS_SAME_OR_POSITIVE_X_SHORT_VECTOR))
            {
            case 0:
                if (avail < 2) return TTF_ERR_FMT;
                x += (int16_t)big16toh(*(int16_t *)p);
                avail -= 2;
                p += 2;
                break;
            case X_IS_SAME_OR_POSITIVE_X_SHORT_VECTOR:
                break;
            case X_SHORT_VECTOR:
                if (avail < 1) return TTF_ERR_FMT;
                x -= *(uint8_t *)p++;
                avail--;
                break;
            case X_SHORT_VECTOR | X_IS_SAME_OR_POSITIVE_X_SHORT_VECTOR:
                if (avail < 1) return TTF_ERR_FMT;
                x += *(uint8_t *)p++;
                avail--;
                break;
            }
            glyph->outline->cont[i].pt[j].x = (float)x;
        }

    /* read y coordinate */
    y = 0;
    for (i = 0; i < glyph->ncontours; i++)
        for (j = 0; j < glyph->outline->cont[i].length; j++)
        {
            flag = glyph->outline->cont[i].pt[j].res;
            switch (flag & (Y_SHORT_VECTOR | Y_IS_SAME_OR_POSITIVE_Y_SHORT_VECTOR))
            {
            case 0:
                if (avail < 2) return TTF_ERR_FMT;
                y += (int16_t)big16toh(*(int16_t *)p);
                avail -= 2;
                p += 2;
                break;
            case Y_IS_SAME_OR_POSITIVE_Y_SHORT_VECTOR:
                break;
            case Y_SHORT_VECTOR:
                if (avail < 1) return TTF_ERR_FMT;
                y -= *(uint8_t *)p++;
                avail--;
                break;
            case Y_SHORT_VECTOR | Y_IS_SAME_OR_POSITIVE_Y_SHORT_VECTOR:
                if (avail < 1) return TTF_ERR_FMT;
                y += *(uint8_t *)p++;
                avail--;
                break;
            }
            glyph->outline->cont[i].pt[j].y = (float)y;
        }

    /* shift contours */
    for (i = 0; i < glyph->outline->ncontours; i++)
    {
        ttf_point_t *p;
        ttf_point_t first;
        int offset;
        n = glyph->outline->cont[i].length;
        p = glyph->outline->cont[i].pt;
        if (n < 2) continue;
        for (offset = 0; offset < n && !p->onc; offset++)
        {
            first = p[0];
            for (j = 0; j < n - 1; j++)
                p[j] = p[j + 1];
            p[n - 1] = first;
        }
    }

    return 0;
}

static __inline float f2dot14_to_float(int16_t f2dot14)
{
    return (float)f2dot14 / 16384;
}

int parse_composite_glyph(ttf_t *ttf, ttf_glyph_t *glyph, uint8_t *p, int avail)
{
    #define ARG_1_AND_2_ARE_WORDS     0x0001
    #define ARGS_ARE_XY_VALUES        0x0002
    #define ROUND_XY_TO_GRID          0x0004
    #define WE_HAVE_A_SCALE           0x0008
    #define MORE_COMPONENTS           0x0020
    #define WE_HAVE_AN_X_AND_Y_SCALE  0x0040
    #define WE_HAVE_A_TWO_BY_TWO      0x0080
    #define WE_HAVE_INSTRUCTIONS      0x0100
    #define USE_MY_METRICS            0x0200
    #define OVERLAP_COMPOUND          0x0400
    #define SCALED_COMPONENT_OFFSET   0x0800
    #define UNSCALED_COMPONENT_OFFSET 0x1000

    uint8_t *stored_p;
    int stored_avail;
    int nglyphs;
    ttf_glyfh_t hdr;
    ttf_point_t *curr;
    unsigned flags; /* component flag */
    unsigned glyphIndex; /* glyph index of component */
    int16_t arg1, arg2; /* arguments */
    float scale[2][2];
    int i, j, n;

    /* read and store glyph header */
    if (avail < (int)sizeof(ttf_glyfh_t)) return TTF_ERR_FMT;
    hdr = *(ttf_glyfh_t *)p;
    conv16(hdr.numberOfContours);
    conv16(hdr.xMin);
    conv16(hdr.yMin);
    conv16(hdr.xMax);
    conv16(hdr.yMax);
    p += sizeof(ttf_glyfh_t);
    avail -= sizeof(ttf_glyfh_t);

    /* first count the number of glyphs and contours */
    flags = MORE_COMPONENTS;
    stored_p = p;
    stored_avail = avail;
    nglyphs = 0;
    while (flags & MORE_COMPONENTS)
    {
        if (avail < 4) return TTF_ERR_FMT;
        flags = big16toh(*(uint16_t *)(p + 0));
        glyphIndex = big16toh(*(uint16_t *)(p + 2));
        avail -= 4;
        p += 4;
        if (flags & ARGS_ARE_XY_VALUES)
        {
            /* Argument1 and argument2 are x and y offsets to be added to the glyph */
        }
        else
        {
            /* Argument1 and argument2 are two point numbers. */
            /* The first point number indicates the point that is to be matched to the new glyph. */
            /* The second number indicates the new glyph’s “matched” point */
            return TTF_ERR_FMT; /* TODO: support this  feature */
        }
        n = flags & ARG_1_AND_2_ARE_WORDS ? 4 : 2;
        if (flags & WE_HAVE_A_SCALE)
            n += 2; else
        if (flags & WE_HAVE_AN_X_AND_Y_SCALE)
            n += 4; else
        if (flags & WE_HAVE_A_TWO_BY_TWO)
            n += 8;
        avail -= n;
        p += n;
        if (avail < 0) return TTF_ERR_FMT;
        if (glyphIndex >= (unsigned)ttf->nglyphs) return TTF_ERR_FMT;
        glyph->ncontours += ttf->glyphs[glyphIndex].ncontours;
        glyph->npoints += ttf->glyphs[glyphIndex].npoints;
        nglyphs++;
    }
    if (glyph->ncontours == 0 || glyph->npoints == 0)
    {
        /* FIXME: support composite glyph recursion! */
        glyph->ncontours = 0;
        glyph->npoints = 0;
        return 0;
    }
    p = stored_p;
    avail = stored_avail;

    /* initialize outline */
    glyph->outline = allocate_ttf_outline(glyph->ncontours, glyph->npoints);
    if (glyph->outline == NULL) return TTF_ERR_NOMEM;

    glyph->xbounds[0] = hdr.xMin;
    glyph->xbounds[1] = hdr.xMax;
    glyph->ybounds[0] = hdr.yMin;
    glyph->ybounds[1] = hdr.yMax;

    n = 0;
    nglyphs = 0;
    curr = glyph->outline->cont[0].pt;
    flags = MORE_COMPONENTS;
    while (flags & MORE_COMPONENTS)
    {
        /* reading flags, glyph index and transformation matrix */

        flags = big16toh(*(uint16_t *)(p + 0));
        glyphIndex = big16toh(*(uint16_t *)(p + 2));
        avail -= 4;
        p += 4;

        if (flags & ARG_1_AND_2_ARE_WORDS)
        {
            arg1 = big16toh(*(uint16_t *)(p + 0));
            arg2 = big16toh(*(uint16_t *)(p + 2));
            avail -= 4;
            p += 4;
        }
        else
        {
            arg1 = (int8_t)*p++;
            arg2 = (int8_t)*p++;
            avail -= 2;
        }

        scale[0][0] = 1.0f;
        scale[0][1] = 0.0f;
        scale[1][0] = 0.0f;
        scale[1][1] = 1.0f;
        if (flags & WE_HAVE_A_SCALE)
        {
            scale[0][0] = f2dot14_to_float(big16toh(*(int16_t *)p));
            scale[1][1] = scale[0][0];
            avail -= 2;
            p += 2;
        }
        else
        if (flags & WE_HAVE_AN_X_AND_Y_SCALE)
        {
            scale[0][0] = f2dot14_to_float(big16toh(*(int16_t *)(p + 0)));
            scale[1][1] = f2dot14_to_float(big16toh(*(int16_t *)(p + 2)));
            avail -= 4;
            p += 4;
        }
        else
        if (flags & WE_HAVE_A_TWO_BY_TWO)
        {
            scale[0][0] = f2dot14_to_float(big16toh(*(int16_t *)(p + 0)));
            scale[0][1] = f2dot14_to_float(big16toh(*(int16_t *)(p + 2)));
            scale[1][0] = f2dot14_to_float(big16toh(*(int16_t *)(p + 4)));
            scale[1][1] = f2dot14_to_float(big16toh(*(int16_t *)(p + 6)));
            avail -= 8;
            p += 8;
        }
        /* If neither flag is set, then the rasterizer will apply a default behavior */
        if ((flags & SCALED_COMPONENT_OFFSET) == 0 && (flags & UNSCALED_COMPONENT_OFFSET) == 0)
            flags |= UNSCALED_COMPONENT_OFFSET;
        /* copy and transform contours from other glyph */
        for (i = 0; i < ttf->glyphs[glyphIndex].ncontours; i++)
        {
            glyph->outline->cont[n].pt = curr;
            glyph->outline->cont[n].length = ttf->glyphs[glyphIndex].outline->cont[i].length;
            glyph->outline->cont[n].subglyph_id = glyphIndex;
            glyph->outline->cont[n].subglyph_order = nglyphs;
            for (j = 0; j < glyph->outline->cont[n].length; j++)
            {
                float x, y;
                *curr = ttf->glyphs[glyphIndex].outline->cont[i].pt[j];
                x = curr->x + ((flags & SCALED_COMPONENT_OFFSET) ? arg1 : 0);
                y = curr->y + ((flags & SCALED_COMPONENT_OFFSET) ? arg2 : 0);
                curr->x = scale[0][0] * x + scale[0][1] * y + (flags & UNSCALED_COMPONENT_OFFSET ? arg1 : 0);
                curr->y = scale[1][0] * x + scale[1][1] * y + (flags & UNSCALED_COMPONENT_OFFSET ? arg2 : 0);
                 curr++;
            }
            n++;
        }
        nglyphs++;
    }
    return 0;
}

int parse_glyf_table(ttf_t *ttf, pps_t *pp)
{
    int i;
    int offset;
    ttf_glyfh_t *hdr;
    int result;

    /* read simple glyphs from table */
    for (i = 0; i < ttf->nglyphs; i++)
    {
        if (pp->ploca16)
        {
            offset = pp->ploca16[i] * 2;
            if (i < ttf->nglyphs - 1)
                if (pp->ploca16[i] == pp->ploca16[i + 1])
                    continue; /* glyph has no outline */
        }
        else
        {
            offset = pp->ploca32[i];
            if (i < ttf->nglyphs - 1)
                if (pp->ploca32[i] == pp->ploca32[i + 1])
                    continue; /* glyph has no outline */
        }
        /* offset = pp->ploca16 ? pp->ploca16[i] * 2 : pp->ploca32[i]; */
        if (offset == pp->sglyf) continue; /* glyph has no outline */
        if (offset + (int)sizeof(ttf_glyfh_t) >= pp->sglyf)
            continue; /* strict parser must return TTF_ERR_FMT */
        hdr = (ttf_glyfh_t *)(pp->pglyf + offset);
        if ((int16_t)big16toh(hdr->numberOfContours) < 0) continue;
        result = parse_simple_glyph(ttf->glyphs + i, i, pp->pglyf + offset, pp->sglyf - offset);
        if (result != 0) goto error;
    }

    /* read composite glyphs */
    for (i = 0; i < ttf->nglyphs; i++)
    {
        offset = pp->ploca16 ? pp->ploca16[i] * 2 : pp->ploca32[i];
        if (offset == pp->sglyf) continue;
        hdr = (ttf_glyfh_t *)(pp->pglyf + offset);
        if ((int16_t)big16toh(hdr->numberOfContours) >= 0) continue;
        result = parse_composite_glyph(ttf, ttf->glyphs + i, pp->pglyf + offset, pp->sglyf - offset);
        if (result != 0) goto error;
    }

    return 0;

error:
    return result;
}

static int parse_hmtx_table(ttf_t *ttf, pps_t *pp)
{
    int size, i;
    float adv, lsb;
    uint16_t *p;
    if (pp->shhea < (int)sizeof(ttf_hhea_t)) return TTF_ERR_FMT;
    conv16(pp->phhea->majorVersion);
    conv16(pp->phhea->minorVersion);
    conv16(pp->phhea->ascender);
    conv16(pp->phhea->descender);
    conv16(pp->phhea->lineGap);
    conv16(pp->phhea->advanceWidthMax);
    conv16(pp->phhea->minLeftSideBearing);
    conv16(pp->phhea->minRightSideBearing);
    conv16(pp->phhea->xMaxExtent);
    conv16(pp->phhea->caretSlopeRise);
    conv16(pp->phhea->caretSlopeRun);
    conv16(pp->phhea->caretOffset);
    conv16(pp->phhea->metricDataFormat);
    conv16(pp->phhea->numberOfHMetrics);
    if (pp->phhea->numberOfHMetrics == 0 || (int)pp->phhea->numberOfHMetrics > ttf->nglyphs) return TTF_ERR_FMT;
    size = (int)pp->phhea->numberOfHMetrics * 4 + (ttf->nglyphs - pp->phhea->numberOfHMetrics) * 2;
    if (pp->shmtx != size) return TTF_ERR_FMT;
    p = pp->phmtx;
    for (i = 0; i < (int)pp->phhea->numberOfHMetrics; i++)
    {
        adv = big16toh(*p++);
        lsb = (int16_t)big16toh(*p++);
        ttf->glyphs[i].advance = adv;
        ttf->glyphs[i].lbearing = lsb;
    }
    for (i = 0; i < ttf->nglyphs - (int)pp->phhea->numberOfHMetrics; i++)
    {
        lsb = (int16_t)big16toh(*p++);
        ttf->glyphs[i].advance = adv;
        ttf->glyphs[i].lbearing = lsb;
    }
    return TTF_DONE;
}

static int ttf_extract_tables(const uint8_t *data, int size, pps_t *s)
{
    int ntab;
    ttf_tab_rec_t *rec;

    if ((size_t)size < sizeof(ttf_file_hdr_t)) return TTF_ERR_FMT;

    s->hdr = (ttf_file_hdr_t *)data;
    if (big32toh(s->hdr->sfntVersion) != 0x00010000) return TTF_ERR_VER;
    ntab = big16toh(s->hdr->numTables);

    if (ntab * sizeof(ttf_tab_rec_t) + sizeof(ttf_file_hdr_t) > (size_t)size) return TTF_ERR_FMT;
    rec = (ttf_tab_rec_t *)(s->hdr + 1);

    #define check_tag(str) (*(uint32_t *)rec->tableTag != *(uint32_t *)str)
    #define match(type, name) \
    if (*(uint32_t *)rec->tableTag == *(uint32_t *)#name) \
    { \
        s->s##name = rec->length; \
        s->p##name = (type)(data + rec->offset); \
    }

    while (ntab--)
    {
        conv32(rec->checkSum);
        conv32(rec->offset);
        conv32(rec->length);
        if (rec->offset + rec->length > (unsigned)size) return TTF_ERR_FMT;
        if (check_tag("head"))
            if (ttf_checksum(data + rec->offset, rec->length) != rec->checkSum)
                return TTF_ERR_CSUM;
        match(ttf_cmap_t *, cmap);
        match(ttf_head_t *, head);
        match(ttf_maxp_t *, maxp);
        match(ttf_name_t *, name);
        match(ttf_hhea_t *, hhea); /* TODO: vhea */
        match(uint16_t *, hmtx);
        match(uint8_t *, loca);
        match(uint8_t *, glyf);
        if (check_tag("glyf"))
            s->glyf_csum = rec->checkSum;
        rec++;
    }
    #undef match
    #undef check_tag

    /*
     Required Tables

     TAG     NAME
    'cmap' Character to glyph mapping
    'head' Font header
    'hhea' Horizontal header
    'hmtx' Horizontal metrics
    'maxp' Maximum profile
    'name' Naming table
     OS/2  OS/2 and Windows specific metrics
    'post' PostScript information

    Tables Related to TrueType Outlines

     TAG     NAME
    'cvt ' Control Value Table (optional table)
    'fpgm' Font program (optional table)
    'glyf' Glyph data
    'loca' Index to location
    'prep' CVT Program (optional table)
    'gasp' Grid-fitting/Scan-conversion (optional table)
    */

    if (s->shead == 0 || s->smaxp == 0 || s->sloca == 0 || s->scmap == 0 || s->sglyf == 0 || s->sname == 0 || s->shhea == 0)
        return TTF_ERR_NOTAB;

    return 0;
}

static int parse_fmt4(ttf_t *ttf, uint8_t *data, int dataSize)
{
    ttf_fmt4_t *tab;
    uint16_t *endCode; /* End characterCode for each segment, last=0xFFFF */
    uint16_t *startCode; /* Start character code for each segment */
    int16_t *idDelta; /* Delta for all character codes in segment */
    uint16_t *idRangeOffset; /* Offsets into glyphIdArray or 0 */
    uint16_t *glyphIdArray; /* Glyph index array (arbitrary length) */
    int segCount, idArrayLen;
    int i, j, k;

    if (dataSize < (int)sizeof(ttf_fmt4_t)) return TTF_ERR_FMT;
    tab = (ttf_fmt4_t *)data;
    conv16(tab->format);
    conv16(tab->length);
    conv16(tab->language);
    conv16(tab->segCountX2);
    conv16(tab->searchRange);
    conv16(tab->entrySelector);
    conv16(tab->rangeShift);
    if (tab->length > dataSize)
        return TTF_ERR_FMT;
    segCount = tab->segCountX2 / 2;
    endCode = (uint16_t *)(tab + 1);
    startCode = (uint16_t *)(endCode + segCount + 1);
    idDelta = (int16_t *)(startCode + segCount);
    idRangeOffset = (uint16_t *)(idDelta + segCount);
    glyphIdArray = (uint16_t *)(idRangeOffset + segCount);
    idArrayLen = (tab->length / sizeof(uint16_t) - (glyphIdArray - (uint16_t *)data));
    if (idArrayLen < 0) return TTF_ERR_FMT;

    ttf->nchars = 0;
    for (i = 0; i < segCount; i++)
    {
        conv16(endCode[i]);
        conv16(startCode[i]);
        conv16(idDelta[i]);
        conv16(idRangeOffset[i]);
        if (i == segCount - 1 && startCode[i] != 0xFFFF)
            return TTF_ERR_FMT;
        if (startCode[i] == 0xFFFF) break;
        ttf->nchars += endCode[i] - startCode[i] + 1;
    }

    ttf->chars = (uint16_t *)malloc(sizeof(uint16_t) * 2 * ttf->nchars);
    ttf->char2glyph = ttf->chars + ttf->nchars;
    k = 0;
    for (i = 0; i < segCount; i++)
    {
        if (startCode[i] == 0xFFFF) break;
        for (j = 0; j <= endCode[i] - startCode[i]; j++)
        {
            if (k >= ttf->nchars) return TTF_ERR_FMT; /* internal error? */
            ttf->chars[k] = startCode[i] + j;
            if (idRangeOffset[i] == 0)
            {
                ttf->char2glyph[k] = startCode[i] + j + idDelta[i];
            }
            else
            {
                uint16_t *addr = &idRangeOffset[i] + idRangeOffset[i] / 2 + j;
                if ((uint8_t *)addr + 2 > data + dataSize)
                    return TTF_ERR_FMT;
                ttf->char2glyph[k] = big16toh(*addr);
            }
            k++;
        }
    }
    for (i = 0; i < ttf->nchars; i++)
    {
        j = ttf->char2glyph[i];
        if (j >= ttf->nglyphs)
            return TTF_ERR_FMT;
        ttf->glyphs[j].index = j;
        ttf->glyphs[j].symbol = ttf->chars[i];
    }
    return TTF_DONE;
}

static int parse_cmap(ttf_t *ttf, uint8_t *tab, int tabsize)
{
    int i;
    ttf_cmap_t *cmap;

    cmap = (ttf_cmap_t *)tab;
    if (tabsize < 4) return TTF_ERR_FMT;
    conv16(cmap->version);
    conv16(cmap->numTables);
    if (cmap->version != 0) return TTF_ERR_UTAB;
    if (cmap->numTables * 8 + 4 > tabsize) return TTF_ERR_FMT;
    for (i = 0; i < (int)cmap->numTables; i++)
    {
        uint16_t format;
        conv16(cmap->encRecs[i].platformID);
        conv16(cmap->encRecs[i].encodingID);
        conv32(cmap->encRecs[i].offset);
        if ((int)cmap->encRecs[i].offset + 4 > tabsize) return TTF_ERR_FMT;
        format = *(uint16_t *)(tab + cmap->encRecs[i].offset);
        conv16(format);
        if (format == 4)
            return parse_fmt4(ttf, tab + cmap->encRecs[i].offset,
                              tabsize - cmap->encRecs[i].offset);
    }
    return TTF_ERR_UTAB;
}

static const char *empty_string = "";

static const char *namerec2ascii(const char *p, int len, int platformID, int encodingID, int languageID)
{
    /*
        platformID      1      - Macintosh
            encodingID  0      - Roman

        platformID      3      - Windows
            encodingID  1      - Unicode BMP
            languageID  0x0409 - United States
    */
    if (platformID == 1 && encodingID == 0)
    {
        char *res = (char *)malloc(len + 1);
        memcpy(res, p, len);
        res[len] = 0;
        return res;
    }
    if (platformID == 3 && encodingID == 1 && languageID == 0x0409) /* required */
    {
        int i;
        len = len / 2;
        char *res = (char *)malloc(len + 1);
        for (i = 0; i < len; i++)
            res[i] = p[i * 2 + 1];
        res[len] = 0;
        return res;
    }
    return empty_string;
}

static bool parse_name(ttf_t *ttf, uint8_t *tab, int tabsize)
{
    int i;
    ttf_name_t *hdr;

    hdr = (ttf_name_t *)tab;
    conv16(hdr->format);
    conv16(hdr->count);
    conv16(hdr->stringOffset);
    if (hdr->format != 0 && hdr->format != 1) return false;
    if (hdr->count * (int)sizeof(hdr->nameRecord[0]) + 6 > tabsize) return false;
    ttf->info.copyright = empty_string;
    ttf->info.family = empty_string;
    ttf->info.subfamily = empty_string;
    ttf->info.unique_id = empty_string;
    ttf->info.full_name = empty_string;
    ttf->info.version = empty_string;
    ttf->info.ps_name = empty_string;
    ttf->info.trademark = empty_string;
    ttf->info.manufacturer = empty_string;
    ttf->info.designer = empty_string;
    ttf->info.description = empty_string;
    ttf->info.url_vendor = empty_string;
    ttf->info.url_designer = empty_string;
    ttf->info.license_desc = empty_string;
    ttf->info.locense_url = empty_string;
    ttf->info.sample_text = empty_string;
    for (i = 0; i < hdr->count; i++)
    {
        char *s;
        conv16(hdr->nameRecord[i].encodingID);
        conv16(hdr->nameRecord[i].languageID);
        conv16(hdr->nameRecord[i].length);
        conv16(hdr->nameRecord[i].nameID);
        conv16(hdr->nameRecord[i].offset);
        conv16(hdr->nameRecord[i].platformID);
        if (hdr->stringOffset +
            hdr->nameRecord[i].offset +
            hdr->nameRecord[i].length > tabsize) return false;
        s = (char *)tab + hdr->stringOffset + hdr->nameRecord[i].offset;
        #define match(id, field) \
        if (hdr->nameRecord[i].nameID == id && ttf->info.field == empty_string) \
            ttf->info.field = namerec2ascii(s, hdr->nameRecord[i].length, \
                                            hdr->nameRecord[i].platformID, \
                                            hdr->nameRecord[i].encodingID, \
                                            hdr->nameRecord[i].languageID)
        match(0, copyright);
        match(1, family);
        match(2, subfamily);
        match(3, unique_id);
        match(4, full_name);
        match(5, version);
        match(6, ps_name);
        match(7, trademark);
        match(8, manufacturer);
        match(9, designer);
        match(10, description);
        match(11, url_vendor);
        match(12, url_designer);
        match(13, license_desc);
        match(14, locense_url);
        match(19, sample_text);
    }
    return true;
}

ttf_t *allocate_ttf_structure(int nglyphs, bool headers_only)
{
    ttf_t *res;
    if (headers_only)
    {
        res = calloc(sizeof(ttf_t), 1);
        if (res == NULL) return NULL;
        return res;
    }
    res = calloc(sizeof(ttf_t) + nglyphs * sizeof(ttf_glyph_t), 1);
    if (res == NULL) return NULL;
    res->nglyphs = nglyphs;
    res->glyphs = (ttf_glyph_t *)(res + 1);
    return res;
}

void ttf_prepare_to_output(ttf_t *ttf, pps_t *pps)
{
    int i, j, k;
    ttf_glyph_t *g;
    ttf_point_t *p;
    float scale;
    scale = 1.0f / big16toh(pps->phead->unitsPerEm);
    for (i = 0; i < ttf->nglyphs; i++)
    {
        g = ttf->glyphs + i;
        g->xbounds[0] *= scale;
        g->xbounds[1] *= scale;
        g->ybounds[0] *= scale;
        g->ybounds[1] *= scale;
        g->advance *= scale;
        g->lbearing *= scale;
        g->rbearing = g->advance - (g->lbearing + g->xbounds[1] - g->xbounds[0]);
        for (j = 0; j < g->ncontours; j++)
        {
            p = g->outline->cont[j].pt;
            for (k = 0; k < g->outline->cont[j].length; k++)
            {
                p[k].x *= scale;
                p[k].y *= scale;
            }
        }
    }
}

int ttf_load_from_mem(const uint8_t *data, int size, ttf_t **output, bool headers_only)
{
    int result;
    ttf_t *ttf;
    pps_t s;
    int i;

    ttf = NULL;

    /* check entire font checksum */
    check(ttf_checksum(data, size) == 0xB1B0AFBA, TTF_ERR_CSUM);

    /* extract top level tables */
    memset(&s, 0, sizeof(pps_t));
    result = ttf_extract_tables(data, size, &s);
    if (result != 0) goto error;

    /* check head table */
    check(s.shead == sizeof(ttf_head_t), TTF_ERR_FMT);
    check(big32toh(((ttf_head_t *)s.phead)->magicNumber) == 0x5F0F3CF5, TTF_ERR_FMT);

    /* check maxp table */
    check(s.smaxp >= 6, TTF_ERR_FMT);
    check(big16toh(((ttf_maxp_t *)s.pmaxp)->verMaj) <= 1, TTF_ERR_UTAB);

    /* allocate ttf structure */
    ttf = allocate_ttf_structure(big16toh(((ttf_maxp_t *)s.pmaxp)->numGlyphs), headers_only);
    check(ttf != NULL, TTF_ERR_NOMEM);

    /* check name table */
    check(s.sname >= (int)sizeof(ttf_name_t), TTF_ERR_FMT);
    check(parse_name(ttf, (uint8_t *)s.pname, s.sname), TTF_ERR_FMT);

    if (!headers_only)
    {
        /* check and parse cmap table */
        result = parse_cmap(ttf, (uint8_t *)s.pcmap, s.scmap);
        if (result != TTF_DONE) goto error;

        /* check and convert loca table */
        check(big16toh(s.phead->indexToLocFormat) <= 1, TTF_ERR_FMT);
        if (s.phead->indexToLocFormat == 0)
        {
            check(s.sloca >= ttf->nglyphs * 2, TTF_ERR_FMT);
            s.ploca16 = (uint16_t *)s.ploca;
            for (i = 0; i < ttf->nglyphs; i++)
                conv16(s.ploca16[i]);
        }
        else
        {
            check(s.sloca >= ttf->nglyphs * 4, TTF_ERR_FMT);
            s.ploca32 = (uint32_t *)s.ploca;
            for (i = 0; i < ttf->nglyphs; i++)
                conv32(s.ploca32[i]);
        }

        /* reading the glyph data */
        result = parse_glyf_table(ttf, &s);
        if (result != TTF_DONE) goto error;

        result = parse_hmtx_table(ttf, &s);
        if (result != TTF_DONE) goto error;
    }

    /* prepare to output */
    ttf->filename = empty_string;
    ttf->glyf_csum = s.glyf_csum;
    ttf_prepare_to_output(ttf, &s);

    *output = ttf;
    return TTF_DONE;

error:
    ttf_free(ttf);
    *output = NULL;
    return result;
}

static bool try_strdup(const char *s, char **dest)
{
    char *res;
    int len = strlen(s);
    res = (char *)malloc(len + 1);
    if (res == NULL) return false;
    memcpy(res, s, len);
    res[len] = 0;
    *dest = res;
    return true;
}

int ttf_load_from_file(const char *filename, ttf_t **output, bool headers_only)
{
    FILE *f;
    int result;
    uint8_t *data;
    int size;
    uint32_t sfntVersion; /* 0x00010000 or 0x4F54544F ('OTTO') */

    data = NULL;
    *output = NULL;

    /* open file and get it size */
    f = fopen(filename, "rb");
    check(f != NULL, TTF_ERR_OPEN);
    check(fread(&sfntVersion, 1, 4, f) == 4, TTF_ERR_FMT);
    check(big32toh(sfntVersion) == 0x00010000, TTF_ERR_FMT);
    check(fseek(f, 0, SEEK_END) == 0, TTF_ERR_FMT);
    size = ftell(f);
    check(size > 0 && size < (TTF_MAX_FILE * 1024 * 1024), TTF_ERR_SIZE);
    check(fseek(f, 0, SEEK_SET) == 0, TTF_ERR_FMT);

    /* allocate memory to file content */
    data = malloc(size);
    check(data != NULL, TTF_ERR_NOMEM);

    /* read file content */
    check(fread(data, 1, size, f) == (size_t)size, TTF_ERR_FMT);

    fclose(f);
    result = ttf_load_from_mem(data, size, output, headers_only);
    free(data);

    if (*output != NULL)
        try_strdup(filename, (char **)&(*output)->filename);

    return result;

error:
    free(data);
    if (f != NULL)
        fclose(f);
    return result;
}

#ifndef TTF_WINDOWS
static void replace_tilda_to_home_path(char path[PATH_MAX])
{
    const char *homedir;
    int hlen, plen;
    if (path[0] != '~') return;
    if ((homedir = getenv("HOME")) == NULL) return;
    hlen = strlen(homedir);
    plen = strlen(path);
    if (hlen + plen > PATH_MAX) return;
    memmove(path + hlen, path + 1, plen); /* and termination null too */
    memcpy(path, homedir, hlen);
}
#endif

static bool make_full_path(char *fullpath, const char *part)
{
    int dlen, flen;
    flen = strlen(fullpath);
    dlen = strlen(part);
    if (flen == 0)
    {
        memcpy(fullpath, part, dlen + 1);
#ifndef TTF_WINDOWS
        replace_tilda_to_home_path(fullpath);
#endif
        return true;
    }
    if (fullpath[flen - 1] != PATH_SEP)
    {
        if (flen + dlen + 2 > PATH_MAX) return false;
        fullpath[flen] = PATH_SEP;
        memcpy(fullpath + flen + 1, part, dlen + 1);
        return true;
    }
    if (flen + dlen + 1 > PATH_MAX) return false;
    memcpy(fullpath + flen, part, dlen + 1);
    return true;
}

static bool check_font_ext(const char *file_name)
{
    int len = strlen(file_name);
    if (len < 4) return false;
    file_name += len - 4;
    return
        (file_name[0] == '.') &&
        (file_name[1] == 'T' || file_name[1] == 't') &&
        (file_name[2] == 'T' || file_name[2] == 't') &&
        (file_name[3] == 'F' || file_name[3] == 'f');
}

#if defined(TTF_LINUX)
static ttf_t **load_fonts_from_dir(ttf_t **list, int *count, int *cap, const char *dir, char *fullpath, int deepmax)
{
    DIR *d;
    struct dirent *entry;
    int flen;

    flen = strlen(fullpath);
    if (!make_full_path(fullpath, dir)) return list;

    d = opendir(fullpath);
    if (d != NULL)
        while ((entry = readdir(d)))
        {
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) continue;
            if ((entry->d_type & DT_DIR) != 0)
            {
                if (deepmax > 0)
                    list = load_fonts_from_dir(list, count, cap, entry->d_name, fullpath, deepmax - 1);
            }
            else
                if ((entry->d_type & DT_REG))
                {
                    ttf_t *font;
                    int old_len = strlen(fullpath);
                    if (!check_font_ext(entry->d_name)) continue;
                    if (!make_full_path(fullpath, entry->d_name)) continue;
                    ttf_load_from_file(fullpath, &font, true);
                    fullpath[old_len] = 0;
                    if (font == NULL)
                        continue;
                    if (*count == *cap - 1)
                    {
                        ttf_t **tmp;
                        *cap *= 2;
                        tmp = (ttf_t **)realloc(list, sizeof(ttf_t *) * *cap);
                        if (tmp == NULL) break;
                        list = tmp;
                    }
                    list[*count] = font;
                    *count += 1;
                }
        }

    if (d != NULL)
        closedir(d);

    fullpath[flen] = 0;

    return list;
}
#elif defined(TTF_WINDOWS)
static ttf_t **load_fonts_from_dir(ttf_t **list, int *count, int *cap, const char *dir, char *fullpath, int deepmax)
{
    HANDLE hfind;
    WIN32_FIND_DATAA entry;
    int flen;

    flen = strlen(fullpath);
    if (!make_full_path(fullpath, dir)) return list;
    if (!make_full_path(fullpath, "*")) return list;

    hfind = FindFirstFileA(fullpath, &entry);
    fullpath[strlen(fullpath) - 2] = 0; /* remove "\*" tail */
    if (hfind == INVALID_HANDLE_VALUE) return list;
    do
    {
        if (strcmp(entry.cFileName, ".") == 0 || strcmp(entry.cFileName, "..") == 0) continue;
        if (entry.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        {
            if (deepmax > 0)
                list = load_fonts_from_dir(list, count, cap, entry.cFileName, fullpath, deepmax - 1);
        }
        else
        {
            ttf_t *font;
            int old_len = strlen(fullpath);
            if (!check_font_ext(entry.cFileName)) continue;
            if (!make_full_path(fullpath, entry.cFileName)) continue;
            ttf_load_from_file(fullpath, &font, true);
            fullpath[old_len] = 0;
            if (font == NULL)
                continue;
            if (*count == *cap - 1)
            {
                ttf_t **tmp;
                *cap *= 2;
                tmp = (ttf_t **)realloc(list, sizeof(ttf_t *) * *cap);
                if (tmp == NULL) break;
                list = tmp;
            }
            list[*count] = font;
            *count += 1;
        }
    } while (FindNextFileA(hfind, &entry) != 0);

    FindClose(hfind);

    fullpath[flen] = 0;

    return list;
}
#endif

static int font_list_sorting(const void *a, const void *b)
{
    const ttf_t *A, *B;
    A = *(const ttf_t **)a;
    B = *(const ttf_t **)b;
    return strcmp(A->info.full_name, B->info.full_name);
}

ttf_t **ttf_list_fonts(const char **directories, int dir_count)
{
    ttf_t **res;
    int count, cap, i, n;
    char fullpath[PATH_MAX];

    if (directories == NULL || dir_count <= 0) return NULL;

    count = 0;
    cap = 64;
    fullpath[0] = 0;

    res = (ttf_t **)malloc(sizeof(ttf_t *) * cap);
    if (res == NULL) return NULL;

    for (i = 0; i < dir_count; i++)
        res = load_fonts_from_dir(res, &count, &cap, directories[i], fullpath, 5);

    /* sorting list by font full_name field */
    qsort(res, count, sizeof(ttf_t *), font_list_sorting);

    /* removing duplicates from the list */
    n = 0;
    for (i = 0; i < count; i++)
    {
        if (i != count - 1)
            if (res[i]->glyf_csum == res[i + 1]->glyf_csum)
                if (strcmp(res[i]->info.full_name, res[i + 1]->info.full_name) == 0)
                {
                    ttf_free(res[i]);
                    continue;
                }
        res[n++] = res[i];
    }
    res[n] = NULL;

    return res;
}

ttf_t **ttf_list_system_fonts(void)
{
    static const char *directories[] = {
#if defined(TTF_LINUX)
        LINUX_FONTS_PATH
#elif defined(TTF_WINDOWS)
        WINDOWS_FONTS_PATH
#endif
    };
    int dir_count = sizeof(directories) / sizeof(char *);
    return ttf_list_fonts(directories, dir_count);
}

int ttf_find_glyph(const ttf_t *ttf, uint16_t utf16)
{
    int i;
    /* todo: half division! */
    for (i = 0; i < ttf->nchars; i++)
        if (utf16 == ttf->chars[i])
            return ttf->char2glyph[i];
    return -1;
}

/*
static int linearize_qbezier(ttf_point_t curve[3], ttf_point_t *dst, uint8_t quality)
{
    int i;
    for (i = 0; i < quality && dst; i++)
    {
        float step, t, tt;
        step = 1.0f / ((int)quality + 1);
        t = step * (i + 1);
        tt = 1.0f - t;

        dst[i].x = tt * tt * curve[0].x +
                   2.0f * t * tt * curve[1].x +
                   t * t * curve[2].x;

        dst[i].y = tt * tt * curve[0].y +
                   2.0f * t * tt * curve[1].y +
                   t * t * curve[2].y;
    }
    return quality;
}
*/

/**
 * @brief Расчёт координаты кривой Безье
 * @param p0
 * @param p1
 * @param p2
 * @param t
 * @return
 */
static __inline float qbezier(float p0, float p1, float p2, float t)
{
    float tt = 1.0f - t;
    return tt * tt * p0 + 2.0f * t * tt * p1 + t * t * p2;
}

/**
 * @brief Первая производная функции Безье
 * @param p0 Опорная точка 0
 * @param p1 Опорная точка 1
 * @param p2 Опорная точка 2
 * @param t Параметр кривой
 * @return Значение производной
 */
static __inline float qbezier_diff1(float p0, float p1, float p2, float t)
{
    return 2.0f * (t * (p0 - 2.0f * p1 + p2) - p0 + p1);
}

static int linearize_qbezier(ttf_point_t curve[3], ttf_point_t *dst, uint8_t quality)
{
    int i, res;
    float v1[2], v2[2], angle, step;

    /* Найдём косательные к кривой в точке t=0 и t=1 */
    v1[0] = qbezier_diff1(curve[0].x, curve[1].x, curve[2].x, 0.0f);
    v1[1] = qbezier_diff1(curve[0].y, curve[1].y, curve[2].y, 0.0f);
    v2[0] = qbezier_diff1(curve[0].x, curve[1].x, curve[2].x, 1.0f);
    v2[1] = qbezier_diff1(curve[0].y, curve[1].y, curve[2].y, 1.0f);

    /* Найдём угол между косательными векторами */
    angle = fabsf(VECCROSS(v1, v2));
    if (angle < EPSILON) return 0;
    angle /= VECLEN(v1);
    angle /= VECLEN(v2);
    if (angle >= 1.0f) angle = 1.0f;
    angle = asinf(angle);

    /* Найдём число точек, которые следует расставить в контуре */
    res = lroundf(angle / (float)(pi * 2) * quality);
    if (res == 0) return 0;
    if (dst == 0) return res;

    /* Расставляем точки равноудалённо */
    step = 1.0f / (res + 1);
    for (i = 0; i < res; i++)
    {
        float t = step * (i + 1);
        dst[i].x = qbezier(curve[0].x, curve[1].x, curve[2].x, t);
        dst[i].y = qbezier(curve[0].y, curve[1].y, curve[2].y, t);
    }

    return res;
}

static __inline float herons_area(float a, float b, float c)
{
    float p = (a + b + c) / 2.0f;
    return sqrtf(p * (p - a) * (p - b) * (p - c));
}

static __inline float herons_area_v(const float v1[2], const float v2[2], const float v3[2])
{
    float e1[2], e2[2], e3[2];
    VECSUB(e1, v1, v2);
    VECSUB(e2, v2, v3);
    VECSUB(e3, v3, v1);
    return herons_area(VECLEN(e1), VECLEN(e2), VECLEN(e3));
}

static __inline float herons_area_p(const ttf_point_t *a, const ttf_point_t *b, const ttf_point_t *c)
{
    return herons_area_v(&a->x, &b->x, &c->x);
}

/* TODO: simplify! */
static int linearize_contour(ttf_point_t *src, ttf_point_t *dst, int src_count, uint8_t quality)
{
    int i, state, res;
    ttf_point_t queue[3];

    state = 0;
    res = 0;
    for (i = 0; i < src_count; i++)
        switch (state)
        {
        case 0:
            queue[0] = src[0];
            if (dst) dst[0] = src[0];
            state = 1;
            res = 1;
            break;
        case 1:
            if (src[i].onc)
            {
                /* TODO: check edge length */
                if (dst) dst[res] = src[i];
                queue[0] = src[i];
                state = 1;
                res++;
            }
            else
            {
                queue[1] = src[i];
                state = 2;
            }
            break;
        case 2:
            if (src[i].onc)
            {
                queue[2] = src[i];
                if (herons_area_p(queue + 0, queue + 1, queue + 2) > 1e-5)
                    res += linearize_qbezier(queue, dst ? dst + res : NULL, quality);
                if (dst) dst[res] = src[i];
                res++;
                queue[0] = src[i];
                state = 1;
            }
            else
            {
                queue[2].x = (queue[1].x + src[i].x) / 2;
                queue[2].y = (queue[1].y + src[i].y) / 2;
                if (herons_area_p(queue + 0, queue + 1, queue + 2) > 1e-5)
                {
                    res += linearize_qbezier(queue, dst ? dst + res : NULL, quality);
                    if (dst) dst[res] = queue[2];
                    res++;
                    queue[0] = queue[2];
                    queue[1] = src[i];
                    state = 2;
                }
                else
                {
                    queue[1] = queue[2];
                    state = 2;
                }
            }
            break;
        }

    if (state == 2)
    {
        queue[2] = src[0];
        if (herons_area_p(queue + 0, queue + 1, queue + 2) > 1e-5)
            res += linearize_qbezier(queue, dst ? dst + res : NULL, quality);
    }

    return res;
}

static int ttf_fix_linear_bags(ttf_point_t *pt, int count)
{
    int i, n;
    if (count < 3) return 0;
    n = 1;
    for (i = 1; i < count - 1; i++)
        if (herons_area_p(pt + n - 1, pt + i, pt + i + 1) > EPSILON)
            pt[n++] = pt[i];
    pt[n++] = pt[count - 1];
    while (n > 1)
    {
        float dx, dy;
        dx = pt->x - pt[n - 1].x;
        dy = pt->y - pt[n - 1].y;
        if (fabsf(dx) > EPSILON || fabsf(dy) > EPSILON) break;
        n--;
    }
    return n >= 3 ? n : 0;
}

ttf_outline_t *ttf_linear_outline(const ttf_glyph_t *glyph, uint8_t quality)
{
    int i, npoints;
    ttf_outline_t *o, *s;

    o = glyph->outline;
    if (o == NULL) return NULL;

    npoints = 0;
    for (i = 0; i < o->ncontours; i++)
        npoints += linearize_contour(o->cont[i].pt, NULL, o->cont[i].length, quality);

    s = allocate_ttf_outline(o->ncontours, npoints);
    if (s == NULL) return NULL;

    s->total_points = 0;
    for (i = 0; i < o->ncontours; i++)
    {
        npoints = linearize_contour(o->cont[i].pt, s->cont[i].pt, o->cont[i].length, quality);
        npoints = ttf_fix_linear_bags(s->cont[i].pt, npoints);
        if (i != o->ncontours - 1)
            s->cont[i + 1].pt = s->cont[i].pt + npoints;
        s->cont[i].length = npoints;
        s->cont[i].subglyph_id = o->cont[i].subglyph_id;
        s->cont[i].subglyph_order = o->cont[i].subglyph_order;
        s->total_points += npoints;
    }

    return s;
}

static int split_qbezier_contour(ttf_point_t *src, ttf_point_t *dst, int len)
{
    int i, j, n;
    n = 0;
    j = 0;
    for (i = 0; i < len; i++)
        switch (n)
        {
        case 0:
            dst[j++] = src[i];
            n = 1;
            break;
        case 1:
            dst[j++] = src[i];
            n = src[i].onc ? 1 : 2;
            break;
        case 2:
            if (src[i].onc)
            {
                dst[j++] = src[i];
                n = 1;
                break;
            }
            dst[j].onc = 1;
            dst[j].spl = 1;
            dst[j].x = (dst[j - 1].x + src[i].x) / 2;
            dst[j].y = (dst[j - 1].y + src[i].y) / 2;
            dst[j + 1] = src[i];
            j += 2;
            n = 2;
            break;
        }
    return j;
}

ttf_outline_t *ttf_splitted_outline(const ttf_glyph_t *glyph)
{
    int i, npoints;
    ttf_outline_t *o, *s;

    o = glyph->outline;
    if (o == NULL) return NULL;

    npoints = 0;
    for (i = 0; i < o->ncontours; i++)
        npoints += o->cont[i].length;

    s = allocate_ttf_outline(o->ncontours, npoints * 2);
    if (s == NULL) return NULL;

    s->total_points = 0;
    for (i = 0; i < s->ncontours; i++)
    {
        npoints = split_qbezier_contour(o->cont[i].pt, s->cont[i].pt, o->cont[i].length);
        s->cont[i].length = npoints;
        s->total_points += npoints;
        s->cont[i].subglyph_id = o->cont[i].subglyph_id;
        s->cont[i].subglyph_order = o->cont[i].subglyph_order;
        if (i != s->ncontours - 1)
            s->cont[i + 1].pt = s->cont[i].pt + npoints;
    }

    return s;
}

static int ttf_glyph2svgpath_impl(ttf_outline_t *o, char *s, int len)
{
    char tmp;
    int res, n, i, j;

    #define FIX() if (len <= 0) { s = &tmp; len = 1; }
    #define APPLY() { s += n; len -= n; res += n; FIX(); }

    res = 0;

    FIX();
    n = snprintf(s, len, "<path style=\"fill-rule:nonzero\" d=\""); APPLY();

    for (i = 0; i < o->ncontours; i++)
    {
        int clen;
        ttf_point_t *p;

        clen = o->cont[i].length;
        p = o->cont[i].pt;
        j = 0;
        n = snprintf(s, len, "M %.3f %.3f ", p->x, -p->y); APPLY();
        while (j < clen)
        {
            if (j == clen - 1) break;
            if (p[j + 1].onc)
            {
                j++;
                n = snprintf(s, len, "L %.3f %.3f ", p[j].x, -p[j].y); APPLY();
                continue;
            }
            if (j == clen - 2)
            {
                j++;
                n = snprintf(s, len, "Q %.3f,%.3f %.3f,%.3f ", p[j].x, -p[j].y, p[0].x, -p[0].y); APPLY();
                break;
            }
            else
            {
                j++;
                n = snprintf(s, len, "Q %.3f,%.3f %.3f,%.3f ", p[j].x, -p[j].y, p[j + 1].x, -p[j + 1].y); APPLY();
                j++;
            }
        }
    }
    n = snprintf(s, len, "\"/>"); APPLY();
    #undef FIX
    #undef APPLY
    return res;
}

int ttf_outline_evenodd_base(const ttf_outline_t *outline, const float point[2], int contour, float *dist)
{
    int counter = 0;
    float closest_dx = 0;

    const ttf_point_t *pt = outline->cont[contour].pt;
    int len = outline->cont[contour].length;
    const ttf_point_t *prev = pt + len - 1;
    for (int i = 0; i < len; i++)
    {
        const ttf_point_t *u, *b;
        if (pt->y > prev->y)
        {
            u = pt;
            b = prev;
        }
        else
        {
            u = prev;
            b = pt;
        }
        if (point[1] <= u->y && point[1] > b->y)
            if (point[0] >= u->x || point[0] >= b->x)
            {
                /*
                           u
                     edge /
                         / <-- dx --> .
                        /            point
                       b

                calculation of distance from edge to point:
                y(x) = b.y + (u.y - b.y) / (u.x - b.x) * (x - b.x)    - the edge line equation
                x(y) = (y - b.y) / (u.y - b.y) * (u.x - b.x) + b.x    - inverse equation of line
                dx = point.x - x(point.y)                             - final equation for distance
                */
                float dy = u->y - b->y;
                if (fabsf(dy) > EPSILON) /* horizontal edges are not handling */
                {
                    float dx = point[0] - (point[1] - b->y) / dy * (u->x - b->x) - b->x;
                    if (dx >= 0)
                    {
                        if (counter == 0 || dx < closest_dx)
                            closest_dx = dx;
                        counter++;
                    }
                }
            }
        prev = pt++;
    }
    if (dist != NULL)
        *dist = closest_dx;
    return counter;
}

bool ttf_outline_evenodd(const ttf_outline_t *outline, const float point[2], int subglyph_order)
{
    int count = 0;
    for (int i = 0; i < outline->ncontours; i++)
    {
        if (subglyph_order >= 0 && outline->cont[i].subglyph_order != subglyph_order) continue;
        count += ttf_outline_evenodd_base(outline, point, i, NULL);
    }
    return (count & 1) == 1;
}

bool ttf_outline_contour_info(const ttf_outline_t *outline, int subglyph_order, int contour, int test_point, int *nested_to)
{
    int count = 0;
    int nto = -1;
    float closest = 0;
    for (int i = 0; i < outline->ncontours; i++)
    {
        if (i == contour) continue;
        if (subglyph_order >= 0 && outline->cont[i].subglyph_order != subglyph_order) continue;
        float dist;
        int res = ttf_outline_evenodd_base(outline, &outline->cont[contour].pt[test_point].x, i, &dist);
        count += res;
        if ((res & 1) == 0) continue;
        if (nto == -1 || dist < closest)
        {
            closest = dist;
            nto = i;
        }
    }
    if (nested_to != NULL)
        *nested_to = nto;
    return (count & 1) == 0;
}

char *ttf_glyph2svgpath(ttf_glyph_t *glyph)
{
    int len;
    char *res;
    ttf_outline_t *o = ttf_splitted_outline(glyph);
    if (o == NULL) return NULL;
    len = ttf_glyph2svgpath_impl(o, NULL, 0);
    if (len <= 0)
    {
        ttf_free_outline(o);
        return NULL;
    }
    res = (char *)malloc(len + 1);
    ttf_glyph2svgpath_impl(o, res, len + 1);
    res[len] = 0;
    ttf_free_outline(o);
    return res;
}

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
/*********************************** MESHER ***********************************/
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

/* mesher definitions */

#define MESHER_DONE 0 /* Мешер успешно отработал */
#define MESHER_WARN 1 /* Не все рёбра на контуре легли в треугольники */
#define MESHER_FAIL 2 /* Критическая ошибка (данных или алгоритма) */
#define MESHER_TRAP 3 /* Мешер завершил работу по требованию отладки */

#define EDGE_HAS_VERT(e, v)        ((e)->v1 == (v) || (e)->v2 == (v))
#define EDGE_SECOND_VERT(e, v)     ((e)->v1 == v ? (e)->v2 : (e)->v1)
#define EDGES_CONNECTED(e1, e2)    (EDGE_HAS_VERT(e1, (e2)->v1) || EDGE_HAS_VERT(e1, (e2)->v2))
#define EDGES_COMMON_VERT(e1, e2)  ((e1)->v1 == (e2)->v1 || (e1)->v1 == (e2)->v2 ? (e1)->v1 : (e1)->v2)
#define TRI_HAS_EDGE(t, e)         ((t)->edge[0] == e || (t)->edge[1] == e || (t)->edge[2] == e)
#define TRI_SECOND_EDGE(t, first)  ((t)->edge[0] == (first) ? (t)->edge[1] : (t)->edge[0])
#define TRI_THIRD_EDGE(t, first)   ((t)->edge[2] == (first) ? (t)->edge[1] : (t)->edge[2])
#define TRI_FIRST_VERT(t)          (EDGES_COMMON_VERT((t)->edge[0], (t)->edge[1]))
#define TRI_SECOND_VERT(t)         (EDGES_COMMON_VERT((t)->edge[1], (t)->edge[2]))
#define TRI_THIRD_VERT(t)          (EDGES_COMMON_VERT((t)->edge[2], (t)->edge[0]))
#define OPPOSITE_EDGE(t, v)        (EDGE_HAS_VERT((t)->edge[0], v) ? (EDGE_HAS_VERT((t)->edge[1], v) ? (t)->edge[2] : (t)->edge[1]) : (t)->edge[0])
#define OPPOSITE_VERT(t, e)        (EDGES_COMMON_VERT(TRI_SECOND_EDGE(t, e), TRI_THIRD_EDGE(t, e)))
#define EDGE1_OF_VERT(t, v)        TRI_SECOND_EDGE(t, OPPOSITE_EDGE(t, v))
#define EDGE2_OF_VERT(t, v)        TRI_THIRD_EDGE(t, OPPOSITE_EDGE(t, v))
#define IS_CONTOUR_EDGE(e)         ((e)->v1->prev_in_contour == (e)->v2 || (e)->v2->prev_in_contour == (e)->v1)

/* mesher types */

typedef struct mesher_vert_to_edge_link v2e_t;
typedef struct mesher_vertex_struct     mvs_t;
typedef struct mesher_circumcircle      mcc_t;
typedef struct mesher_edge_struct       mes_t;
typedef struct mesher_triangle_struct   mts_t;

struct mesher_vert_to_edge_link
{
    v2e_t *next;
    v2e_t *prev;
    mes_t *edge;
};

struct mesher_vertex_struct
{
    float x;
    float y;
    int contour;
    int subglyph;
    bool is_hole;
    int nested_to;
    int object;
    v2e_t edges;
    mvs_t *next_in_contour;
    mvs_t *prev_in_contour;
    int index;
};

struct mesher_circumcircle
{
    float radius;
    float center[2];
};

struct mesher_edge_struct
{
    mes_t *next;
    mes_t *prev;
    mvs_t *v1;
    mvs_t *v2;
    mts_t *tr[2];
    mcc_t alt_cc[2];
    int index;
};

struct mesher_triangle_struct
{
    mts_t *next; /* Следующий треугольник в списке */
    mts_t *prev; /* Предыдущий треугольник в списке */
    mes_t *edge[3]; /* Ссылки на 3 образующих ребра */
    mcc_t cc;
    int helper; /* Поле для разных задач */
};

typedef struct
{
    int maxv;  /* Максимальное число вершин */
    int maxe;  /* Максимальное число рёбер */
    int maxt;  /* Максимальное число треугольников */
    int nv;    /* Фактическое число вершин в v (может быть меньше maxv) */
    mvs_t *v; /* Пул вершин, массив длиной maxv */
    mes_t *e; /* Пул рёбер, массив длиной maxe */
    mts_t *t; /* Пул треугольников, массив длиной maxt */
    v2e_t *l; /* Пул ссылок вершина->ребро, массив длиной (2 * maxe) */
    mes_t efree; /* Корень списка свободных рёбер */
    mes_t eused; /* Корень списка использованных рёбер */
    mts_t tfree; /* Корень списка свободных треугольников */
    mts_t tused; /* Корень списка использованных треугольников */
    v2e_t lfree; /* Корень списка свободных ссылок вершина->ребро */
    mes_t convx; /* Корень списка рёбер в выпуклой оболочке */
    mvs_t vinit[2]; /* Две инициализационные точки по нижней границе глифа */
    mvs_t **s; /* Сортированный по (y) массив вершин */
    struct /* Отладочные поля */
    {
        int curr_step; /* Текущий шаг алгоритма */
        char message[64]; /* Текстовое сообщение при отладке или ошибке */
        int stop_at_step; /* Шаг, на котором следует прервать работу алгоритма */
        bool breakpoint; /* Если заполнено true, то будет сформирован SIGINT */
    } debug;
} mesher_t;

/* Определяет максимальное число треугольников в */
/* триангуляции по числу точек в ней (maxv) */
#define MAXV_TO_MAXT(maxv)  ((maxv - 3) * 2 + 1)

/* Определяет максимальное число рёбер в */
/* триангуляции по числу треугольников в ней (maxt) */
#define MAXT_TO_MAXE(maxt)  (maxt * 2 + 1)

int mvs_sorting_fn(const void *p1, const void *p2)
{
    mvs_t *v1 = *(mvs_t **)p1;
    mvs_t *v2 = *(mvs_t **)p2;
    if (v1->y == v2->y)
    {
        if (v1->x == v2->x) return 0;
        return v1->x > v2->x ? 1 : -1;
    }
    return v1->y > v2->y ? 1 : -1;
}

static bool compare_contours(const ttf_outline_t *o, int i1, int i2)
{
    if (o->cont[i1].length != o->cont[i2].length) return false;
    for (int i = 0; i < o->cont[i1].length; i++)
    {
        float dx = o->cont[i1].pt[i].x - o->cont[i2].pt[i].x;
        float dy = o->cont[i1].pt[i].y - o->cont[i2].pt[i].y;
        if (fabsf(dx) > EPSILON || fabsf(dy) > EPSILON) return false;
    }
    return true;
}

static bool ttf_outline_contour_info_majority(const ttf_outline_t *outline, int subglyph_order, int contour, int *nested_to)
{
    bool cont[3];
    int nto[3];
    int step = outline->cont[contour].length / 3;
    cont[0] = ttf_outline_contour_info(outline, subglyph_order, contour, step * 0, nto + 0);
    cont[1] = ttf_outline_contour_info(outline, subglyph_order, contour, step * 1, nto + 1);
    cont[2] = ttf_outline_contour_info(outline, subglyph_order, contour, step * 2, nto + 2);
    int index = -1;
    if (nto[0] == -1) index = 0;
    if (nto[1] == -1) index = 1;
    if (nto[2] == -1) index = 2;
    if (index == -1)
    {
        if (nto[0] == nto[1] || nto[0] == nto[2])
            index = 0; else
            index = 1;
    }
    *nested_to = nto[index];
    return cont[index];
}

mesher_t *create_mesher(const ttf_outline_t *o)
{
    int maxv, maxt, maxe, maxv2e;
    mesher_t *m;

    /* Выделяем память и инициализируем поля */

    maxv = o->total_points;
    maxt = MAXV_TO_MAXT(maxv + 2); /* Две вершины - инициализационные */
    maxe = MAXT_TO_MAXE(maxt);
    maxv2e = maxe * 2;
    int size =
        sizeof(mesher_t) +
        sizeof(mvs_t) * maxv +   /* v */
        sizeof(mes_t) * maxe +   /* e */
        sizeof(v2e_t) * maxv2e + /* l */
        sizeof(mts_t) * maxt +   /* t */
        sizeof(mvs_t *) * maxv;  /* s */
    m = (mesher_t *)calloc(size, 1);
    if (m == NULL) return NULL;
    m->v = (mvs_t *)(m + 1);
    m->e = (mes_t *)(m->v + maxv);
    m->t = (mts_t *)(m->e + maxe);
    m->s = (mvs_t **)(m->t + maxt);
    m->l = (v2e_t *)(m->s + maxv);
    m->maxv = maxv;
    m->maxe = maxe;
    m->maxt = maxt;

    /* Заполняем вершины по точкам outline-а */
    mvs_t *v = m->v;
    mvs_t **s = m->s;
    for (int i = 0; i < o->ncontours; i++)
    {
        const ttf_point_t *pt = o->cont[i].pt;
        int len = o->cont[i].length;
        if (len < 3) continue;
        /* Делается побайтовое сравнение контуров для борьбы с повторами. */
        /* Дизайнеры иногда допускают. Пример U+2592 во множестве шрифтов. */
        bool duplicated = false;
        for (int j = 0; j < i && !duplicated; j++)
            duplicated = compare_contours(o, i, j);
        if (duplicated) continue;

        /* Определим тип контура (обычный или дырка) и если дырка, то кто родитель */
        int nested_to;
        bool hole = !ttf_outline_contour_info_majority(o, o->cont[i].subglyph_order, i, &nested_to);

        for (int j = 0; j < len; j++)
        {
            LIST_INIT(&v->edges);
            v->x = pt[j].x;
            v->y = pt[j].y;
            v->contour = i;
            v->subglyph = o->cont[i].subglyph_order;
            v->is_hole = hole;
            v->nested_to = nested_to;
            v->prev_in_contour = j == 0 ? v + len - 1 : v - 1;
            v->next_in_contour = j == len - 1 ? v - j : v + 1;
            v->index = v - m->v;
            *s = v;
            s++;
            v++;
        }
    }
    m->nv = v - m->v;

    /* Сортируем массив вершин по координате y */
    qsort(m->s, m->nv, sizeof(mvs_t *), mvs_sorting_fn);

    /* Инициализируем списки */
    LIST_INIT(&m->efree);
    LIST_INIT(&m->eused);
    LIST_INIT(&m->tfree);
    LIST_INIT(&m->tused);
    LIST_INIT(&m->lfree);
    LIST_INIT(&m->convx);
    for (int i = 0; i < m->maxe; i++)
    {
        m->e[i].index = i;
        LIST_INS_AFTER(m->e + i, m->efree.prev);
        LIST_INS_AFTER(m->l + i * 2 + 0, m->lfree.prev);
        LIST_INS_AFTER(m->l + i * 2 + 1, m->lfree.prev);
    }
    for (int i = 0; i < m->maxt; i++)
        LIST_INS_AFTER(m->t + i, m->tfree.prev);

    /* Инициализируем начальную грань. Для этого */
    /* находим boundbox по набору точек и делаем */
    /* грань по его нижней границе с запасом */
    float bbox_min[2];
    float bbox_max[2];
    bbox_min[0] = m->v->x; bbox_min[1] = m->v->y;
    bbox_max[0] = m->v->x; bbox_max[1] = m->v->y;
    for (int i = 0; i < m->nv; i++)
    {
        if (m->v[i].x < bbox_min[0]) bbox_min[0] = m->v[i].x;
        if (m->v[i].x > bbox_max[0]) bbox_max[0] = m->v[i].x;
        if (m->v[i].y < bbox_min[1]) bbox_min[1] = m->v[i].y;
        if (m->v[i].y > bbox_max[1]) bbox_max[1] = m->v[i].y;
    }
    m->vinit[0].x = bbox_min[0] - (bbox_max[0] - bbox_min[0]) * 0.12f;
    m->vinit[1].x = bbox_max[0] + (bbox_max[0] - bbox_min[0]) * 0.12f;
    m->vinit[0].y = bbox_min[1] - (bbox_max[1] - bbox_min[1]) * 0.21f;
    m->vinit[1].y = m->vinit[0].y;
    LIST_INIT(&m->vinit[0].edges);
    LIST_INIT(&m->vinit[1].edges);

    /* Инициализируем отладочные поля */
    m->debug.stop_at_step = -1;
    m->debug.curr_step = 0;
    m->debug.message[0] = 0;
    m->debug.breakpoint = false;

    return m;
}

static __inline v2e_t *create_v2e_link(mesher_t *m, mvs_t *v, mes_t *e)
{
    if (LIST_EMPTY(&m->lfree)) return NULL;
    v2e_t *res = LIST_FIRST(&m->lfree);
    LIST_DETACH(res);
    LIST_ATTACH(&v->edges, res);
    res->edge = e;
    return res;
}

static const mcc_t null_cc = { 0.0f, {0.0f, 0.0f} };

static mes_t *create_edge(mesher_t *m, mvs_t *v1, mvs_t *v2)
{
    if (LIST_EMPTY(&m->efree) || LIST_EMPTY(&m->lfree)) return NULL;
    mes_t *res = LIST_FIRST(&m->efree);
    LIST_DETACH(res);
    LIST_ATTACH(&m->eused, res);
    res->v1 = v1;
    res->v2 = v2;
    res->alt_cc[0] = null_cc;
    res->alt_cc[1] = null_cc;
    res->tr[0] = NULL;
    res->tr[1] = NULL;
    create_v2e_link(m, v1, res);
    create_v2e_link(m, v2, res);
    return res;
}

static mts_t *create_triangle(mesher_t *m, mes_t *e1, mes_t *e2, mes_t *e3)
{
    if (e1->tr[1] != NULL || e2->tr[1] != NULL || e2->tr[1] != NULL) return NULL;
    if (LIST_EMPTY(&m->tfree)) return NULL;
    mts_t *t = LIST_FIRST(&m->tfree);
    LIST_DETACH(t);
    LIST_ATTACH(&m->tused, t);
    t->helper = -1;
    t->cc = null_cc;
    e1->tr[1] = e1->tr[0]; e1->tr[0] = t;
    e2->tr[1] = e2->tr[0]; e2->tr[0] = t;
    e3->tr[1] = e3->tr[0]; e3->tr[0] = t;
    t->edge[0] = e1;
    t->edge[1] = e2;
    t->edge[2] = e3;
    return t;
}

static bool free_edge(mesher_t *m, mes_t *e)
{
    if (e->tr[0] != NULL) return false;
    for (v2e_t *l = e->v1->edges.next; l != &e->v1->edges; l = l->next)
        if (l->edge == e)
        {
            LIST_DETACH(l);
            LIST_ATTACH(&m->lfree, l);
            break;
        }
    for (v2e_t *l = e->v2->edges.next; l != &e->v2->edges; l = l->next)
        if (l->edge == e)
        {
            LIST_DETACH(l);
            LIST_ATTACH(&m->lfree, l);
            break;
        }
    LIST_DETACH(e);
    LIST_ATTACH(&m->efree, e);
    return true;
}

static __inline void change_edge(mesher_t *m, mes_t *e, mvs_t *v1, mvs_t *v2)
{
    mes_t *prev = e->prev;
    free_edge(m, e);
    create_edge(m, v1, v2);
    LIST_DETACH(e);
    LIST_INS_AFTER(e, prev);
}

static void free_triangle(mesher_t *m, mts_t *t, bool and_bare_edges)
{
    if (t->edge[0]->tr[0] == t) t->edge[0]->tr[0] = t->edge[0]->tr[1];
    if (t->edge[1]->tr[0] == t) t->edge[1]->tr[0] = t->edge[1]->tr[1];
    if (t->edge[2]->tr[0] == t) t->edge[2]->tr[0] = t->edge[2]->tr[1];
    t->edge[0]->tr[1] = NULL;
    t->edge[1]->tr[1] = NULL;
    t->edge[2]->tr[1] = NULL;
    t->edge[0]->alt_cc[0] = null_cc; t->edge[0]->alt_cc[1] = null_cc;
    t->edge[1]->alt_cc[0] = null_cc; t->edge[1]->alt_cc[1] = null_cc;
    t->edge[2]->alt_cc[0] = null_cc; t->edge[2]->alt_cc[1] = null_cc;
    if (and_bare_edges && t->edge[0]->tr[0] == NULL) free_edge(m, t->edge[0]);
    if (and_bare_edges && t->edge[1]->tr[0] == NULL) free_edge(m, t->edge[1]);
    if (and_bare_edges && t->edge[2]->tr[0] == NULL) free_edge(m, t->edge[2]);
    LIST_DETACH(t);
    LIST_ATTACH(&m->tfree, t);
}

void free_mesher(mesher_t *m)
{
    free(m);
}

#define DEBUG_POINT(msg) \
{ \
    if (m->debug.curr_step == m->debug.stop_at_step) \
    { \
        sprintf(m->debug.message, "%s", msg); \
        if (m->debug.breakpoint) \
            raise(SIGINT); else \
            return MESHER_TRAP; \
    } \
    m->debug.curr_step++; \
}

#define DEBUG_POINTF2(fmt, arg1, arg2) \
{ \
    if (m->debug.curr_step == m->debug.stop_at_step) \
    { \
        sprintf(m->debug.message, fmt, arg1, arg2); \
        if (m->debug.breakpoint) \
            raise(SIGINT); else \
            return MESHER_TRAP; \
    } \
    m->debug.curr_step++; \
}

#define DEBUG_POINTF(fmt, arg) DEBUG_POINTF2(fmt "%c", arg, 0)

#ifdef FAILED
#undef FAILED
#endif

#define FAILED(msg) \
{ \
    sprintf(m->debug.message, "%s", msg); \
    return MESHER_FAIL; \
}

int debug_check_for_corrupt_lists(mesher_t *m)
{
    int limit = 1000000;
    for (mes_t *e = m->eused.next; e != &m->eused; e = e->next)
        if (limit-- <= 0)
        {
            raise(SIGINT);
            FAILED("CORRUPT eused");
        }
    for (mts_t *t = m->tused.next; t != &m->tused; t = t->next)
        if (limit-- <= 0)
        {
            raise(SIGINT);
            FAILED("CORRUPT tused");
        }
    return MESHER_DONE;
}

static __inline mes_t *find_edge(mvs_t *v1, mvs_t *v2)
{
    for (v2e_t *v2l = v1->edges.next; v2l != &v1->edges; v2l = v2l->next)
        if (EDGE_HAS_VERT(v2l->edge, v1) && EDGE_HAS_VERT(v2l->edge, v2))
            return v2l->edge;
    return NULL;
}

/**
 * @brief Проверяет, есть ли у двух отрезков хотя бы одна общая точка
 *
 * Допускаются отрезки, вырожденные в прямую.
 * Результат работы соответствует геометрическому представлению,
 * когда два объекта (отрезок или точка) совпадают либо имеют на
 * чертеже хотя бы одну общую точку при конечном масштабном
 * приближении (определяется константой EPSILON)
 */
bool lines_has_common_point(const float *l1p1, const float *l1p2, const float *l2p1, const float *l2p2)
{
    float e1[2], e2[2], l2[2][2];
    float len, k, b, x0;

    VECSUB(e1, l1p2, l1p1);
    len = VECLEN2(e1);

    /* l1 - это точка? */
    if (len < EPSILON)
    {
        /* ---------->x---------> */
        /*   vec e1      vec e2   */
        VECSUB(e1, l1p1, l2p1);
        VECSUB(e2, l2p2, l1p1);
        if (fabsf(VECCROSS(e1, e2)) > EPSILON) return false;
        return VECDOT(e1, e2) >= -EPSILON;
    }

    /* Подготовим базис <e1,e2> такой что проекция */
    /* вектора (l1p2-l1p1) будет давать единицу */
    len = 1.0f / len;
    e1[0] *= len;
    e1[1] *= len;
    e2[0] = e1[1];
    e2[1] = -e1[0];

    /* Спроецируем l2 в этот базис */
    VECSUB(l2[0], l2p1, l1p1);
    VECSUB(l2[1], l2p2, l1p1);
    VECPROJ(l2[0], l2[0], e1, e2);
    VECPROJ(l2[1], l2[1], e1, e2);

    /* Проверим, имеет ли l2 общие точки c */
    /* единичным отрезком на оси OX */

    /* l2 - это точка? */
    VECSUB(e1, l2[1], l2[0]);
    len = VECLEN2(e1);
    if (len < EPSILON)
        return (l2[0][0] >= -EPSILON) && (l2[0][0] <= 1.0f + EPSILON) && (fabsf(l2[0][1]) <= EPSILON);

    /* l2 - это горизонтальная прямая? */
    if (fabsf(l2[0][1] - l2[1][1]) <= EPSILON)
    {
        if (fabsf(l2[0][1]) > EPSILON) return false; /* Не на оси OX */
        if (l2[0][0] >= -EPSILON && l2[0][0] <= 1.0f + EPSILON) return true;
        if (l2[1][0] >= -EPSILON && l2[1][0] <= 1.0f + EPSILON) return true;
        if (l2[0][0] * l2[1][0] <= EPSILON) return true;
        return false;
    }

    /* l2 - это вертикальная прямая? */
    if (fabsf(l2[0][0] - l2[1][0]) <= EPSILON)
        return (l2[0][0] >= -EPSILON) && (l2[0][0] <= 1.0f + EPSILON) &&
               (l2[0][1] * l2[1][1] <= EPSILON);

    /* l2 - это наклонная прямая, проверим её в диапазоне y */
    if (l2[0][1] * l2[1][1] > EPSILON) return false;

    /* Проверим пересечение с OX решив уравнение kx + b = 0  */
    /* Выражденные случаи мы проверили предыдущими условиями */
    CRAMER2x2(k, b,
              l2[0][0], 1.0f, l2[0][1],
              l2[1][0], 1.0f, l2[1][1]);
    x0 = -b / k;

    return x0 > -EPSILON && x0 <= 1.0f + EPSILON;
}

bool lines_cross_args(const mvs_t *a1, const mvs_t *a2, const mvs_t *b1, const mvs_t *b2, float *aarg, float *barg)
{
/*
      a1
     /
b1__/_______ b2
   /
  a2
*/

    /* { c = a1 + (a2 - a1) * d1 */
    /* { c = b1 + (b2 - b1) * d2 */

    /* (a2 - a1) * d1 - (b2 - b1) * d2 + a1 - b1 = 0 */

    /*
        { (ax2 - ax1) d1  +  (bx1 - bx2) d2  =  bx1 - ax1
        { (ay2 - ay1) d1  +  (by1 - by2) d2  =  by1 - ay1

        [a1 b1] [d1] = [c1]
        [a2 b2] [d2]   [c2]
    */

    float A1 = a2->x - a1->x;
    float B1 = b1->x - b2->x;
    float A2 = a2->y - a1->y;
    float B2 = b1->y - b2->y;
    float C1 = b1->x - a1->x;
    float C2 = b1->y - a1->y;

    float t = A1 * B2 - B1 * A2;
    if (fabs(t) < EPSILON) return false;
    t = 1.0f / t;
    *aarg = (C1 * B2 - B1 * C2) * t;
    *barg = (A1 * C2 - C1 * A2) * t;
    return true;
}

/* return 1 if quad is convex */
int is_convex_quad(const mvs_t *A, const mvs_t *B, const mvs_t *C, const mvs_t *D)
{
    float v[4][2];
    float z[4];
    VECSUB(v[0], &B->x, &A->x);
    VECSUB(v[1], &C->x, &B->x);
    VECSUB(v[2], &D->x, &C->x);
    VECSUB(v[3], &A->x, &D->x);
    z[0] = VECCROSS(v[0], v[1]);
    z[1] = VECCROSS(v[1], v[2]);
    z[2] = VECCROSS(v[2], v[3]);
    z[3] = VECCROSS(v[3], v[0]);
    if (z[0] * z[1] <= 0.0f) return 0;
    if (z[1] * z[2] <= 0.0f) return 0;
    if (z[2] * z[3] <= 0.0f) return 0;
    if (z[3] * z[0] <= 0.0f) return 0;
    return 1;
}

int flip_edge(mesher_t *m, mes_t *e)
{
    /*
      Делаем инваиантное преобразование

      v1
      /|\                      / \
   c / | \ a                c /t0 \ a
  B / e|  \ A      ->        /_____\ e->v1
    \t1|t0/                  \   e /
   d \ | / b                d \t1 / b
      \|/                      \ /
      v2
    */

    mts_t *t0 = e->tr[0];
    mts_t *t1 = e->tr[1];
    mes_t *a = TRI_SECOND_EDGE(t0, e);
    mes_t *b = TRI_THIRD_EDGE(t0, e);
    mes_t *c = TRI_SECOND_EDGE(t1, e);
    mes_t *d = TRI_THIRD_EDGE(t1, e);
    if (!EDGE_HAS_VERT(a, e->v1)) SWAP(mes_t *, a, b);
    if (!EDGE_HAS_VERT(c, e->v1)) SWAP(mes_t *, c, d);
    mvs_t *A = EDGES_COMMON_VERT(a, b);
    mvs_t *B = EDGES_COMMON_VERT(c, d);

    DEBUG_POINTF("flip edge %i", e->index);

    mts_t t0copy = *t0;
    mts_t t1copy = *t1;
    mes_t ecopy = *e;
    free_triangle(m, t0, false);
    free_triangle(m, t1, false);
    change_edge(m, e, A, B);
    t0 = create_triangle(m, a, c, e);
    t1 = create_triangle(m, b, d, e);
    if (t0 == NULL || t1 == NULL) FAILED("flip_edge");

    /* Восстанавливаем поля после пересоздания */
    t0->helper = t0copy.helper;
    t1->helper = t1copy.helper;
    t0->cc = ecopy.alt_cc[0];
    t1->cc = ecopy.alt_cc[1];
    e->alt_cc[0] = t1copy.cc; /* flip copy! */
    e->alt_cc[1] = t0copy.cc;

    return MESHER_DONE;
}

static bool calc_circumcircle(const float A[2], const float B[2], const float C[2], mcc_t *cc)
{
    /*
    { (o[0] - A[0])^2 + (o[1] - A[1])^2 = r^2
    { (o[0] - B[0])^2 + (o[1] - B[1])^2 = r^2
    { (o[0] - C[0])^2 + (o[1] - C[1])^2 = r^2

  _ { o[0]^2 - 2 o[0] A[0] + A[0]^2   +   o[1]^2 - 2 o[1] A[1] + A[1]^2   =   r^2
  _ { o[0]^2 - 2 o[0] B[0] + B[0]^2   +   o[1]^2 - 2 o[1] B[1] + B[1]^2   =   r^2
    { o[0]^2 - 2 o[0] C[0] + C[0]^2   +   o[1]^2 - 2 o[1] C[1] + C[1]^2   =   r^2

    { o[0] 2 (A[0] - B[0]) + o[1] 2 (A[1] - B[1]) = A[0]^2 + A[1]^2 - B[0]^2 - B[1]^2
    { o[0] 2 (B[0] - C[0]) + o[1] 2 (B[1] - C[1]) = B[0]^2 + B[1]^2 - C[0]^2 - C[1]^2
    */

    float a[4], b[2];
    a[0] = (A[0] - B[0]) * 2.0f; a[1] = (A[1] - B[1]) * 2.0f;
    a[2] = (B[0] - C[0]) * 2.0f; a[3] = (B[1] - C[1]) * 2.0f;
    b[0] = A[0]*A[0] + A[1]*A[1] - B[0]*B[0] - B[1]*B[1];
    b[1] = B[0]*B[0] + B[1]*B[1] - C[0]*C[0] - C[1]*C[1];

    /* linsolver of: a * XY = c */
    float det = a[0] * a[3] - a[1] * a[2];
    if (fabsf(det) <= EPSILON) return false;
    cc->center[0] = (b[0] * a[3] - a[1] * b[1]) / det;
    cc->center[1] = (a[0] * b[1] - b[0] * a[2]) / det;

    /* calc r */
    float dx = A[0] - cc->center[0];
    float dy = A[1] - cc->center[1];
    cc->radius = sqrtf(dx * dx + dy * dy);
    return true;
}

int optimize(mesher_t *m, mes_t *e, int deep)
{
    if (deep <= 0 || e->tr[1] == NULL) return MESHER_DONE;
    if (IS_CONTOUR_EDGE(e)) return MESHER_DONE;
    mvs_t *o0 = OPPOSITE_VERT(e->tr[0], e);
    mvs_t *o1 = OPPOSITE_VERT(e->tr[1], e);

    /* Проверим четырёхугольник на выпуклость (впуклые уже оптимальны по Делоне) */
    if (!is_convex_quad(e->v1, o0, e->v2, o1)) return MESHER_DONE;

    /* Проверим условие Делоне */
//    if (check_delone(e->v1, o0, e->v2, o1)) return MESHER_DONE;

    bool done1 = true;
    if (e->tr[0]->cc.radius == 0.0f) done1 &= calc_circumcircle(&e->v1->x, &o0->x, &e->v2->x, &e->tr[0]->cc);
    if (e->tr[1]->cc.radius == 0.0f) done1 &= calc_circumcircle(&e->v1->x, &o1->x, &e->v2->x, &e->tr[1]->cc);

    bool done2 = true;
    if (e->alt_cc[0].radius == 0.0f) done2 &= calc_circumcircle(&o0->x, &e->v1->x, &o1->x, &e->alt_cc[0]);
    if (e->alt_cc[1].radius == 0.0f) done2 &= calc_circumcircle(&o0->x, &e->v2->x, &o1->x, &e->alt_cc[1]);

    if (!done2)
        return MESHER_DONE;

    if (done1 && done2)
        if (e->alt_cc[0].radius + e->alt_cc[1].radius + EPSILON > e->tr[0]->cc.radius + e->tr[1]->cc.radius)
            return MESHER_DONE;

    int res = flip_edge(m, e);
    if (res != MESHER_DONE) return res;

    deep--;
    if (deep == 0) return MESHER_DONE;
    optimize(m, TRI_SECOND_EDGE(e->tr[0], e), deep);
    optimize(m, TRI_SECOND_EDGE(e->tr[1], e), deep);
    optimize(m, TRI_THIRD_EDGE(e->tr[0], e), deep);
    optimize(m, TRI_THIRD_EDGE(e->tr[1], e), deep);
    return MESHER_DONE;
}

static int find_triangles_track(mesher_t *m, mvs_t *v1, mvs_t *v2, mes_t *root)
{
    mts_t *tcurr = NULL;
    mes_t *ecurr = NULL;

    (void)m;

    DEBUG_POINTF2("find track p%i->p%i", v1->index, v2->index);

    /* Найдём треугольник и его ребро, противоположное v1, которое пересекает отрезок v1-v2 */
    for (v2e_t *v2l = v1->edges.next; v2l != &v1->edges && tcurr == NULL; v2l = v2l->next)
    {
        /* Цикл по треугольникам на текущем ребре */
        for (int i = 0; i < 2 && tcurr == NULL; i++)
        {
            mts_t *t = v2l->edge->tr[i];
            if (t == NULL) continue;
            mes_t *e = OPPOSITE_EDGE(t, v1);
            if (!lines_has_common_point(&v1->x, &v2->x, &e->v1->x, &e->v2->x)) continue;
            tcurr = t;
            ecurr = e;
        }
    }

    if (tcurr == NULL) FAILED("find_triangles_track #1");
    tcurr->helper = -2;
    while (true)
    {
        if (ecurr->tr[1] == NULL) FAILED("find_triangles_track #2");
        if (ecurr->tr[1] == tcurr) SWAP(mts_t *, ecurr->tr[0], ecurr->tr[1]);
        LIST_DETACH(ecurr);
        LIST_INS_LAST(root, ecurr);
        tcurr = ecurr->tr[1];
        if (tcurr->helper == -2) FAILED("find_triangles_track #3");
        tcurr->helper = -2;
        if (OPPOSITE_VERT(tcurr, ecurr) == v2) break;
        mes_t *e1 = TRI_SECOND_EDGE(tcurr, ecurr);
        mes_t *e2 = TRI_THIRD_EDGE(tcurr, ecurr);
        if (lines_has_common_point(&v1->x, &v2->x, &e1->v1->x, &e1->v2->x))
            ecurr = e1;
        else
        if (lines_has_common_point(&v1->x, &v2->x, &e2->v1->x, &e2->v2->x))
            ecurr = e2;
        else
            FAILED("find_triangles_track #4");
    }

    return MESHER_DONE;
}

static mes_t *make_convex(mesher_t *m, mes_t *e1, mes_t *e2, mes_t *ret_default)
{
    float d[2][2];
    VECSUB(d[0], &e1->v2->x, &e1->v1->x);
    VECSUB(d[1], &e2->v2->x, &e2->v1->x);
    float cross = VECCROSS(d[0], d[1]);
    if (cross <= 0) return ret_default;

    mes_t *N = create_edge(m, e1->v1, e2->v2);
    if (N == NULL) return NULL;
    LIST_DETACH(N);
    LIST_INS_AFTER(N, e2);
    LIST_REATTACH(&m->eused, e1);
    LIST_REATTACH(&m->eused, e2);
    if (create_triangle(m, e1, e2, N) == NULL)
        return NULL;
    return N;
}

static mes_t *make_convex90(mesher_t *m, mes_t *e1, mes_t *e2, mes_t *ret_default)
{
    float d[2][2];
    VECSUB(d[0], &e1->v2->x, &e1->v1->x);
    VECSUB(d[1], &e2->v2->x, &e2->v1->x);
    float l1 = VECLEN(d[0]);
    float l2 = VECLEN(d[1]);
    l1 = 1.0f / l1;
    l2 = 1.0f / l2;
    float as = VECCROSS(d[0], d[1]) * l1 * l2;
    float ac = VECDOT(d[0], d[1]) * l1 * l2;
    if (as < 0) return ret_default;
    if (ac > 0) return ret_default;

    mes_t *N = create_edge(m, e1->v1, e2->v2);
    if (N == NULL) return NULL;
    LIST_DETACH(N);
    LIST_INS_AFTER(N, e2);
    LIST_REATTACH(&m->eused, e1);
    LIST_REATTACH(&m->eused, e2);
    if (create_triangle(m, e1, e2, N) == NULL)
        return NULL;
    return N;
}

/**
 * @brief Алгоритм линейного заметания (без ограничений)
 * @param m Структура мешера
 * @return MESHER_XXX
 *
 * Модификация линейного алгоритма заметания точек. Результатом
 * работы является выпуклая неоптимальная триангуляция без
 * ограничений. Суть алгоритма: перебираются точки триангуляции
 * ранее отсортированные по координате (y); из каждой точки
 * опускается вертикаль на заметающую ломаную. Эта вертикаль
 * пересекает определённый отрезок в составе ломаной кривой.
 * Такой отрезок и текущая точка составляют новый треугольник,
 * после чего он исключается из заметающей ломаной, а заместо
 * него вставляются 2 образовавшихся ребра треугольника.
 * Модификация алгоритма состоит в том, что образовавшийся выступ
 * соединяется с соседними отрезками заметающей ломаной при
 * соблюдении ограничивающих условий. Такой подход позволяет
 * заметно приблизиться к оптимальному графу и зачастую
 * предотвращает образование сильно игольчатой заметающей ломаной.
 * После перебора всех точек триангуляция достраивается до
 * выпуклой.
 * Сложность алгоритма стремится к линейной, что обусловлено
 * характером следования точек: при переборе отсортированных
 * по (y) точек велика вероятность, что соседние точки принадлежат
 * одному контуру глифа, то есть удалены друг от друга по (x)
 * незначительно. В этом случае опустившаяся вертикаль на ломаную
 * кривую со значительной вероятностью попадёт на недавно созданный
 * отрезок этой ломаной. Поэтому в большинстве случаев не
 * приходится искать в списке отрезков ломаной тот отрезок, который
 * пересекает вертикаль.
 */
static int sweep_points(mesher_t *m, int object)
{
    /* ЭТАП 1. Невыпуклая триангуляция */

    /* Инициализация */
    mes_t *curr = create_edge(m, m->vinit + 0, m->vinit + 1);
    if (curr == NULL) FAILED("sweep_contour");
    LIST_INIT(&m->convx);
    LIST_REATTACH(&m->convx, curr);

    /* Цикл по всем вершинам выбранного контура */
    for (int i = 0; i < m->nv; i++)
    {
        mvs_t *v = m->s[i];

        if (v->object != object) continue;

        DEBUG_POINTF("sweeping point %i", v->index);

        /* Нойдём ребро прямо под текущей точкой */
        if (curr->v1->x > v->x)
        {
            /* Движемся влево по оболочке */
            while (1)
            {
                curr = curr->prev;
                float dx1 = curr->v1->x - v->x;
                float dx2 = curr->v2->x - v->x;
                if (dx1 * dx2 <= 0 && (dx1 != 0 || dx2 != 0)) break;
            }
        }
        else
        if (curr->v2->x < v->x)
        {
            /* Движемся вправо по оболочке */
            while (1)
            {
                curr = curr->next;
                float dx1 = curr->v1->x - v->x;
                float dx2 = curr->v2->x - v->x;
                if (dx1 * dx2 <= 0 && (dx1 != 0 || dx2 != 0)) break;
            }
        }

        if (fabsf(curr->v1->x - v->x) <= EPSILON)
            if (fabsf(curr->v1->y - v->y) <= EPSILON)
                FAILED("sweep: dup points");

        if (fabsf(curr->v2->x - v->x) <= EPSILON)
            if (fabsf(curr->v2->y - v->y) <= EPSILON)
                FAILED("sweep: dup points");

        /* Создаём два новых ребра (слева и справа от текущей точки) */
        mes_t *L = create_edge(m, curr->v1, v);
        mes_t *R = create_edge(m, v, curr->v2);
        if (L == NULL || R == NULL) FAILED("sweep: create_edge");

        /* Заменяем в оболочке найденное ребро на два созданных. При этом */
        /* соблюдаем свойство сортировки рёбер в оболочке по координате x. */
        LIST_DETACH(L);
        LIST_DETACH(R);
        LIST_INS_AFTER(L, curr);
        LIST_INS_AFTER(R, L);
        LIST_REATTACH(&m->eused, curr);

        /* Регистрируем треугольник на всех трёх рёбрах */
        if (create_triangle(m, L, R, curr) == NULL)
            FAILED("sweep: create_triangle");

        DEBUG_POINT("sweep: make_convex");

        /* Если грань вертикальная, то её обязательно заметаем */
        if (fabsf(L->v1->x - L->v2->x) <= EPSILON)
        {
            L = make_convex(m, L->prev, L, L);
            if (L == NULL) FAILED("sweep: make_convex");
        }
        if (fabsf(R->v1->x - R->v2->x) <= EPSILON)
        {
            R = make_convex(m, R, R->next, R);
            if (L == NULL) FAILED("sweep: make_convex");
        }

        while (L->prev != &m->convx)
        {
            mes_t *tmp = make_convex90(m, L->prev, L, L);
            if (tmp == NULL) FAILED("sweep: make_convex90");
            if (tmp == L) break;
            L = tmp;
        }

        while (R->next != &m->convx)
        {
            mes_t *tmp = make_convex90(m, R, R->next, R);
            if (tmp == R) break;
            if (tmp == NULL) FAILED("sweep: make_convex90");
            R = tmp;
        }

        curr = R; /* Может и L, нет смысла угадывать характер данных */

        if (L == NULL || R == NULL)
            FAILED("sweep: make_convex");
    }

    /* ЭТАП 2. Достраивание триангуляции до выпуклой */

    bool done = true;
    while (done)
    {
        done = false;
        mes_t *e1 = m->convx.next;
        mes_t *e2 = e1->next;
        while (e1 != &m->convx && e2 != &m->convx)
        {
            DEBUG_POINT("sweep finishing");
            e1 = make_convex(m, e1, e2, e2);
            if (e1 == NULL) FAILED("sweep finishing: make_convex90");
            if (e1 != e2) done = true;
            e2 = e1->next;
        }
    }

    while (!LIST_EMPTY(&m->convx))
    {
        mes_t *e = m->convx.next;
        LIST_DETACH(e);
        LIST_ATTACH(&m->eused, e);
    }

    return MESHER_DONE;
}

/**
 * @brief Удаляет лишние треугольники из выпуклой триангуляции
 * @param m Мешер
 * @return 0 или код ошибки (MESHER_XXX)
 *
 * Алгоритм простой и по сути является вариантом even-odd.
 * Обход всех треугольников выпуклой триангуляции начинается с
 * треугольника, построенного на вспомогательном ребре ниже
 * всего графа. Этот треугольник помечается для удаления,
 * а также - все его соседи, если они не отделены от него
 * структурным отрезком. Каждый раз при переходе структурного
 * отрезка признак удаления инвертируется. После обхода всех
 * треугольников удаляются те, что помечены для удаления.
 * Алгоритм показывет себя весьма эффективным с линейной
 * сложностью.
 */
static int remove_excess_triangles(mesher_t *m)
{
    /* Сделаем перебор треугольников, двигаясь от соседа к соседу */
    /* Если разделяющая грань лежит на контуре, то инвертируем знак */
    /* при переходе через неё. Затем удалим все отрицательные треугольники. */

    DEBUG_POINT("removing triangles");

    mts_t root; /* Корень формируемого списка */
    LIST_INIT(&root);

    /* Добавим первый треугольник с отрицательным знаком (helper=0) */
    v2e_t *l = m->vinit[0].edges.next;
    if (l == &m->vinit[0].edges) FAILED("remove_excess_triangles");
    if (l->edge[0].tr[0] == NULL) FAILED("remove_excess_triangles");
    l->edge[0].tr[0]->helper = 0;
    LIST_REATTACH(&root, l->edge[0].tr[0]);

    mts_t *t = root.next;
    while (t != &root)
    {
        for (int i = 0; i < 3; i++)
        {
            mts_t *neighbor = t->edge[i]->tr[0] == t ? t->edge[i]->tr[1] : t->edge[i]->tr[0];
            if (neighbor == NULL) continue;
            if (neighbor->helper >= 0) continue;
            neighbor->helper = IS_CONTOUR_EDGE(t->edge[i]) ? t->helper ^ 1 : t->helper;
            LIST_DETACH(neighbor);
            LIST_INS_LAST(&root, neighbor);
        }
        mts_t *tmp = t;
        t = t->next;

        /* Удаляем, либо возвращаем в штатный список */
        if (tmp->helper == 0)
            free_triangle(m, tmp, true); else
            LIST_REATTACH(&m->tused, tmp);
    }

    return MESHER_DONE;
}

/**
 * @brief Вспомогательная функция вызова insert_fixed_edge
 * @param m Мешер
 * @param cntr Список рёбер, описывающих отверстие в сетке
 * @param base Опорное ребро
 * @return 0 или код ошибки (MESHER_XXX)
 *
 * Опорное ребро на начальном этапе работы insert_fixed_edge -
 * это вставляемый структурный отрезок. Относительно него дыра
 * описывается двумя контурами (сверху и снизу). Дыра эта
 * образуется до вызова функции после удаления треугольников
 * и рёбер на пути вставления структурного отрезка. Таким
 * образом, согласно классификации А.В. Скворцова, данная
 * функция является строительной в части алгоритма "вставляй
 * и строй".
 * Алгоритм строительства описывается следующим образом.
 * Ищем в контуре ближайшую точку к опорному ребру, образуем
 * треугольник на опорном ребре и этой точке, повторяем алгоритм
 * на двух новых опорных рёбрах, образованных треугольником. При
 * этом в ближайшей найденной точке рвём список cntr.
 */
int triangulate_hole(mesher_t *m, mes_t *cntr, mes_t *base)
{
    if (cntr->next == cntr)
    {
        LIST_REATTACH(&m->eused, cntr);
        return MESHER_DONE;
    }

    /* Если в контуре только 2 грани, то формируем треугольник */
    if (cntr->next->next == cntr)
    {
        DEBUG_POINTF("make triangle on e%i", base->index);
        mts_t *t = create_triangle(m, cntr, cntr->next, base);
        if (t == NULL) FAILED("triangulate_holl #1");
        /* И вернём мешеру грани из контура */
        LIST_REATTACH(&m->eused, t->edge[0]);
        LIST_REATTACH(&m->eused, t->edge[1]);
        return MESHER_DONE;
    }

    /* Ищем самую близкую точку из контура к ребру base */

    DEBUG_POINTF("find closest point to e%i", base->index);

    mes_t *closest_edge = NULL;
    mvs_t *closest_vert = NULL;
    float closest_proj = 0; /* Дальность не нормированная */

    float base_dir[2]; /* Вектор направленности */
    float orth[2]; /* Ортогональный вектор к вектору base */
    VECSUB(base_dir, &base->v2->x, &base->v1->x);
    orth[0] = +base_dir[1];
    orth[1] = -base_dir[0];

    for (mes_t *e = cntr; e->next != cntr; e = e->next)
    {
        mvs_t *v = EDGES_COMMON_VERT(e, e->next);
        /*
                        . v
                        |
                        | proj
                        |
        base.v1 ._______|_______. base.b2
        */

        float vvec[2];
        VECSUB(vvec, &v->x, &base->v1->x);
        float proj = fabsf(VECDOT(orth, vvec));
        if (closest_edge == NULL || proj < closest_proj)
        {
            closest_proj = proj;
            closest_edge = e;
            closest_vert = v;
        }
    }

    if (closest_vert == base->v1 || closest_vert == base->v2)
        FAILED("triangulate_holl #2");

    /* Формируем треугольник на базовом ребре и найденной точке */
    mes_t *L = find_edge(base->v1, closest_vert);
    if (L == NULL)
        L = create_edge(m, base->v1, closest_vert);
    mes_t *R = find_edge(closest_vert, base->v2);
    if (R == NULL)
        R = create_edge(m, closest_vert, base->v2);
    if (L == NULL || R == NULL) FAILED("triangulate_holl #3");
    DEBUG_POINTF2("make triangle on e%i and p%i", base->index, closest_vert->index);
    mts_t *t = create_triangle(m, base, L, R);
    if (t == NULL) FAILED("triangulate_holl #4");

    /* Запускаем функцию рекурсивно с базовым ребром L и R */
    /* Сначала размыкаем контур, формируя из него 2 других */
    mes_t *new_cntr1_beg = cntr;
    mes_t *new_cntr1_end = closest_edge;
    mes_t *new_cntr2_beg = closest_edge->next;
    mes_t *new_cntr2_end = cntr->prev;
    new_cntr1_beg->prev = new_cntr1_end;
    new_cntr1_end->next = new_cntr1_beg;
    new_cntr2_beg->prev = new_cntr2_end;
    new_cntr2_end->next = new_cntr2_beg;

    int res = triangulate_hole(m, new_cntr1_beg, L);
    if (res != MESHER_DONE) return res;

    res = triangulate_hole(m, new_cntr2_beg, R);
    if (res != MESHER_DONE) return res;

    return MESHER_DONE;
}

/**
 * @brief Функция вставки одного структурного отрезка
 * @param m Мешер
 * @param v1 Первая образующая вершина
 * @param v2 Вторая образующая вершина
 * @return 0 или код ошибки (MESHER_XXX)
 *
 * Эта функция вызывается для всех структурных отрезков,
 * которые должны описывать замкнутые контуры, но отсутствуют
 * после выполнения триангуляции без ограничений. Функция
 * является одной из самых сложных. Дальнейшая работа
 * заключается в её оптимизации и повышении устойчивости.
 * Если исходные контуры содержат ошибки, то это, как правило,
 * выявляется в процессе работы этой функции. Посему всегда
 * актуально искать её наиболее устойчивую вариацию.
 */
int insert_fixed_edge(mesher_t *m, mvs_t *v1, mvs_t *v2)
{
    mes_t track;
    LIST_INIT(&track);
    int res = find_triangles_track(m, v1, v2, &track);
    if (res != MESHER_DONE) return res;

    /* Формируем два контура из рёбер, описывающих образовавшуюся после удаления */
    /* треугольников пустоту (с одной и с другой стороны от вставляемого ребра v1->v2) */

    mes_t *cntr1 = find_edge(v1, track.next->v1);
    mes_t *cntr2 = find_edge(v1, track.next->v2);
    LIST_DETACH(cntr1); LIST_INIT(cntr1);
    LIST_DETACH(cntr2); LIST_INIT(cntr2);
    for (mes_t *e = track.next; e != &track; e = e->next)
    {
        mts_t *t = e->tr[1];
        mes_t *e2dn = TRI_SECOND_EDGE(t, e);
        mes_t *e3rd = TRI_THIRD_EDGE(t, e);
        mes_t *c2nd = e2dn == e->next ? NULL : (EDGES_CONNECTED(cntr1, e2dn) ? cntr1 : cntr2);
        mes_t *c3rd = e3rd == e->next ? NULL : (EDGES_CONNECTED(cntr1, e3rd) ? cntr1 : cntr2);
        if (c2nd != NULL) LIST_REATTACH(c2nd, e2dn);
        if (c3rd != NULL) LIST_REATTACH(c3rd, e3rd);
        if (c2nd == cntr1 || c3rd == cntr1) cntr1 = cntr1->next;
        if (c2nd == cntr2 || c3rd == cntr2) cntr2 = cntr2->next;
    }
    cntr1 = cntr1->next;
    cntr2 = cntr2->next;

    DEBUG_POINTF2("remove track v%i->v%i triangles", v1->index, v2->index);

    /* Удалим мешающие треугольники */
    for (mes_t *e = track.next; e != &track; e = e->next)
    {
        if (e->tr[1] != NULL) free_triangle(m, e->tr[1], false);
        if (e->tr[0] != NULL) free_triangle(m, e->tr[0], false);
    }

    /* Удаляем больше не нужные грани трека */
    while (!LIST_EMPTY(&track))
    {
        if (IS_CONTOUR_EDGE(track.next))
            FAILED("Contours intersect");
        free_edge(m, track.next);
    }

    /* Проверяем контуры на целостность. Это временная мера. */
    mvs_t *v = v1;
    for (mes_t *e = cntr1; ; e = e->next)
    {
        if (!EDGE_HAS_VERT(e, v)) FAILED("insert_fixed_edge #1");
        v = EDGE_SECOND_VERT(e, v);
        if (e->next != cntr1) continue;
        if (v != v2) FAILED("insert_fixed_edge #2");
        break;
    }
    v = v1;
    for (mes_t *e = cntr2; ; e = e->next)
    {
        if (!EDGE_HAS_VERT(e, v)) FAILED("insert_fixed_edge #1");
        v = EDGE_SECOND_VERT(e, v);
        if (e->next != cntr2) continue;
        if (v != v2) FAILED("insert_fixed_edge #2");
        break;
    }

    /* Создаём вставляемое ребро */
    DEBUG_POINTF2("insert edge v%i->v%i triangles", v1->index, v2->index);
    mes_t *ins = create_edge(m, v1, v2);
    if (ins == NULL) FAILED("insertion");

    /* Триангулируем получившуюся после удаления дыру рекурсивным */
    /* алгоритмом. В процессе работы он вернёт все грани списков */
    /* cntr1 и cntr2 обратно мешеру (в список eused) */
    res = triangulate_hole(m, cntr1, ins);
    if (res != MESHER_DONE) return res;

    res = triangulate_hole(m, cntr2, ins);
    if (res != MESHER_DONE) return res;

    return MESHER_DONE;
}

/**
 * @brief Функция последовательной вставки структурных отрезков
 * @param m Мешер
 * @param object Объект триангуляции
 * @return 0 или код ошибки (MESHER_XXX)
 */
int handle_constraints(mesher_t *m, int object)
{
    for (int i = 0; i < m->nv; i++)
    {
        if (m->v[i].object != object) continue;
        if (find_edge(&m->v[i], m->v[i].prev_in_contour) != NULL) continue;
        int res = insert_fixed_edge(m, &m->v[i], m->v[i].prev_in_contour);
        if (res != MESHER_DONE) return res;
    }
    return MESHER_DONE;
}

/**
 * @brief Попытка оптимизировать весь граф
 * @param m Мешер
 * @param deep Глубина оптимизации (не имеет сузначения при deep > 100)
 * @param object Объект триангуляции
 * @return 0 или код ошибки (MESHER_XXX)
 */
int optimize_all(mesher_t *m, int deep, int object)
{
    mes_t *e = m->eused.next;
    while (e != &m->eused)
    {
        mes_t *opt = e;
        e = e->next;
        if (opt->v1->object != object) continue;
        int res = optimize(m, opt, deep);
        if (res != MESHER_DONE) return res;
    }
    return MESHER_DONE;
}

/**
 * @brief Подготавливает независимые объекты триангуляции
 *
 * Некоторые символы содержат несколько независимых контуров.
 * Если их обрабатывать одним проходом триангуляции, то высока
 * вероятность ошибки из-за дублирующихся точек и пересечений.
 * Поэтому триангуляция проводится независимо по таким контурам.
 * Данная функция назначает каждой точке её порядковый номер
 * триангуляции в зависимости от выставленных ранее полей
 * contour и nested_to. Возвращает функция количество объектов
 * триангуляции.
 */
int prepare_triangulation_objects(mesher_t *m)
{
    int res = 0;
    int16_t cont2obj[256];
    for (int i = 0; i < 256; i++)
        cont2obj[i] = -1;
    for (int i = 0; i < m->nv; i++)
    {
        if (m->v[i].contour >= 256) return -1;
        if (m->v[i].is_hole) continue;
        int c = m->v[i].contour;
        if (cont2obj[c] == -1)
            cont2obj[c] = res++;
    }
    for (int i = 0; i < m->nv; i++)
        if (m->v[i].is_hole)
        {
            if (m->v[i].nested_to < 0 || m->v[i].nested_to >= 256) return -1;
            m->v[i].object = cont2obj[m->v[i].nested_to];
        }
        else
        {
            m->v[i].object = cont2obj[m->v[i].contour];
        }
    return res;
}

/**
 * @brief Функция исправления контурных ошибок
 *
 * Пытается исправить два типа ошибок: точки с дублирующимися
 * координатами и перекруты контура. В некоторых шрифтах такие
 * эффекты наблюдаются. В большинстве случаев можно говорить
 * об ошибках дизайна. Подобные ошибки не критичны для типового
 * растеризатора, но триангуляция на таких контурах терпит
 * неудачу. По этой причине вводится данная функция. Далее она
 * будет усложняться.
 */
int fix_contours_bugs(mesher_t *m)
{
    /* Попытаемся бороться с дублирующимися точками */
    bool need_resorting = false;
    for (int i = 0; i < m->nv - 1; i++)
    {
        mvs_t *v1 = m->s[i];
        mvs_t *v2 = m->s[i + 1];
        float dx = v1->x - v2->x;
        float dy = v1->y - v2->y;
        if (fabsf(dx) > EPSILON || fabsf(dy) > EPSILON) continue;

        /* Встретились дубликаты. Попробуем раздвинуть точки */
        /* так чтобы не создать коллизию контуров */
        DEBUG_POINTF2("separate p%i and p%i", v1->index, v2->index);
        float v1dir[2][2]; /* Направление от точки в сторону её соседей в контуре */
        float v2dir[2][2];
        float delta[2][2]; /* Вектор перемещения двух точек */
        VECSUB(v1dir[0], &v1->prev_in_contour->x, &v1->x); VECSCALE(v1dir[0], v1dir[0], 1e-4f * VECLEN(v1dir[0]));
        VECSUB(v1dir[1], &v1->next_in_contour->x, &v1->x); VECSCALE(v1dir[1], v1dir[1], 1e-4f * VECLEN(v1dir[1]));
        VECSUB(v2dir[0], &v2->prev_in_contour->x, &v2->x); VECSCALE(v2dir[0], v2dir[0], 1e-4f * VECLEN(v2dir[0]));
        VECSUB(v2dir[1], &v2->next_in_contour->x, &v2->x); VECSCALE(v2dir[1], v2dir[1], 1e-4f * VECLEN(v2dir[1]));
        VECADD(delta[0], v1dir[0], v1dir[1]);
        VECADD(delta[1], v2dir[0], v2dir[1]);
        VECADD(&v1->x, &v1->x, delta[0]);
        VECADD(&v2->x, &v2->x, delta[1]);
        need_resorting = true;
    }
    /* Сортируем массив вершин по координате y */
    if (need_resorting)
        qsort(m->s, m->nv, sizeof(mvs_t *), mvs_sorting_fn);

    /* Попытаемся бороться с перекрутами контура, вроде такого:
            D|                         D
   A ________|__ B                     |
             | /      меняем на        / B    путём перестановки B и C местами в контуре
             |/               ________/
             C                A       C
    */
    for (int i = 0; i < m->nv; i++)
    {
        mvs_t *A= m->v + i;
        mvs_t *B = A->next_in_contour;
        mvs_t *C = B->next_in_contour;
        mvs_t *D = C->next_in_contour;
        if (A == B || A == C || A == D) continue;
        float arg1, arg2;
        if (!lines_cross_args(A, B, C, D, &arg1, &arg2)) continue;
        if (!(arg1 > 0.0f && arg1 < 1.0f && arg2 > 0.0f && arg2 < 1.0f)) continue;
        /*
        arg1 = fabsf(arg1 - 0.5f) * 2;
        arg2 = fabsf(arg2 - 0.5f) * 2;
        if (!(arg1 > 0.8f && arg1 < 1.0f && arg2 > 0.8f && arg2 < 1.0f)) continue;
        */
        DEBUG_POINTF2("untangle p%i and p%i", B->index, C->index);
        A->next_in_contour = C;
        C->next_in_contour = B;
        B->next_in_contour = D;
        C->prev_in_contour = A;
        B->prev_in_contour = C;
        D->prev_in_contour = B;
    }

    return MESHER_DONE;
}

int mesher(mesher_t *m, int deep)
{
    int res = fix_contours_bugs(m);
    if (res != MESHER_DONE) return res;

    int nobjects = prepare_triangulation_objects(m);
    if (nobjects <= 0) FAILED("get_triangulation_objects");

    for (int object = 0; object < nobjects; object++)
    {
        /* Триангуляция без ограничений. */
        /* Создаёт выпуклую триангуляцию на всём множестве точек */
        res = sweep_points(m, object);
        if (res != MESHER_DONE) return res;

        /* Оптимизация сетки */
        res = optimize_all(m, deep, object);
        if (res != MESHER_DONE) return res;

        /* Вставка структурных отрезков */
        res = handle_constraints(m, object);
        if (res != MESHER_DONE) return res;

        /* Удаление лишних треугольников */
        res = remove_excess_triangles(m);
        if (res != MESHER_DONE) return res;

        /* Оптимизация сетки */
        res = optimize_all(m, deep, object);
        if (res != MESHER_DONE) return res;
    }
    return MESHER_DONE;
}

int ttf_glyph2mesh(ttf_glyph_t *glyph, ttf_mesh_t **output, uint8_t quality, int features)
{
    ttf_outline_t *o;
    mesher_t *mesh;
    ttf_mesh_t *out;
    int res;

    *output = NULL;
    if (glyph->outline == NULL)
        return TTF_ERR_NO_OUTLINE;

    if (quality < 8) quality = 8;
    if (quality > 128) quality = 128;

    /* Создаём outline и mesher */
    o = ttf_linear_outline(glyph, quality);
    if (o == NULL) return TTF_ERR_NOMEM;
    if (o->total_points < 3)
    {
        ttf_free_outline(o);
        return TTF_ERR_NO_OUTLINE;
    }
    mesh = create_mesher(o);
    if (mesh == NULL)
    {
        ttf_free_outline(o);
        return TTF_ERR_NOMEM;
    }

    /* Запускаем mesher */
    res = mesher(mesh, 128);
    if (res == MESHER_FAIL) goto failed;
    if (res == MESHER_WARN && (features & TTF_FEATURE_IGN_ERR) == 0) goto failed;

    /* Считаем число треугольников */
    int nt = 0;
    for (mts_t *t = mesh->tused.next; t != &mesh->tused; t = t->next)
        nt++;

    /* Создаём выходной объект */
    out = (ttf_mesh_t *)calloc(
        /* this */  sizeof(ttf_mesh_t) +
        /* vert */  mesh->nv * 2 * sizeof(float) +
        /* faces */ nt * 3 * sizeof(int), 1);
    if (out == NULL)
    {
        ttf_free_outline(o);
        free_mesher(mesh);
        return TTF_ERR_NOMEM;
    }
    out->nvert = mesh->nv;
    out->nfaces = 0;
    out->outline = o;
    *(void **)&out->vert = out + 1;
    *(void **)&out->faces = &out->vert[out->nvert];

    /* Заполняем выходной объект */
    for (int i = 0; i < mesh->nv; i++)
    {
        out->vert[i].x = mesh->v[i].x;
        out->vert[i].y = mesh->v[i].y;
    }
    for (mts_t *t = mesh->tused.next; t != &mesh->tused; t = t->next)
    {
        mvs_t *v1, *v2, *v3;
        float d1[2], d2[2];
        v1 = EDGES_COMMON_VERT(t->edge[0], t->edge[1]);
        v2 = EDGES_COMMON_VERT(t->edge[1], t->edge[2]);
        v3 = EDGES_COMMON_VERT(t->edge[2], t->edge[0]);
        VECSUB(d1, &v2->x, &v1->x);
        VECSUB(d2, &v3->x, &v2->x);
        if (VECCROSS(d1, d2) < 0)
            SWAP(mvs_t *, v2, v3);
        out->faces[out->nfaces].v1 = v1 - mesh->v;
        out->faces[out->nfaces].v2 = v2 - mesh->v;
        out->faces[out->nfaces].v3 = v3 - mesh->v;
        out->nfaces++;
    }

    *output = out;
    free_mesher(mesh);
    return TTF_DONE;

failed:
    ttf_free_outline(o);
    free_mesher(mesh);
    return TTF_ERR_MESHER;
}

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
/************************************ END *************************************/
/*********************************** MESHER ***********************************/
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

int ttf_export_to_obj(ttf_t *ttf, const char *file_name, uint8_t quality)
{
    FILE *f = fopen(file_name, "wb");
    if (f == NULL) return TTF_ERR_OPEN;
    if (fprintf(f, "# File generated by ttf2mesh %s\n", TTF2MESH_VERSION) == 0) goto werror;
    if (fprintf(f, "# Font full name: %s\n", ttf->info.full_name) == 0) goto werror;
    if (fprintf(f, "# Font family, subfamily: %s, %s\n", ttf->info.family, ttf->info.subfamily) == 0) goto werror;
    if (fprintf(f, "# Export quality parameter: %i\n\n", (int)quality) == 0) goto werror;
    int vtotal = 0;
    int ttotal = 0;
    int writed = 0;
    int errors = 0;
    for (int i = 0; i < ttf->nchars; i++)
    {
        ttf_glyph_t *g = ttf->glyphs + ttf->char2glyph[i];
        if (fprintf(f, "o symbol U+%04X glyph %i xadv %.3f lsb %.3f rsb %.3f\n",
                    (int)ttf->chars[i], (int)ttf->char2glyph[i], g->advance,
                    g->lbearing, g->rbearing) == 0) goto werror;
        bool has_data = g->outline != NULL;
        if (has_data) has_data = g->outline->total_points >= 3;
        if (!has_data)
        {
            if (fprintf(f, "# No points in this glyph\n\n") == 0) goto werror;
            continue;
        }
        ttf_mesh_t *m;
        ttf_glyph2mesh(g, &m, quality, 0);
        if (m == NULL)
        {
            if (fprintf(f, "# Mesh building error\n\n") == 0) goto werror;
            errors++;
            continue;
        }
        for (int j = 0; j < m->nvert; j++)
            if (fprintf(f, "v %.4f %.4f 0.0\n", m->vert[j].x, m->vert[j].y) == 0)
                goto werror;
        for (int j = 0; j < m->nfaces; j++)
            if (fprintf(f, "f %i %i %i\n",
                        m->faces[j].v1 + vtotal + 1,
                        m->faces[j].v2 + vtotal + 1,
                        m->faces[j].v3 + vtotal + 1) == 0)
                goto werror;
        writed++;
        vtotal += m->nvert;
        ttotal += m->nfaces;
        if (fprintf(f, "# %i vertices %i triangles\n\n", m->nvert, m->nfaces) == 0)
            goto werror;
        ttf_free_mesh(m);
    }
    if (fprintf(f, "# export finished\n") == 0) goto werror;
    if (fprintf(f, "# %i glyphs exported\n", writed) == 0) goto werror;
    if (fprintf(f, "# %i glyphs unable to export\n", errors) == 0) goto werror;
    if (fprintf(f, "# %i total triangles\n", ttotal) == 0) goto werror;
    if (fprintf(f, "# %i total vertices\n", vtotal) == 0) goto werror;
    fclose(f);
    return TTF_DONE;

werror:
    fclose(f);
    return TTF_ERR_WRITING;
}

void ttf_free_outline(ttf_outline_t *outline)
{
    free(outline);
}

void ttf_free_mesh(ttf_mesh_t *mesh)
{
    if (mesh == NULL) return;
    free(mesh->outline);
    free(mesh);
}

void ttf_free_list(ttf_t **list)
{
    if (list == NULL) return;
    for (int i = 0; list[i] != NULL; i++)
        ttf_free(list[i]);
    free(list);
}

void ttf_free(ttf_t *ttf)
{
    int i;
    if (ttf == NULL) return;
    free(ttf->chars);
    if (ttf->glyphs)
        for (i = 0; i < ttf->nglyphs; i++)
            ttf_free_outline(ttf->glyphs[i].outline);
    #define free_string(s) if (s != empty_string) free((char *)s)
    free_string(ttf->filename         );
    free_string(ttf->info.copyright   );
    free_string(ttf->info.family      );
    free_string(ttf->info.subfamily   );
    free_string(ttf->info.unique_id   );
    free_string(ttf->info.full_name   );
    free_string(ttf->info.version     );
    free_string(ttf->info.ps_name     );
    free_string(ttf->info.trademark   );
    free_string(ttf->info.manufacturer);
    free_string(ttf->info.designer    );
    free_string(ttf->info.description );
    free_string(ttf->info.url_vendor  );
    free_string(ttf->info.url_designer);
    free_string(ttf->info.license_desc);
    free_string(ttf->info.locense_url );
    free_string(ttf->info.sample_text );
    free(ttf);
}

#ifdef __cplusplus
}
#endif
