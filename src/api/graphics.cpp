/*
 *   This file is part of mGPlus, a component for MiniGUI.
 * 
 *   Copyright (C) 2008~2018, Beijing FMSoft Technologies Co., Ltd.
 * 
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 * 
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 * 
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 * 
 *   Or,
 * 
 *   As this program is a library, any link to this program must follow
 *   GNU General Public License version 3 (GPLv3). If you cannot accept
 *   GPLv3, you need to be licensed from FMSoft.
 * 
 *   If you have got a commercial license of this program, please use it
 *   under the terms and conditions of the commercial license.
 * 
 *   For more information about the commercial license, please refer to
 *   <http://www.minigui.com/en/about/licensing-policy/>.
 */
/*
 ** graphics.cpp: Implementation of draw and fill path, and MGPlusGraphic. 
 ** This file includes macro definitions, typedefs and function interfaces of 
 ** mgplus component. mGPlus includes path, gradient filling and color composite.
 **
 ** Create date: 2008/12/02
 */
#include <string.h>
#include <math.h>
#include <assert.h>

#ifdef _MGGAL_NEXUS
#   define CFG_HARDWARE_SURFACE
#endif
#define CFG_ARGB

#include "agg_renderer_base.h"
#include "agg_conv_stroke.h"
#include "agg_path_storage.h"
#include "agg_rasterizer_scanline_aa.h"
#include "agg_scanline_u.h"
#include "agg_renderer_scanline.h"
#include "agg_trans_bilinear.h"
#include "agg_ellipse.h"
#include "agg_image_accessors.h"
#include "agg_gamma_lut.h"
#include "agg_gradient_lut.h"
#include "agg_bounding_rect.h"

#include "mgplus.h"
#include "pen.h"
#include "path.h"
#include "graphics.h"

#include "agg_span_gradient.h"
#include "agg_platform_support.h"
#include "agg_gamma_ctrl.h"
#include <math.h>
#include <assert.h>
#ifndef _MGRM_THREADS
#include <pthread.h>
#endif


#define DRAW_GRAPHIC(hdc, rgn, pctx, cb) do {\
        if (LockDCEx (hdc, rgn, pctx, cb)) UnlockDC(hdc); \
    }while(0)

#define SET_USE_DEVRC(hdc, rc) SetRect(&rc, 0, 0,   \
                    GetGDCapability (hdc, GDCAP_MAXX)+1, GetGDCapability (hdc, GDCAP_MAXY)+1);

#define GET_GDCAP_MAXX(hdc) GetGDCapability (hdc, GDCAP_MAXX)+1
#define GET_GDCAP_MAXY(hdc) GetGDCapability (hdc, GDCAP_MAXY)+1

/* houhh 20090512, draw_argb32 is must need.*/
extern agg_draw_op draw_argb32;
#ifdef _MGPLUS_PIXFMT_RGBA32
extern agg_draw_op draw_rgba32;
#endif
#ifdef _MGPLUS_PIXFMT_ABGR32
extern agg_draw_op draw_abgr32;
#endif
#ifdef _MGPLUS_PIXFMT_BGRA32
extern agg_draw_op draw_bgra32;
#endif
#ifdef _MGPLUS_PIXFMT_RGB24
extern agg_draw_op draw_rgb24;
#endif
#ifdef _MGPLUS_PIXFMT_BGR24
extern agg_draw_op draw_bgr24;
#endif
#ifdef _MGPLUS_PIXFMT_RGB555
extern agg_draw_op draw_rgb555;
#endif
#ifdef _MGPLUS_PIXFMT_RGB565
extern agg_draw_op draw_rgb565;
#endif

enum MP_PIXFMT
{
    MP_PIX_ARGB32,
    MP_PIX_RGBA32,
    MP_PIX_ABGR32,
    MP_PIX_BGRA32,
    MP_PIX_RGB24,
    MP_PIX_BGR24,
    MP_PIX_RGB555,
    MP_PIX_RGB565,
    MP_PIX_MAX
};

typedef struct _pixfmt_op_
{
    unsigned int    rmask;
    unsigned int    gmask;
    unsigned int    bmask;
    int             bpp;
    MP_PIXFMT       fmt;
    agg_draw_op*    op;
}PIXFMT_OP;

PIXFMT_OP pixfmt_op[]=
{
// #ifdef _MGPLUS_PIXFMT_ARGB32
    { 0xFF00, 0xFF0000, 0xFF000000, 4, MP_PIX_ARGB32, &draw_argb32},
// #endif
#ifdef _MGPLUS_PIXFMT_RGBA32
    { 0xFF, 0xFF00, 0xFF0000, 4, MP_PIX_RGBA32, &draw_rgba32},
#endif
#ifdef _MGPLUS_PIXFMT_ABGR32
    { 0xFF000000, 0xFF0000, 0xFF00, 4, MP_PIX_ABGR32, &draw_abgr32},
#endif
#ifdef _MGPLUS_PIXFMT_BGRA32
    { 0xFF0000, 0xFF00, 0xFF, 4, MP_PIX_BGRA32, &draw_bgra32},
#endif
#ifdef _MGPLUS_PIXFMT_RGB24
    { 0xFF, 0xFF00, 0xFF0000, 3, MP_PIX_BGR24, &draw_rgb24},
#endif
#ifdef _MGPLUS_PIXFMT_BGR24
    { 0xFF0000, 0xFF00, 0xFF, 3, MP_PIX_BGR24, &draw_bgr24},
#endif
#ifdef _MGPLUS_PIXFMT_RGB555
    { 0x7C00, 0x3E0, 0x1F, 2, MP_PIX_RGB555, &draw_rgb555},
#endif
#ifdef _MGPLUS_PIXFMT_RGB565
    { 0xF800, 0x7E0, 0x1F, 2, MP_PIX_RGB565, &draw_rgb565},
#endif
};

static agg_draw_op* get_pixfmt_op(HDC hdc)
{
    unsigned int rmask;
    unsigned int gmask;
    unsigned int bmask;
    unsigned int i;
    int bpp;

    rmask = GetGDCapability (hdc, GDCAP_RMASK);
    gmask = GetGDCapability (hdc, GDCAP_GMASK);
    bmask = GetGDCapability (hdc, GDCAP_BMASK);
    bpp = GetGDCapability (hdc, GDCAP_BPP);

    for(i = 0; i < TABLESIZE(pixfmt_op); i++) {
        if (pixfmt_op[i].rmask == rmask &&
            pixfmt_op[i].gmask == gmask &&
            pixfmt_op[i].bmask == bmask && 
            pixfmt_op[i].bpp == bpp)
            return pixfmt_op[i].op;
    }
    return NULL;
}

static Uint8* GetDCPixels(HDC hdc)
{
    Uint8*  pixels=NULL;
    int     pitch;
    POINT   pt={0, 0};
    int     bpp = GetGDCapability (hdc, GDCAP_BPP);

    LPtoSP(hdc, &pt);
    
    pixels = LockDC(hdc, NULL, NULL, NULL, &pitch);
    UnlockDC(hdc);
    pixels -= pt.y * pitch;
    pixels -= pt.x * bpp;

    return pixels;
}

