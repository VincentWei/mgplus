/*
 ** $Id: graphics.h 10885 2008-12-05 12:01:39Z tangjianbin$
 **
 ** This file includes MPGraphics struct define. 
 **
 ** Copyright (C) 2003 ~ 2008 Feynman Software.
 ** Copyright (C) 2000 ~ 2002 Wei Yongming.
 **
 ** Create date: 2008/12/02
 */
#ifndef MGPLUS_GRAHPICS_H
#define MGPLUS_GRAPHICS_H

#include "agg_rendering_buffer.h"
#include "agg_rasterizer_scanline_aa.h"
#include "agg_scanline_p.h"
#include "agg_renderer_scanline.h"
#include "agg_conv_bspline.h"
#include "agg_conv_segmentator.h"
#include "agg_trans_single_path.h"
#include "agg_image_accessors.h"
#include "interactive_polygon.h"
#include "agg_span_allocator.h"
#include "agg_span_interpolator_linear.h"
#include "agg_span_image_filter_rgb.h"
#include "agg_span_image_filter_rgba.h"
#include "agg_span_image_filter_gray.h"
#include "agg_path_storage.h"
#include "agg_span_gouraud_rgba.h"
#include "agg_arc.h"
#include "agg_spline_ctrl.h"
#include "agg_trans_perspective.h"
#include "agg_basics.h"
#include "agg_pixfmt_gray.h"
#include "agg_alpha_mask_u8.h"
#include "agg_scanline_p.h"
#include "agg_platform_support.h"

#include "mgplus.h"
#include "brush.h"
#include "pen.h"