HGRAPHICS 
MGPlusGraphicCreateWithoutCanvas(HDC hdc)
{
    int width, height;
    int swidth, sheight;
    int pitch = 0;
    MPGraphics *pg;
    agg_draw_op *op;

    if (hdc == HDC_INVALID)
        return (HGRAPHICS)MP_INV_HANDLE;

    width  = GET_GDCAP_MAXX(hdc);
    height = GET_GDCAP_MAXY(hdc);

    if (width <= 0 || height <= 0)
        return (HGRAPHICS)MP_INV_HANDLE;

    op = get_pixfmt_op(hdc);
    if (!op)
        return MP_INV_HANDLE;

    pg = new MPGraphics;

    if (!pg)
        return MP_INV_HANDLE;

    /* width and height of screen */
    swidth  = GET_GDCAP_MAXX(HDC_SCREEN);
    sheight = GET_GDCAP_MAXY(HDC_SCREEN);

    pitch = GetGDCapability (hdc, GDCAP_PITCH);
    pg->hdc_addr = GetDCPixels(hdc);
    pg->rendering_buff.attach(pg->hdc_addr, swidth, sheight, pitch);

    pg->hdc = hdc;
    pg->hdc_flags           = MP_HDC_EXTERNAL;
    pg->text_hints          = MP_TEXT_RENDER_HINT_ANTIALIAS_ON;
    pg->path_hints          = MP_PATH_RENDER_HINT_ANTIALIAS_ON;
    pg->smoothing_mode      = MP_SMOOTHING_QUALITY;
    pg->interpolation_mode  = MP_INTERPOLATION_MODE_PERSPECTIVE;
    pg->compositing_mode    = MP_COMP_OP_SRC_OVER;
    
    pg->width  = width; 
    pg->height = height; 
    pg->matrix_gfx.reset();

    pg->op = op; 
    pg->img_alpha = 255;

    /* create mem dc of bgra32 pixfmt, 
     * 50x20 size is to reduce space be used */
    pg->img_dc = CreateMemDC (50, 20, 32, 
                                MEMDC_FLAG_SWSURFACE,
                                0xFF00,     /*rmask*/
                                0xFF0000,   /*gmask*/
                                0xFF000000, /*bmask*/
                                0xFF);      /*amask*/

    memset(pg->surf_img, 0, sizeof(pg->surf_img));

    /* init clip_ras.*/
    pg->clip_ras.state = false;
    pg->clip_ras.buf  = NULL;
    pg->clip_ras.path = NULL;

    return (HGRAPHICS)pg;
}

MPStatus 
MGPlusGraphicCopyFromDC (HGRAPHICS graphic, HDC hdc, int sx,
            int sy, int sw, int sh, int dx, int dy)
{
    MPGraphics* pgraphic = (MPGraphics*) graphic;

    if (!pgraphic)
        return MP_GENERIC_ERROR;

    if (sx < 0) {
        sw = sw + sx;
        sx = 0;
    }

    if (sy < 0) {
        sh = sh + sy;
        sy = 0;
    }

    if (dx < 0) {
        sx = sx + dx;
        dx = 0;
    }

    if (dy < 0) {
        sy = sy + dy;
        dy = 0;
    }

    BitBlt (hdc, sx, sy, sw, sh, pgraphic->hdc, dx, dy, 0);
    return MP_OK;
}

HGRAPHICS 
MGPlusGraphicCreateFromDC (HDC hdc)
{
    int width, height;
    HGRAPHICS pgraphic;

    width  = GET_GDCAP_MAXX(hdc) + 1;
    height = GET_GDCAP_MAXY(hdc) + 1;

    if (width <= 0 || height <= 0)
        return (HGRAPHICS)MP_INV_HANDLE;

    pgraphic = MGPlusGraphicCreate (width, height);

    if (pgraphic)
        MGPlusGraphicCopyFromDC (pgraphic, hdc, 0, 0, 0, 0, 0, 0);

    return pgraphic;
}


HGRAPHICS 
MGPlusGraphicCreate (int width, int height)
{
    unsigned int m_bpp;
    unsigned int m_rmask;
    unsigned int m_gmask;
    unsigned int m_bmask;
    unsigned int m_amask;
    int pitch = 0;
    int i = 0;

    if (width <= 0 || height <= 0)
        return MP_INV_HANDLE;

    MPGraphics *pgraphic = new MPGraphics;
    if (!pgraphic)
        return MP_INV_HANDLE;

#ifndef CFG_ARGB
    m_amask = 0x000000FF;
    m_rmask = 0x0000FF00;
    m_gmask = 0x00FF0000;
    m_bmask = 0xFF000000;
#else
    m_amask = 0xFF000000;
    m_rmask = 0x00FF0000;
    m_gmask = 0x0000FF00;
    m_bmask = 0x000000FF;
#endif
    m_bpp   = 32;


#ifdef CFG_HARDWARE_SURFACE
    pgraphic->hdc = CreateMemDC (width, height, m_bpp, MEMDC_FLAG_HWSURFACE,
                    m_rmask, m_gmask, m_bmask, m_amask);
#else
    pgraphic->hdc = CreateMemDC (width, height, m_bpp, MEMDC_FLAG_SWSURFACE,
                    m_rmask, m_gmask, m_bmask, m_amask);
#endif

    if( pgraphic->hdc == HDC_INVALID){
        delete(pgraphic);
        return MP_INV_HANDLE;
    }

    pitch = width * (m_bpp/8);

    pgraphic->hdc_addr = (unsigned char*)LockDC(pgraphic->hdc, 
            NULL, NULL, NULL, NULL);
    pgraphic->rendering_buff.attach(pgraphic->hdc_addr, width, height, pitch);
    UnlockDC(pgraphic->hdc);

    pgraphic->hdc_flags           = MP_HDC_PRIVATE;
    pgraphic->smoothing_mode      = MP_SMOOTHING_QUALITY;
    pgraphic->text_hints          = MP_TEXT_RENDER_HINT_ANTIALIAS_ON;
    pgraphic->path_hints          = MP_PATH_RENDER_HINT_ANTIALIAS_ON;
    pgraphic->interpolation_mode  = MP_INTERPOLATION_MODE_PERSPECTIVE;
    pgraphic->compositing_mode    = MP_COMP_OP_SRC_OVER;
    pgraphic->width  = width; 
    pgraphic->height = height; 
    pgraphic->matrix_gfx.reset ();
    pgraphic->op = get_pixfmt_op(pgraphic->hdc);
    if (pgraphic->op == NULL)
    {
        delete(pgraphic);
        assert(0);
        return MP_INV_HANDLE;
    }

    pgraphic->img_alpha = 255;

    /* init clip_ras.*/
    pgraphic->clip_ras.state = false;
    pgraphic->clip_ras.buf = NULL;
    pgraphic->clip_ras.path = NULL;

    while (i < MAX_BMP_NUM) {
        pgraphic->surf_img [i] = NULL;
        i ++;
    }
    return (HGRAPHICS)pgraphic;
}

static inline gal_pixel _mem_get_pixel (Uint8 *dst, int bpp)
{
    switch (bpp) {
        case 1:
            return *dst;
        case 2:
            return *(Uint16*)dst;
        case 3:
        {
            gal_pixel pixel;
#if MGUI_BYTEORDER == MGUI_LIL_ENDIAN
            pixel = dst[0] + (dst[1] << 8) + (dst[2] << 16);
#else
            pixel = (dst[0] << 16) + (dst[1] << 8) + dst[2];
#endif
            return pixel;
        }
        case 4:
            return *(Uint32*)dst;
    }

    return 0;
}

static inline BYTE* _mem_set_pixel (BYTE* dst, int bpp, Uint32 pixel)
{
    switch (bpp) {
        case 1:
            *dst= pixel;
            break;
        case 2:
            *(Uint16 *) dst = pixel;
            break;
        case 3:
#if MGUI_BYTEORDER == MGUI_LIL_ENDIAN
            dst [0] = pixel;
            dst [1] = pixel >> 8;
            dst [2] = pixel >> 16;
#else
            dst [0] = pixel >> 16;
            dst [1] = pixel >> 8;
            dst [2] = pixel;
#endif
            break;
        case 4:
            *(Uint32 *) dst = pixel;
            break;
    }

    return dst + bpp;
}

#define direct_graphic_save_copy_pixel   do{\
       Uint8 *dst_pixel, *src_pixel;\
       gal_pixel pixel;\
       Uint8 r, g, b, a;\
       dst_pixel = dst + (dst_rc->left + step) * dst_bytes_per_pixel;\
       src_pixel = src + step * ctxt->src_bytes_per_pixel;\
       pixel = _mem_get_pixel(src_pixel, ctxt->src_bytes_per_pixel);\
       Pixel2RGBA(ctxt->pg->hdc, pixel, &r, &g, &b, &a);\
       pixel = RGBA2Pixel(dst_hdc, r, g, b, a);\
       _mem_set_pixel(dst_pixel, dst_bytes_per_pixel, pixel);\
	   step++;} while(0)

static void 
direct_graphic_save (HDC dst_hdc, Uint8* dst_pixels,
        int dst_pitch, int dst_bytes_per_pixel, const RECT* dst_rc, void* content)
{
   MPSaveParam* ctxt = (MPSaveParam*) content;
   Uint8* dst = dst_pixels;
   Uint8* src = ctxt->src_addr;
   int off_x = dst_rc->left - ctxt->start_x;
   int off_y = dst_rc->top  - ctxt->start_y;
   int height = RECTHP(dst_rc), width  = RECTWP(dst_rc);

   src = src + (off_y + ctxt->sy) * ctxt->src_pitch + \
         (ctxt->sx+ off_x) * ctxt->src_bytes_per_pixel;
   while ( height-- ) {
       int step = 0;
#ifdef WIN32
       DUFFS_LOOP(direct_graphic_save_copy_pixel
		   /*
           (
               {
                   Uint8 *dst_pixel, *src_pixel;
                   gal_pixel pixel;
                   Uint8 r, g, b, a;
                   dst_pixel = dst + (dst_rc->left + step) * dst_bytes_per_pixel;
                   src_pixel = src + step * ctxt->src_bytes_per_pixel;

                   pixel = _mem_get_pixel(src_pixel, ctxt->src_bytes_per_pixel);
                   Pixel2RGBA(ctxt->pg->hdc, pixel, &r, &g, &b, &a);
                   pixel = RGBA2Pixel(dst_hdc, r, g, b, a);

                   _mem_set_pixel(dst_pixel, dst_bytes_per_pixel, pixel);
                   step++;
               }
           )*/, 
           width);
#else
       DUFFS_LOOP(
           (
               {
                   Uint8 *dst_pixel, *src_pixel;
                   gal_pixel pixel;
                   Uint8 r, g, b, a;
                   dst_pixel = dst + (dst_rc->left + step) * dst_bytes_per_pixel;
                   src_pixel = src + step * ctxt->src_bytes_per_pixel;

                   pixel = _mem_get_pixel(src_pixel, ctxt->src_bytes_per_pixel);
                   Pixel2RGBA(ctxt->pg->hdc, pixel, &r, &g, &b, &a);
                   pixel = RGBA2Pixel(dst_hdc, r, g, b, a);

                   _mem_set_pixel(dst_pixel, dst_bytes_per_pixel, pixel);
                   step++;
               }
           ), 
           width);
#endif

       src += ctxt->src_pitch;
       dst += dst_pitch;
   }
}

MPStatus 
MGPlusGraphicSave (HGRAPHICS graphics, HDC hdc, 
        int sx, int sy, int sw, int sh, int dx, int dy)
{
    MPGraphics *pg = (MPGraphics *)graphics;
    if(!pg)
        return MP_GENERIC_ERROR;

    if( pg->hdc_flags == MP_HDC_PRIVATE) {
        RECT rect_dst, rect_src, rect_inter;

        if (sw <= 0) sw = GET_GDCAP_MAXX(pg->hdc) - sx;
        if (sh <= 0) sh = GET_GDCAP_MAXY(pg->hdc) - sy;

        SetRect (&rect_src, sx, sy, sx + sw, sy + sh);
        SetRect (&rect_dst, dx, dy, dx + sw, dy + sh);

        IntersectRect (&rect_inter, &rect_src, &rect_dst);

        PCLIPRGN cliprgn_dst = CreateClipRgn ();
        SetClipRgn (cliprgn_dst, &rect_dst);

        if (cliprgn_dst) {
            MPSaveParam ctx; 

            if (sx < 0) {
                dx = dx - sx;
                sx = 0;
            }
            if (sy < 0) {
                dy = dy - sy;
                sy = 0;
            }

            POINT point = {dx, dy};
            LPtoSP (hdc, &point);

            ctx.start_x = point.x;
            ctx.start_y = point.y;
            ctx.sx      = sx;
            ctx.sy      = sy;

            ctx.src_pitch = GetGDCapability (pg->hdc, GDCAP_PITCH);
            ctx.src_addr = pg->hdc_addr; 
            ctx.src_bytes_per_pixel     = GetGDCapability (pg->hdc, GDCAP_BPP);
            ctx.pg = pg;

            DRAW_GRAPHIC(hdc, cliprgn_dst, &ctx, direct_graphic_save);

            DestroyClipRgn (cliprgn_dst);

            return MP_OK;
        }
        DestroyClipRgn (cliprgn_dst);
    }

    return MP_GENERIC_ERROR;
}

HDC 
MGPlusGetGraphicDC (HGRAPHICS graphic)
{
    MPGraphics *pg = (MPGraphics *)graphic;
    if(!pg)
        return HDC_INVALID;

    return pg->hdc;
}

MPStatus 
MGPlusGraphicCopy (HGRAPHICS src_gs, HGRAPHICS dst_gs)
{
    CTX_COPY ctx;
    MPGraphics *src_pg = (MPGraphics *)src_gs;
    MPGraphics *dst_pg = (MPGraphics *)dst_gs;

    if(!src_pg || !dst_pg)
        return MP_GENERIC_ERROR;

    ctx.src = src_pg;
    ctx.dst = dst_pg;
    ctx.blend = 0;

    RECT rc;
    SetRect(&rc, 0, 0, 
            GET_GDCAP_MAXX(dst_pg->hdc), 
            GET_GDCAP_MAXY(dst_pg->hdc));

    PCLIPRGN region = CreateClipRgn ();
    SetClipRgn (region, &rc);

    DRAW_GRAPHIC(dst_pg->hdc, region, &ctx, dst_pg->op->copy);

    DestroyClipRgn(region);
    return MP_OK;
}