#ifdef __cplusplus
extern "C" {
#endif

/* hdc is external */
#define MP_HDC_EXTERNAL 0
/* hdc is create by self */
#define MP_HDC_PRIVATE  1

#define COMP_DRAW       1

#define MAX_BMP_NUM     10
#define MPGetBValue(rgba)      ((BYTE)(rgba))
#define MPGetGValue(rgba)      ((BYTE)(((DWORD)(rgba)) >> 8))
#define MPGetRValue(rgba)      ((BYTE)((DWORD)(rgba) >> 16))
#define MPGetAValue(rgba)      ((BYTE)((DWORD)(rgba) >> 24))

#define RGB2PIXEL565(r,g,b)    \
         ((((r) & 0xf8) << 8) | (((g) & 0xfc) << 3) | (((b) & 0xf8) >> 3))

/* houhh 20090224, copy this code from minigui.
 * src/newgal/blit.h. */

/* This is a very useful loop for optimizing blitters */
#define USE_DUFFS_LOOP
#ifdef USE_DUFFS_LOOP

/* 8-times unrolled loop */
#define DUFFS_LOOP8(pixel_copy_increment, width)                \
{ int n = (width+7)/8;                                          \
    switch (width & 7) {                                        \
    case 0: do {    pixel_copy_increment;                       \
    case 7:        pixel_copy_increment;                        \
    case 6:        pixel_copy_increment;                        \
    case 5:        pixel_copy_increment;                        \
    case 4:        pixel_copy_increment;                        \
    case 3:        pixel_copy_increment;                        \
    case 2:        pixel_copy_increment;                        \
    case 1:        pixel_copy_increment;                        \
        } while ( --n > 0 );                                    \
    }                                                           \
}

/* 4-times unrolled loop */
#define DUFFS_LOOP4(pixel_copy_increment, width)                \
{ int n = (width+3)/4;                                          \
    switch (width & 3) {                                        \
    case 0: do {    pixel_copy_increment;                       \
    case 3:        pixel_copy_increment;                        \
    case 2:        pixel_copy_increment;                        \
    case 1:        pixel_copy_increment;                        \
        } while ( --n > 0 );                                    \
    }                                                           \
}

/* Use the 8-times version of the loop by default */
#define DUFFS_LOOP(pixel_copy_increment, width)                 \
    DUFFS_LOOP8(pixel_copy_increment, width)

#else

/* Don't use Duff's device to unroll loops */
#define DUFFS_LOOP(pixel_copy_increment, width)                 \
{ int n;                                                        \
    for ( n=width; n > 0; --n ) {                               \
        pixel_copy_increment;                                   \
    }                                                           \
}

#define DUFFS_LOOP8(pixel_copy_increment, width)                \
    DUFFS_LOOP(pixel_copy_increment, width)

#define DUFFS_LOOP4(pixel_copy_increment, width)                \
    DUFFS_LOOP(pixel_copy_increment, width)

#endif /* USE_DUFFS_LOOP */

typedef struct _MPGraphics MPGraphics;

typedef struct _ctx_copy_
{
    MPGraphics *src;
    MPGraphics *dst;
    BOOL        blend;
}CTX_COPY;

typedef struct _ctx_clear_
{
    MPGraphics* gps;
    ARGB        color;
}CTX_CLEAR;

typedef struct _ctx_draw_path_
{
    MPGraphics* gps;
    MPPen*      pen;
    MPPath*     path;
    agg::trans_affine mot;/*matrix of transform*/
}CTX_DRAW_PATH;

typedef struct _ctx_fill_path_
{
    MPGraphics* gps;
    MPBrush*    brush;
    MPPath*     path;
    agg::trans_affine mot;/*matrix of transform*/
}CTX_FILL_PATH;

typedef struct _ctx_image_pts_
{
    int         idx;
    int         count;
    MPGraphics* gps;
    MPPOINT*    pt;
    agg::trans_affine mot;/*matrix of transform*/
}CTX_IMAGE_PTS;

typedef struct _ctx_image_path_
{
    int         idx;
    MPGraphics* gps;
    MPPath*     path;
    RECT*       rc;
    agg::trans_affine mot;/*matrix of transform*/
}CTX_IMAGE_PATH;

#ifdef _MGPLUS_FONT_FT2
typedef struct _ctx_draw_glyph_
{
    MPGraphics* gps;
    MPFont*     font;
    int         x;
    int         y;
    LPGLYPHDATA lpdata;
    ARGB        color;
    agg::trans_affine mot;/*matrix of transform*/
} CTX_DRAW_GLYPH;
#endif

typedef struct _agg_draw_op {
    void (*copy)(HDC hdc, Uint8* pixels, int pitch, int bpp, 
            const RECT* rc, void* ctx);
    void (*clear)(HDC hdc, Uint8* pixels, int pitch, int bpp, 
            const RECT* rc, void* ctx);
    void (*draw_path)(HDC hdc, Uint8* pixels, int pitch, int bpp, 
            const RECT* rc, void* ctx);
    void (*fill_path)(HDC hdc, Uint8* pixels, int pitch, int bpp, 
            const RECT* rc, void* ctx);
    void (*draw_image_with_pts)(HDC hdc, Uint8* pixels, int pitch, int bpp, 
            const RECT* rc, void* ctx);
    void (*draw_image_with_path)(HDC hdc, Uint8* pixels, int pitch, int bpp, 
            const RECT* rc, void* ctx);
#ifdef _MGPLUS_FONT_FT2
    void (*draw_glyph)(HDC hdc, Uint8* pixels, int pitch, int bpp, 
            const RECT* rc, void* ctx);
#endif
} agg_draw_op;

typedef struct _agg_clip_ras {
    bool state;
    unsigned char* buf;
    MPPath* path;
    agg::rendering_buffer ren_rbuf;
    agg::alpha_mask_gray8 alpha_mask;
    agg::rasterizer_scanline_aa<> ras;
} agg_clip_ras;

struct _MPGraphics
{
    /* the hint. */
    MPRenderingControl      render_control;
    MPTextRenderingHint     text_hints;
    MPPathRenderingHint     path_hints;
    /* draw quality, anti-aa control. */
    MPSmoothingMode         smoothing_mode; 
    /* composite quality. */
    MPInterpolationMode     interpolation_mode;
    /* color composite mode. */
    MPCompositingMode       compositing_mode;
    /* the width ad height of agg's internal buff */
    DWORD                   width, height;     
    HDC                     hdc; 
    /* 32bpp mem dc to load bitmap */
    HDC                     img_dc;
    DWORD                   hdc_flags;
    unsigned char*          hdc_addr;
    /* translate matrix. */
    agg::trans_affine       matrix_gfx;
    /* AGG rendering buff. */
    agg::rendering_buffer   rendering_buff;
    /* graphic class max render_buff pointer*/
    agg::rendering_buffer   rendering_img [MAX_BMP_NUM];
    /* graphic class max bitmap data*/
    PBITMAP                 surf_img [MAX_BMP_NUM];
    agg_draw_op*            op;
    agg_clip_ras            clip_ras;
    int                     img_alpha;
};

typedef struct _MPSaveParam
{
   int start_x;
   int start_y;
   int sx;
   int sy;
   unsigned char* src_addr;
   int src_pitch;
   int src_bytes_per_pixel;
   MPGraphics *pg;
}MPSaveParam;


#if 0
typedef enum _MPTextRenderingHint
{
    // Glyph with system default rendering hint
    MP_TEXT_RENDERING_HINT_SYSTEM_DEFAULT = 0,               //reserved           
    // Glyph bitmap with hinting
    MP_TEXT_RENDERING_HINT_SINGLE_BITPERPIXEL_GRIDFIT = 1,   //reserved   
    // Glyph bitmap without hinting
    MP_TEXT_RENDERING_HINT_SINGLE_BITPERPIXEL = 2,           //reserved
    // Glyph anti-alias bitmap with hinting
    MP_TEXT_RENDERING_HINT_ANTIALIAS_GRIDFIT = 3,            //reserved
    // Glyph anti-alias bitmap without hinting
    MP_TEXT_RENDERING_HINT_ANTIALIAS = 4,                    //reserved
    // Glyph CT bitmap with hinting
    MP_TEXT_RENDERING_HINT_CLEARTYPE_GRIDFIT = 5             //reserved
}MPTextRenderingHint;

typedef enum _MPCompositingQuality
{
    //Compositing quality invalid
    MP_COMPOSITING_QUALITY_Invalid=-1,        //reserved
    //Compositing quality with default
    MP_COMPOSITING_QUALITY_DEFAULT=0,         //reserved
    //Compositing quality with speed
    MP_COMPOSITING_QUALITY_HIGH_SPEED=1,      //reserved
    //Compositing quality with quality
    MP_COMPOSITING_QUALITY_HIGH_QUALITY=2,    //reserved
    //Compositing quality with gamma
    MP_COMPOSITING_QUALITY_GAMMA_CORRECTED=3, //reserved
    //Compositing quality with linear
    MP_COMPOSITING_QUALITY_ASSUME_LINEAR      //reserved
}MPCompositingQuality;

typedef enum _MGPlusPixFormat
{
    MP_PIX_FORMAT_UNDEFINED = 0,
    //Simple 256 level grayscale
    MP_PIX_FORMAT_GRAY8 = 1,  
    //Simple 65535 level grayscale
    MP_PIX_FORMAT_GRAY16 = 2, 
    //15 bit rgb. Depends on the byte ordering!
    MP_PIX_FORMAT_RGB555 = 3, 
    //16 bit rgb. Depends on the byte ordering!
    MP_PIX_FORMAT_RGB565 = 4, 
    //R-G-B, one byte per color component
    MP_PIX_FORMAT_RGB24 = 5,  
    //B-G-R, native win32 BMP format.
    MP_PIX_FORMAT_BGR24 = 6,  
    //R-G-B-A, one byte per color component
    MP_PIX_FORMAT_RGBA32 = 7, 
    //A-R-G-B, native MAC format
    MP_PIX_FORMAT_ARGB32 = 8, 
    //A-B-G-R, one byte per color component
    MP_PIX_FORMAT_ABGR32 = 9, 
    //B-G-R-A, native win32 BMP format
    MP_PIX_FORMAT_BGRA32 = 10,
    MP_PIX_FORMAT_END
}MGPlusPixFormat;

typedef enum _MPWrapMode { /*just fill the image as normal*/
    MP_WRAP_MODE_TILE,         
    /*fill the image on the horizontal at the next col*/
    MP_WRAP_MODE_TILE_FLIP_X,  
    /*fill the image on the vertical at the next row*/
    MP_WRAP_MODE_TILE_FLIP_Y,  
    /*fill the image on the horizontal and vertical step by step*/
    MP_WRAP_MODE_TILE_FLIP_XY, 
    MP_WRAP_MODE_TILE_FLIP = MP_WRAP_MODE_TILE_FLIP_XY,
    /*fill the image on the center*/
    MP_WRAP_MODE_CLAMP         
}MPWrapMode;

typedef enum _MPHatchStyle
{
    /*draw the horizontal line*/
    MP_HATCH_STYLE_HORIZONTAL = 0,       
    /*draw the vertical line*/
    MP_HATCH_STYLE_VERTICAL = 1,         
    /*draw the forwarddiagonal line*/
    MP_HATCH_STYLE_FORWARDDIAGONAL = 2,  
    /*draw the backwarddiagonal line*/
    MP_HATCH_STYLE_BACKWARDDIAGONAL = 3, 
    /*draw the cross line*/
    MP_HATCH_STYLE_CROSS = 4,            
    /*draw the diagonal cross line*/
    MP_HATCH_STYLE_DIAGONALCROSS = 5,    
}MPHatchStyle;

#endif


#ifdef __cplusplus
}
#endif
#endif