MPStatus 
MGPlusGraphicBlend (HGRAPHICS src_gs, HGRAPHICS dst_gs)
{
    CTX_COPY ctx;
    MPGraphics *src_pg = (MPGraphics *)src_gs;
    MPGraphics *dst_pg = (MPGraphics *)dst_gs;

    if(!src_pg || !dst_pg)
        return MP_GENERIC_ERROR;

    ctx.src = src_pg;
    ctx.dst = dst_pg;
    ctx.blend = TRUE;

    RECT rc;
    SetRect(&rc, 0, 0, 
            GET_GDCAP_MAXX(dst_pg->hdc), 
            GET_GDCAP_MAXY(dst_pg->hdc));

    PCLIPRGN region = CreateClipRgn ();
    SetClipRgn (region, &rc);

    DRAW_GRAPHIC(dst_pg->hdc, region, &ctx, dst_pg->op->copy);

    DestroyClipRgn(region);
    return MP_OK;
}

MPStatus 
MGPlusGraphicClearEx (HGRAPHICS graphics, RECT* rect, ARGB color)
{
    CTX_CLEAR ctx;
    MPGraphics *pg = (MPGraphics *)graphics;
    if(!pg)
        return MP_GENERIC_ERROR;

    RECT rc;
    SetRect(&rc, 0, 0, 
            GET_GDCAP_MAXX(pg->hdc), 
            GET_GDCAP_MAXY(pg->hdc));

    if (rect)
        IntersectRect (&rc, &rc, rect);

    PCLIPRGN region = CreateClipRgn ();
    SetClipRgn (region, &rc);

    ctx.gps = pg;
    ctx.color = color;

    DRAW_GRAPHIC(pg->hdc, region, &ctx, pg->op->clear);

    DestroyClipRgn(region);
    return MP_OK;
}

MPStatus 
MGPlusGraphicDelete (HGRAPHICS graphics)
{
    MPGraphics *pg = (MPGraphics *)graphics;
    int i = 0;
    if(!pg)
        return MP_GENERIC_ERROR;

    if( pg->hdc_flags == MP_HDC_PRIVATE)
        DeleteMemDC(pg->hdc);
    else
        DeleteMemDC(pg->img_dc);

    while (i < MAX_BMP_NUM) {
        if (pg->surf_img [i]) {
            delete pg->surf_img[i];
        }
        i ++;
    }

    if (pg->clip_ras.buf) {
        delete [] pg->clip_ras.buf; 
    }
    if (pg->clip_ras.path) {
        MGPlusPathDelete((HPATH)pg->clip_ras.path);
    }

#if 1
    delete pg;
#else
    free(pg);
#endif
    return MP_OK;
}

MPStatus 
MGPlusDrawPath (HGRAPHICS graphics, HPEN pen, HPATH path)
{
    CTX_DRAW_PATH ctx;
    MPGraphics *pg = (MPGraphics *)graphics;
    MPPath *mppath = (MPPath *)path;
    MPPen *mppen   = (MPPen *)pen;

    if (!pg || !mppath || !mppen) 
        return MP_GENERIC_ERROR;

    RECT rc;
    PCLIPRGN region = CreateClipRgn ();

    SetRect(&rc, 0, 0, 
           GET_GDCAP_MAXX(pg->hdc), 
           GET_GDCAP_MAXY(pg->hdc));

    SetClipRgn (region, &rc);

    ctx.gps = pg;
    ctx.pen = mppen;
    ctx.path = mppath;
    ctx.mot.multiply(pg->matrix_gfx);

    DRAW_GRAPHIC(pg->hdc, region, &ctx, pg->op->draw_path);

    DestroyClipRgn(region);
    return MP_OK;
}


MPStatus 
MGPlusDrawLine (HGRAPHICS graphics, HPEN pen, 
        float x1, float y1, float x2, float y2)
{
    HPATH path = MGPlusPathCreate (MP_PATH_FILL_MODE_WINDING);
    MPStatus state;

    if(!path)
    {
        MGPlusPathDelete (path);
        return MP_GENERIC_ERROR;
    }

    MGPlusPathAddLine (path, x1, y1, x2, y2);

    state = MGPlusDrawPath (graphics, pen, (HPATH)path);
    MGPlusPathDelete (path);

    return state;
}

MPStatus 
MGPlusDrawLineI (HGRAPHICS graphics, HPEN pen, 
        int x1, int y1, int x2, int y2)
{
    return MGPlusDrawLine (graphics, pen, (float)x1, 
            (float)y1, (float)x2, (float)y2);
}

MPStatus 
MGPlusDrawArc (HGRAPHICS graphics, HPEN pen, float cx, float cy,
        float rx, float ry, float startAngle, float sweepAngle)
{
    HPATH path = MGPlusPathCreate (MP_PATH_FILL_MODE_WINDING);
    MPStatus state;

    if (!path || rx < 1 || ry < 1 || sweepAngle==0)
    {
        MGPlusPathDelete (path);
        return MP_GENERIC_ERROR;
    }
    
    MGPlusPathAddArc (path, cx, cy, rx, ry, startAngle, sweepAngle);
    state = MGPlusDrawPath (graphics, pen, path);

    MGPlusPathDelete (path);
    return state;
}

MPStatus 
MGPlusDrawArcI (HGRAPHICS graphics, HPEN pen, int cx, int cy,
        int rx, int ry, float startAngle, float sweepAngle)
{
    return MGPlusDrawArc (graphics, pen, (float)cx, (float)cy,
        (float)rx, (float)ry, startAngle, sweepAngle);
}


MPStatus 
MGPlusFillArc (HGRAPHICS graphics, HBRUSH brush, float cx, float cy,
        float rx, float ry, float startAngle, float sweepAngle)
{
    HPATH path = MGPlusPathCreate (MP_PATH_FILL_MODE_WINDING);
    MPStatus state;

    if (!path || rx < 1 || ry < 1 || sweepAngle==0)
    {
        MGPlusPathDelete (path);
        return MP_GENERIC_ERROR;
    }

    MGPlusPathAddArc (path, cx, cy, rx, ry, startAngle, sweepAngle);

    state = MGPlusFillPath (graphics, brush, path);
    MGPlusPathDelete (path);
    return state;
}

MPStatus 
MGPlusFillArcI (HGRAPHICS graphics, HBRUSH brush, int cx, int cy,
        int rx, int ry, float startAngle, float sweepAngle)
{
    return MGPlusFillArc (graphics, brush, (float)cx, (float)cy,
        (float)rx, (float)ry, startAngle, sweepAngle);
}


MPStatus 
MGPlusDrawBezier (HGRAPHICS graphics, HPEN pen, float x1, float y1,
        float x2, float y2, float x3, float y3, float x4, float y4)
{
    HPATH path = MGPlusPathCreate (MP_PATH_FILL_MODE_WINDING);
    MPStatus state;

    if (!path)
    {
        MGPlusPathDelete (path);
        return MP_GENERIC_ERROR;
    }

    MGPlusPathAddBezier (path, x1, y1, x2, y2, x3, y3, x4, y4);

    state = MGPlusDrawPath(graphics, pen, (HPATH)path);
    MGPlusPathDelete (path);
    return state;
}

MPStatus 
MGPlusDrawBezierI (HGRAPHICS graphics, HPEN pen, int x1, int y1,
        int x2, int y2, int x3, int y3, int x4, int y4)
{
    return MGPlusDrawBezier(graphics, pen, 
            (float)x1, (float)y1,
            (float)x2, (float)y2, 
            (float)x3, (float)y3, 
            (float)x4, (float)y4); 
}

MPStatus 
MGPlusFillBezier (HGRAPHICS graphics, HBRUSH brush, float x1, float y1,
        float x2, float y2, float x3, float y3, float x4, float y4)
{
    HPATH path = MGPlusPathCreate (MP_PATH_FILL_MODE_WINDING);
    MPStatus state;

    if (!path)
    {
        MGPlusPathDelete (path);
        return MP_GENERIC_ERROR;
    }

    MGPlusPathAddBezier (path, x1, y1, x2, y2, x3, y3, x4, y4);

    state = MGPlusFillPath(graphics, brush, path);
    MGPlusPathDelete (path);
    return state;
}

MPStatus 
MGPlusFillBezierI (HGRAPHICS graphics, HBRUSH brush, int x1, int y1,
        int x2, int y2, int x3, int y3, int x4, int y4)
{
    return MGPlusFillBezier(graphics, brush, 
            (float)x1, (float)y1,
            (float)x2, (float)y2, 
            (float)x3, (float)y3, 
            (float)x4, (float)y4); 
}


MPStatus 
MGPlusFillRectangle (HGRAPHICS graphics, HBRUSH brush, float x, float y,
        float width, float height)
{
    HPATH path = MGPlusPathCreate (MP_PATH_FILL_MODE_WINDING);
    MPStatus state;
    if (!path)
    {
        MGPlusPathDelete (path);
        return MP_GENERIC_ERROR;
    }

    MGPlusPathAddRectangle (path, (int)x, (int)y, (int)width, (int)height);

    state = MGPlusFillPath (graphics, brush, path);

    MGPlusPathDelete (path);
    return state;
}

MPStatus 
MGPlusFillRectangleI (HGRAPHICS graphics, HBRUSH brush, int x, int y,
        int width, int height)
{
    return MGPlusFillRectangle (graphics, brush, (float)x, (float)y,
        (float)width, (float)height);

}


MPStatus 
MGPlusFillRoundRectIEx (HGRAPHICS graphics, HBRUSH brush, int x, int y,
        int width, int height, int rx, int ry)
{
    HPATH path = MGPlusPathCreate (MP_PATH_FILL_MODE_WINDING);
    MPStatus state;

    if (!path)
    {
        MGPlusPathDelete (path);
        return MP_GENERIC_ERROR;
    }

    MGPlusPathAddRoundRectEx (path, x, y, width, height, rx, ry);

    state = MGPlusFillPath (graphics, brush, path);

    MGPlusPathDelete (path);
    return state;
}

MPStatus 
MGPlusFillRoundRectEx (HGRAPHICS graphics, HBRUSH brush, float x, float y,
        float width, float height, float rx, float ry)
{
    HPATH path = MGPlusPathCreate (MP_PATH_FILL_MODE_WINDING);
    MPStatus state;

    if (!path)
    {
        MGPlusPathDelete (path);
        return MP_GENERIC_ERROR;
    }

    MGPlusPathAddRoundRectEx (path, (int)x, (int)y, (int)width, (int) height,
            (int) rx, (int) ry);

    state = MGPlusFillPath (graphics, brush, path);

    MGPlusPathDelete (path);
    return state;
}


MPStatus 
MGPlusDrawRoundRectIEx (HGRAPHICS graphics, HPEN pen, int x, int y,
        int width, int height, int rx, int ry)
{
    HPATH path = MGPlusPathCreate (MP_PATH_FILL_MODE_WINDING);
    MPStatus state;

    if (!path)
    {
        MGPlusPathDelete (path);
        return MP_GENERIC_ERROR;
    }

    MGPlusPathAddRoundRectEx (path, x, y, width, height, rx, ry);

    state = MGPlusDrawPath (graphics, pen, path);

    MGPlusPathDelete (path);
    return state;
}

MPStatus 
MGPlusDrawRoundRectEx (HGRAPHICS graphics, HPEN pen, float x, float y,
        float width, float height, float rx, float ry)
{
    HPATH path = MGPlusPathCreate (MP_PATH_FILL_MODE_WINDING);
    MPStatus state;

    if (!path)
    {
        MGPlusPathDelete (path);
        return MP_GENERIC_ERROR;
    }

    MGPlusPathAddRoundRectEx (path, (int)x, (int)y, (int)width, (int) height,
            (int) rx, (int)ry);

    state = MGPlusDrawPath (graphics, pen, path);

    MGPlusPathDelete (path);
    return state;
}


MPStatus 
MGPlusDrawRectangle (HGRAPHICS graphics, HPEN pen, float x, float y,
        float width, float height)
{
    HPATH path = MGPlusPathCreate (MP_PATH_FILL_MODE_WINDING);
    MPStatus state;
    if (!path)
    {
        MGPlusPathDelete (path);
        return MP_GENERIC_ERROR;
    }

    MGPlusPathAddRectangle (path, x, y, width, height);

    state = MGPlusDrawPath (graphics, pen, path);

    MGPlusPathDelete (path);
    return state;
}

MPStatus 
MGPlusDrawRectangleI (HGRAPHICS graphics, HPEN pen, int x, int y,
        int width, int height)
{
    return MGPlusDrawRectangle (graphics, pen, (float)x, (float)y,
        (float)width, (float)height);

}

MPStatus 
MGPlusDrawEllipse (HGRAPHICS graphics, HPEN pen, float cx, float cy,
        float rx, float ry)
{
    if (!graphics || !pen || rx < 0 || ry < 0)
        return MP_GENERIC_ERROR;
    HPATH path = MGPlusPathCreate (MP_PATH_FILL_MODE_WINDING);
    MPStatus state;

    MGPlusPathAddEllipse (path, cx, cy, rx, ry, TRUE);

    state = MGPlusDrawPath(graphics, pen, path);
    MGPlusPathDelete (path);

    return state;
}

MPStatus 
MGPlusDrawEllipseI (HGRAPHICS graphics, HPEN pen, int cx, int cy,
        int rx, int ry)
{
    return MGPlusDrawEllipse(graphics, pen, (float)cx, (float)cy, 
            (float)rx, (float)ry);
}

MPStatus 
MGPlusFillEllipse (HGRAPHICS graphics, HBRUSH brush, float cx, float cy,
        float rx, float ry)
{
    if (!graphics || !brush || rx < 0 || ry < 0)
        return MP_GENERIC_ERROR;
    HPATH path = MGPlusPathCreate (MP_PATH_FILL_MODE_WINDING);
    MPStatus state;

    MGPlusPathAddEllipse (path, cx, cy, rx, ry, TRUE);

    state = MGPlusFillPath(graphics, brush, path);
    MGPlusPathDelete (path);

    return state;
}

MPStatus 
MGPlusFillEllipseI (HGRAPHICS graphics, HBRUSH brush, int cx, int cy,
        int rx, int ry)
{
    return MGPlusFillEllipse(graphics, brush, (float)cx, (float)cy, 
            (float)rx, (float)ry);
}


#if 0 /*unsupported*/
MPStatus MGPlusSetCompositingQuality (HGRAPHICS graphics, 
        MPCompositingQuality compositingQuality)
{
    MPGraphics* pgs = (MPGraphics *) graphics; 
    return MP_OK;
}

MPStatus MGPlusGetCompositingQuality (HGRAPHICS graphics, 
        MPCompositingQuality* compositingQuality)
{
    MPGraphics* pgs = (MPGraphics *) graphics; 
    return MP_OK;
}
#endif

MPStatus 
MGPlusSetCompositingMode (HGRAPHICS graphics, MPCompositingMode composite_mode)
{
    MPGraphics* pgs = (MPGraphics *) graphics; 
    if (!pgs)
        return MP_GENERIC_ERROR;
    pgs->compositing_mode = composite_mode;
    return MP_OK;
}

MPStatus 
MGPlusGetCompositingMode (HGRAPHICS graphics, MPCompositingMode* composite_mode)
{
    MPGraphics* pgs = (MPGraphics *) graphics; 
    if (!pgs || !composite_mode)
        return MP_GENERIC_ERROR;
    *composite_mode = pgs->compositing_mode;
    return MP_OK;
}

MPStatus 
MGPlusSetImageAlpha (HGRAPHICS graphics, int alpha)
{
    MPGraphics* pgs = (MPGraphics *) graphics; 
    if (!pgs) 
        return MP_GENERIC_ERROR;
    pgs->img_alpha = alpha;
    return MP_OK;
}

MPStatus 
MGPlusGetImageAlpha (HGRAPHICS graphics, int* alpha)
{
    MPGraphics* pgs = (MPGraphics *) graphics; 
    if (!pgs || !alpha)
        return MP_GENERIC_ERROR;
    (*alpha) = pgs->img_alpha;
    return MP_OK;
}


MPStatus 
MGPlusSetPathRenderingHint (HGRAPHICS graphics, MPPathRenderingHint value)
{
    MPGraphics* pgs = (MPGraphics *) graphics; 
    if (!pgs)
        return MP_GENERIC_ERROR;

    pgs->path_hints = value;

    return MP_OK;
}


MPStatus 
MGPlusGetPathRenderingHint (HGRAPHICS graphics, MPPathRenderingHint* value)
{
    MPGraphics* pgs = (MPGraphics *) graphics; 
    if (!pgs || !value)
        return MP_GENERIC_ERROR;

    (*value) = pgs->path_hints;

    return MP_OK;
}


MPStatus 
MGPlusSetSmoothingMode (HGRAPHICS graphics, MPSmoothingMode value)
{
    MPGraphics* pgs = (MPGraphics *) graphics; 
    if (!pgs)
        return MP_GENERIC_ERROR;

    pgs->smoothing_mode= value;

    return MP_OK;
}

MPStatus 
MGPlusGetSmoothingMode (HGRAPHICS graphics, MPSmoothingMode* value)
{
    MPGraphics* pgs = (MPGraphics *) graphics; 
    if (!pgs || !value)
        return MP_GENERIC_ERROR;

    (*value) = pgs->smoothing_mode;

    return MP_OK;
}


MPStatus 
MGPlusSetTextRenderingHint (HGRAPHICS graphics, MPTextRenderingHint value)
{
    MPGraphics* pgs = (MPGraphics *) graphics; 
    if (!pgs)
        return MP_GENERIC_ERROR;

    pgs->text_hints = value;

    return MP_OK;
}

MPStatus 
MGPlusGetTextRenderingHint (HGRAPHICS graphics, MPTextRenderingHint* value)
{
    MPGraphics* pgs = (MPGraphics *) graphics; 
    if (!pgs || !value)
        return MP_GENERIC_ERROR;

    (*value) = pgs->text_hints;

    return MP_OK;
}


MPStatus 
MGPlusSetInterpolationMode (HGRAPHICS graphics, MPInterpolationMode value)
{
    MPGraphics* pgs = (MPGraphics *) graphics; 
    if (!pgs)
        return MP_GENERIC_ERROR;
    pgs->interpolation_mode = value;
    return MP_OK;
}

MPStatus 
MGPlusGetInterpolationMode (HGRAPHICS graphics, MPInterpolationMode* value)
{
    MPGraphics* pgs = (MPGraphics *) graphics; 
    if (!pgs || !value)
        return MP_GENERIC_ERROR;

    (*value) = pgs->interpolation_mode;

    return MP_OK;
}

MPStatus 
MGPlusWorldTransform (HGRAPHICS graphics, MPMatrix *matrix)
{
    MPGraphics* pgs = (MPGraphics *)graphics; 
    if (!pgs)
        return MP_GENERIC_ERROR;
    if (!matrix)
        return MP_INVALID_PARAMETER;

    if (matrix->sx == 0 && matrix->shy == 0 &&
            matrix->shx == 0 && matrix->sy == 0)
        return MP_INVALID_PARAMETER;

    double m_mtx [6] = 
    {
        matrix->sx, matrix->shy, matrix->shx, 
        matrix->sy, matrix->tx, matrix->ty
    };
    agg::trans_affine aff;

    aff.load_from (m_mtx);
    pgs->matrix_gfx.multiply(aff);

    return MP_OK;
}

MPStatus 
MGPlusGetWorldTransform (HGRAPHICS graphics, MPMatrix *matrix)
{
    MPGraphics* pgs = (MPGraphics *)graphics; 
    if (!pgs)
        return MP_GENERIC_ERROR;
    if (!matrix)
        return MP_INVALID_PARAMETER;

    pgs->matrix_gfx.store_to((double*)matrix);

    return MP_OK;
}

MPStatus 
MGPlusSetWorldTransform (HGRAPHICS graphics, MPMatrix *matrix)
{
    MPGraphics* pgs = (MPGraphics *)graphics; 
    if (!pgs)
        return MP_GENERIC_ERROR;
    if (!matrix)
        return MP_INVALID_PARAMETER;

    if (matrix->sx == 0 && matrix->shy == 0 &&
            matrix->shx == 0 && matrix->sy == 0)
        return MP_INVALID_PARAMETER;

    double m_mtx [6] = 
    {
        matrix->sx, matrix->shy, matrix->shx, 
        matrix->sy, matrix->tx, matrix->ty
    };

    pgs->matrix_gfx.load_from (m_mtx);

    return MP_OK;
}

MPStatus 
MGPlusResetWorldTransform (HGRAPHICS graphics)
{
    MPGraphics* pgs = (MPGraphics *)graphics; 
    if (!pgs)
        return MP_GENERIC_ERROR;

    pgs->matrix_gfx.reset();

    return MP_OK;
}

MPStatus 
MGPlusTranslateWorldTransform (HGRAPHICS graphics, float dx, float dy)
{
    MPGraphics* pgss = (MPGraphics *)graphics; 
    if (!graphics)
        return MP_GENERIC_ERROR;

    pgss->matrix_gfx.translate(dx, dy);
    return MP_OK;
}

MPStatus 
MGPlusScaleWorldTransform (HGRAPHICS graphics, float sx, float sy)
{
    MPGraphics* pgss = (MPGraphics *)graphics; 
    if (!graphics)
        return MP_GENERIC_ERROR;

    pgss->matrix_gfx.scale(sx, sy);
    return MP_OK;
}

MPStatus 
MGPlusRotateWorldTransform (HGRAPHICS graphics, float angle)
{
    MPGraphics* pgss = (MPGraphics *)graphics; 
    if (!graphics)
        return MP_GENERIC_ERROR;

    pgss->matrix_gfx.rotate(angle * agg::pi / 180.0);
    return MP_OK;
}

MPStatus 
MGPlusDrawImage (HGRAPHICS graphics, int n_index, int x, int y, int w, int h)
{
    MPPOINT pt[4] = {{0, 0}};
    MPGraphics* pgss = (MPGraphics *) graphics;

    if (!graphics || n_index < 0
            || n_index >= MAX_BMP_NUM)
        return MP_GENERIC_ERROR;
    
    if (pgss->surf_img [n_index] == NULL)
        return MP_GENERIC_ERROR;

    if (w <= 0) w = pgss->surf_img[n_index]->bmWidth;
    if (h <= 0) h = pgss->surf_img[n_index]->bmHeight;

    pt[0].x = x;   pt[0].y = y;
    pt[1].x = x+w; pt[1].y = y;
    pt[2].x = x+w; pt[2].y = y+h;
    pt[3].x = x;   pt[3].y = y+h;

    MGPlusDrawImageWithPoints (graphics, n_index, pt, 4);
    return MP_OK;
}

MPStatus 
MGPlusDrawImageWithPoints (HGRAPHICS graphics, int n_index, 
        const MPPOINT* point, int count)
{
    CTX_IMAGE_PTS ctx;
    MPGraphics* pg = NULL; 

    if (!graphics || !point || count < 3 || n_index < 0
            || n_index >= MAX_BMP_NUM)
        return MP_GENERIC_ERROR;

    pg = (MPGraphics *) graphics;

    if (pg->surf_img [n_index] == NULL)
        return MP_GENERIC_ERROR;

    RECT rc;
    SetRect(&rc, 0, 0, 
            GET_GDCAP_MAXX(pg->hdc), 
            GET_GDCAP_MAXY(pg->hdc));

    PCLIPRGN region = CreateClipRgn ();
    SetClipRgn (region, &rc);

    ctx.gps = pg;
    ctx.idx = n_index;
    ctx.pt = (MPPOINT *)point;
    ctx.count = count;
    ctx.mot.multiply(pg->matrix_gfx);

    DRAW_GRAPHIC(pg->hdc, region, &ctx, pg->op->draw_image_with_pts);

    DestroyClipRgn(region);
    return MP_OK;
}

MPStatus 
MGPlusDrawImageWithPath(HGRAPHICS graphics, int n_index, HPATH path)
{
    CTX_IMAGE_PATH ctx;
    MPGraphics* pg = (MPGraphics *)graphics; 
    MPPath* p_path = (MPPath *)path;

    if (!pg || n_index < 0 || n_index >= MAX_BMP_NUM 
         || !p_path)
        return MP_GENERIC_ERROR;

    if (pg->surf_img [n_index] == NULL)
        return MP_GENERIC_ERROR;

    RECT rc;
    SetRect(&rc, 0, 0, 
            GET_GDCAP_MAXX(pg->hdc), 
            GET_GDCAP_MAXY(pg->hdc));

    PCLIPRGN region = CreateClipRgn ();
    SetClipRgn (region, &rc);

    ctx.gps = pg;
    ctx.idx = n_index;
    ctx.path = p_path;
    ctx.mot.multiply(pg->matrix_gfx);

    DRAW_GRAPHIC(pg->hdc, region, &ctx, pg->op->draw_image_with_path);

    DestroyClipRgn(region);
    return MP_OK;
}


template<class Array>
void fill_color_array(Array& array, ARGB* p_color, int count)
{
    unsigned int i, j;
    unsigned int size = (int) (array.size() / (count - 1));

    if (!size) return;

    j = 0;
    for(i = 0; i < array.size(); ++i)
    {
        if (i % size == 0)
        {
            j ++;
        }
        agg::rgba8 start (MPGetRValue (p_color [j - 1]),\
                MPGetGValue (p_color [j - 1]),\
                MPGetBValue (p_color [j - 1]),\
                MPGetAValue (p_color [j - 1])); 
        agg::rgba8 end (MPGetRValue (p_color [j]),\
                MPGetGValue (p_color [j]),\
                MPGetBValue (p_color [j]),\
                MPGetAValue (p_color [j]));
        array[i] = start.gradient (end, ((i - (j - 1) * size) / double (size)));
    }
}

MPStatus 
MGPlusFillPath (HGRAPHICS graphics, HBRUSH brush, HPATH path)
{
    CTX_FILL_PATH ctx;
    MPGraphics *pg = (MPGraphics *)graphics;
    MPBrush* m_brush = (MPBrush*) brush;
    MPPath* m_path = (MPPath *) path;
    RECT rc;
    PCLIPRGN region;

    if (!pg || !m_brush || !m_path) 
        return MP_GENERIC_ERROR;

    SetRect(&rc, 0, 0, 
           GET_GDCAP_MAXX(pg->hdc), 
           GET_GDCAP_MAXY(pg->hdc));

    region = CreateClipRgn ();
    SetClipRgn (region, &rc);

    ctx.gps = pg;
    ctx.brush = m_brush;
    ctx.path = m_path;
    ctx.mot.multiply(pg->matrix_gfx);

    DRAW_GRAPHIC(pg->hdc, region, &ctx, pg->op->fill_path);

    DestroyClipRgn(region);

    return MP_OK;
}

MPStatus 
MGPlusGraphicLoadBitmap (HGRAPHICS graphics, int n_index, PBITMAP p_bitmap)
{
    MPGraphics *pgs = (MPGraphics *)graphics;

    if (n_index > MAX_BMP_NUM || n_index < 0 || !p_bitmap)
        return MP_INDEX_NOT_MATCH;

    if (pgs->surf_img [n_index] == NULL)
        pgs->surf_img [n_index] =  new BITMAP;

    memcpy(pgs->surf_img [n_index], p_bitmap, sizeof(BITMAP));
    pgs->rendering_img [n_index].attach (pgs->surf_img[n_index]->bmBits,
                pgs->surf_img[n_index]->bmWidth,
                pgs->surf_img[n_index]->bmHeight,
                pgs->surf_img[n_index]->bmPitch);

    return MP_OK;
}

MPStatus 
MGPlusGraphicUnLoadBitmap (HGRAPHICS graphics, int n_index)
{

    MPGraphics *pgs = (MPGraphics *)graphics;

    if (n_index > MAX_BMP_NUM || n_index < 0)
        return MP_INDEX_NOT_MATCH;

#ifndef _MG_MINIMALGDI
    if (pgs->surf_img [n_index]) {
        UnloadBitmap(pgs->surf_img[n_index]);
    }
    return MP_OK;
#endif
}

PBITMAP
MGPlusGraphicGetBitmap (HGRAPHICS graphics, int n_index)
{
    MPGraphics *pgs = (MPGraphics *)graphics;

    if (n_index > MAX_BMP_NUM || n_index < 0)
        return NULL;

    if (pgs->surf_img [n_index]) {
        return pgs->surf_img [n_index];
    }

    return NULL;
}

MPStatus 
MGPlusGraphicLoadBitmapFromFile(HGRAPHICS graphics, int n_index, char* file)
{
#ifndef _MG_MINIMALGDI
    MPGraphics *pgs = (MPGraphics *)graphics;
    HDC hdc;

    if (n_index > MAX_BMP_NUM || n_index < 0)
        return MP_INDEX_NOT_MATCH;
    if (pgs->surf_img [n_index] == NULL)
        pgs->surf_img [n_index] = new BITMAP;

    if (pgs->hdc_flags == MP_HDC_EXTERNAL)
        hdc = pgs->img_dc;
    else
        hdc = pgs->hdc;

    if (LoadBitmapFromFile (hdc, pgs->surf_img[n_index], file)) {
        delete (pgs->surf_img[n_index]);
        pgs->surf_img[n_index] = NULL;
        return MP_GENERIC_ERROR;
    }

    pgs->rendering_img [n_index].attach (pgs->surf_img [n_index]->bmBits,
                pgs->surf_img [n_index]->bmWidth, 
                pgs->surf_img [n_index]->bmHeight, 
                pgs->surf_img [n_index]->bmPitch);

    return MP_OK;
#else
    return MP_GENERIC_ERROR;
#endif
}

#ifdef _MGPLUS_FONT_FT2
MPStatus 
MGPlusDrawGlyph (HGRAPHICS graphics, HFONT hfont, int x, int y, 
        LPGLYPHDATA lpdata, ARGB color)
{
    CTX_DRAW_GLYPH ctx;
    MPGraphics* pg = (MPGraphics *)graphics; 
    MPFont*     m_font = (MPFont*) hfont;

    if (!pg || !m_font)
        return MP_GENERIC_ERROR;

    /* data_type must be invalid.*/
    if (!lpdata  || lpdata->data_type == GLYPH_DATA_INVALID) {
        return MP_GENERIC_ERROR;
    }

    RECT rc;
    SetRect(&rc, 0, 0, 
            GET_GDCAP_MAXX(pg->hdc), 
            GET_GDCAP_MAXY(pg->hdc));

    PCLIPRGN region = CreateClipRgn ();
    SetClipRgn (region, &rc);

    ctx.gps = pg;
    ctx.font = m_font;
    ctx.x = x;
    ctx.y = y;
    ctx.lpdata = lpdata;
    ctx.color = color;
    ctx.mot.multiply(pg->matrix_gfx);
    
    DRAW_GRAPHIC(pg->hdc, region, &ctx, pg->op->draw_glyph);

    DestroyClipRgn(region);
    return MP_OK;
}
#endif

MPStatus 
MGPlusSetClipPath (HGRAPHICS graphics, HPATH path)
{
    MPGraphics* pg = (MPGraphics *)graphics; 
    if (!pg)
        return MP_GENERIC_ERROR;
    int w = pg->width;
    int h = pg->height;

    pg->clip_ras.state = true;
    if (pg->clip_ras.buf) {
        delete [] pg->clip_ras.buf; 
    }

    pg->clip_ras.buf = new unsigned char[w * h];
    pg->clip_ras.ren_rbuf.attach(pg->clip_ras.buf, w, 
            h, w);
    pg->clip_ras.alpha_mask.attach(pg->clip_ras.ren_rbuf);

    typedef agg::renderer_base<agg::pixfmt_gray8> ren_base;
    typedef agg::renderer_scanline_aa_solid<ren_base> renderer;

    agg::pixfmt_gray8 pixf(pg->clip_ras.ren_rbuf);
    agg::scanline_p8 sl;
    ren_base rb(pixf);
    renderer r(rb);

    rb.clear(agg::gray8(0));

    if (pg->clip_ras.path == 0) {
        pg->clip_ras.path = (MPPath*) MGPlusPathCreate(MP_PATH_FILL_MODE_WINDING);
    }
    MGPlusPathAddPath ((HPATH)pg->clip_ras.path, path);

    pg->clip_ras.ras.add_path(pg->clip_ras.path->m_agg_ps);
    r.color(agg::gray8(0xFF, 0xFF));
    agg::render_scanlines(pg->clip_ras.ras, sl, r);

    return MP_OK;
}

typedef struct _DCSTATE
{
    MPRenderingControl      render_control;
    MPTextRenderingHint     text_hints;
    MPPathRenderingHint     path_hints;
    MPSmoothingMode         smoothing_mode; 
    MPInterpolationMode     interpolation_mode;
    MPCompositingMode       compositing_mode;
    /* translate matrix. */
    agg::trans_affine       matrix_gfx;
    int                     img_alpha;
    struct _DCSTATE *next, *prev;
} DCSTATE;

#define INIT_LOCK(lock, attr)   pthread_mutex_init(lock, attr)
#define LOCK(lock)              pthread_mutex_lock(lock)
#define UNLOCK(lock)            pthread_mutex_unlock(lock)
#define DESTROY_LOCK(lock)      pthread_mutex_destroy(lock)

static DCSTATE* hg_state_stack = NULL;
static int nr_hg_states = 0;
static BOOL lock_state = FALSE;
static pthread_mutex_t lock;
static pthread_once_t once_control = PTHREAD_ONCE_INIT;

void init_routine(void)
{
    INIT_LOCK(&lock, NULL);
}

int MGPlusSaveHG (HGRAPHICS hg)
{
    DCSTATE* hg_state;
    MPGraphics* pg = (MPGraphics *)hg; 

    hg_state = new DCSTATE;
    if (hg_state == NULL)
        return 0;

    hg_state->render_control     = pg->render_control;
    hg_state->smoothing_mode     = pg->smoothing_mode;
    hg_state->text_hints         = pg->text_hints;
    hg_state->path_hints         = pg->path_hints;
    hg_state->interpolation_mode = pg->interpolation_mode;
    hg_state->compositing_mode   = pg->compositing_mode;
    hg_state->matrix_gfx         = pg->matrix_gfx;
    //hg_state->clip_ras           = pg->clip_ras;
    hg_state->img_alpha          = pg->img_alpha;

    hg_state->next = NULL;
    hg_state->prev = NULL;

#ifndef _MG_MINIMALGDI
    pthread_once(&once_control, init_routine);
#endif
    LOCK (&lock);

    nr_hg_states ++;
    if (hg_state_stack == NULL) {
        hg_state_stack = hg_state;
    }
    else {
        hg_state_stack->next = hg_state;
        hg_state->prev = hg_state_stack;
        hg_state_stack = hg_state;
    }

    UNLOCK (&lock);
    return nr_hg_states;
}

static void destroy_hg_state (DCSTATE* hg_state)
{
    delete  (hg_state);
}

BOOL MGPlusRestoreHG (HGRAPHICS hg, int saved_hg)
{
    DCSTATE* hg_state;
    MPGraphics* pg = (MPGraphics *)hg; 

    if ((pg == NULL) || 
                (saved_hg > nr_hg_states) ||
                (-saved_hg > nr_hg_states))
        return FALSE;

    if (lock_state == FALSE) {
        lock_state = TRUE;
        INIT_LOCK(&lock, NULL);
    }
    LOCK (&lock);

    if (saved_hg > 0) {
        int n = nr_hg_states - saved_hg;
        while (n--) {
            hg_state = hg_state_stack->prev;
            destroy_hg_state (hg_state_stack);
            hg_state_stack = hg_state;
            hg_state_stack->next = NULL;
            nr_hg_states --;
        }
    }
    else { /* saved_hg < 0 */
        int n = -saved_hg - 1;
        while (n--) {
            hg_state = hg_state_stack->prev;
            destroy_hg_state (hg_state_stack);
            hg_state_stack = hg_state;
            hg_state_stack->next = NULL;
            nr_hg_states --;
        }
    }
    hg_state = hg_state_stack;

    pg->render_control     = hg_state->render_control;
    pg->smoothing_mode     = hg_state->smoothing_mode;
    pg->text_hints         = hg_state->text_hints;
    pg->path_hints         = hg_state->path_hints;
    pg->interpolation_mode = hg_state->interpolation_mode;
    pg->compositing_mode   = hg_state->compositing_mode;
    pg->matrix_gfx         = hg_state->matrix_gfx;
    //pg->clip_ras           = hg_state->clip_ras;           
    pg->img_alpha          = hg_state->img_alpha;            

    hg_state_stack = hg_state->prev;
    if (hg_state_stack)
        hg_state_stack->next = NULL;
    nr_hg_states --;

    destroy_hg_state (hg_state);

    UNLOCK(&lock);

    return TRUE;
}
