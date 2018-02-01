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
 ** opt_basic.cpp: Implementation of common draw and fill interface
 ** for all pixformt support. 
 **
 ** Create date: 2009/03/22
 */
#include "agg_pixfmt_rgba.h"
#include "agg_conv_dash.h"
#include "agg_math_stroke.h"
#include "agg_bounding_rect.h"
#include "agg_renderer_primitives.h"
#include "agg_rasterizer_outline.h"
#include "agg_span_converter.h"

#include "self_stroke.h"

typedef PIXFMT::color_type              COLOR_TYPE;
typedef COLOR_TYPE::value_type          VALUE_TYPE;
typedef agg::rendering_buffer           RBUF_TYPE;
typedef agg::renderer_base<PIXFMT>      RENDERER_BASE;

//typedef agg::pixfmt_argb32              IMG_PIXFMT;
//typedef PIXFMT              IMG_PIXFMT;
typedef agg::image_accessor_clone<IMG_PIXFMT> AIMG_ACCESSOR_TYPE;
typedef agg::span_image_filter_rgba_2x2<AIMG_ACCESSOR_TYPE, 
                                        BILINEAR_INTERPOLATOR> ASPAN_GEN_BILINEAR;
typedef agg::span_image_filter_rgba_2x2<AIMG_ACCESSOR_TYPE, 
                                        AFFINE_INTERPOLATOR> ASPAN_GEN_AFFINE;
typedef agg::span_image_filter_rgba_2x2<AIMG_ACCESSOR_TYPE, 
                                        PERSPECTIVE_INTERPOLATOR> ASPAN_GEN_PERSPECTIVE;

typedef agg::renderer_base<PIXFMT_PRE>  RENDERER_BASE_PRE;
typedef agg::renderer_scanline_aa_solid<RENDERER_BASE> RENDERER_SOLID;

typedef agg::conv_dash<agg::path_storage>   DASH_TYPE;
typedef agg::conv_stroke<DASH_TYPE>         STROKE_DASH_TYPE;
typedef agg::conv_stroke<agg::path_storage> STROKE_STORAGE_TYPE;

typedef agg::span_interpolator_linear<>     SPAN_INTERPOLATOR;

typedef agg::span_allocator<COLOR_TYPE>     SPAN_ALLOCATOR_DEFAULT;
typedef agg::span_allocator<agg::rgba8>     IMG_SPAN_ALLOCATOR_DEFAULT;

/* gradient*/
typedef agg::pod_auto_array <COLOR_TYPE, 256> COLOR_ARRAY;
typedef agg::gradient_x             GRADIENT_X;   
typedef agg::gradient_y             GRADIENT_Y;   
typedef agg::gradient_radial_focus  GRADIENT_RADIAL;

typedef agg::gamma_lut<agg::int8u, agg::int8u>          GAMMA_LUT;
typedef agg::gradient_reflect_adaptor<GRADIENT_RADIAL>  GRADIENT_ADAPTOR_RADIAL;
typedef agg::gradient_lut<agg::color_interpolator<COLOR_TYPE>, 1024> GRADIENT_LUT;

typedef agg::span_gradient <COLOR_TYPE,
                            SPAN_INTERPOLATOR, 
                            GRADIENT_X, 
                            COLOR_ARRAY> SPAN_GRADIENT_X;

typedef agg::span_gradient <COLOR_TYPE,
                            SPAN_INTERPOLATOR, 
                            GRADIENT_Y, 
                            COLOR_ARRAY> SPAN_GRADIENT_Y;

typedef agg::span_gradient <COLOR_TYPE,
                            SPAN_INTERPOLATOR, 
                            GRADIENT_ADAPTOR_RADIAL, 
                            GRADIENT_LUT> SPAN_GRADIENT_RADIAL;

typedef agg::span_allocator<COLOR_TYPE> SPAN_ALLOCATOR;

typedef agg::span_gouraud_rgba <COLOR_TYPE> SPAN_GOURAUD;

typedef agg::renderer_scanline_bin_solid<RENDERER_BASE>     RENDERER_BIN_SOLID;
typedef agg::serialized_integer_path_adaptor<agg::int32, 6> PATH_ADAPTOR;

#ifdef COMP_DRAW
typedef agg::blender_rgba_pre<COLOR_TYPE, ORDER> BLENDER_PRE; 
typedef agg::pixfmt_alpha_blend_rgba<BLENDER_PRE, RBUF_TYPE, PIXEL_TYPE> PIXFMT_BLEND_PRE;
typedef agg::renderer_base<PIXFMT_BLEND_PRE>                       RENDERER_BLEND_PRE;

typedef agg::comp_op_adaptor_rgba<COLOR_TYPE, ORDER>               COMP_OP_ADAPTOR;
typedef agg::pixfmt_custom_blend_rgba<COMP_OP_ADAPTOR, RBUF_TYPE>  PIXFMT_CUSTOM_TYPE;
typedef agg::renderer_base<PIXFMT_CUSTOM_TYPE>                     RENDERER_CUSTOM_TYPE;

typedef agg::renderer_scanline_aa < RENDERER_CUSTOM_TYPE, 
                                    SPAN_ALLOCATOR, 
                                    SPAN_GRADIENT_X> RENDERER_GRADIENT_X;

typedef agg::renderer_scanline_aa < RENDERER_CUSTOM_TYPE, 
                                    SPAN_ALLOCATOR, 
                                    SPAN_GRADIENT_Y> RENDERER_GRADIENT_Y;

#define SET_COMP_OP(gps, ren_pixf) ren_pixf.comp_op((agg::comp_op_e)gps->compositing_mode);

#else /*no define COMP_DRAW*/

typedef PIXFMT_PRE                      PIXFMT_BLEND_PRE;
typedef agg::renderer_base<PIXFMT_PRE>  RENDERER_BLEND_PRE;

typedef agg::renderer_base<PIXFMT>      RENDERER_CUSTOM_TYPE;
typedef PIXFMT                          PIXFMT_CUSTOM_TYPE;

typedef agg::renderer_scanline_aa < RENDERER_BASE, 
                                    SPAN_ALLOCATOR, 
                                    SPAN_GRADIENT_X> RENDERER_GRADIENT_X;

typedef agg::renderer_scanline_aa < RENDERER_BASE, 
                                    SPAN_ALLOCATOR, 
                                    SPAN_GRADIENT_Y> RENDERER_GRADIENT_Y;

#define SET_COMP_OP(pgs, ren_pixf)

#endif /*end of #ifdef COMP_DRAW*/

typedef agg::renderer_primitives<RENDERER_CUSTOM_TYPE>  PRIMITIVES_RENDERER;
typedef agg::rasterizer_outline<PRIMITIVES_RENDERER>    OUTLINE_RASTERIZER;

#define SET_CLIP_BOX(ren_comp, rc) ren_comp.clip_box(rc->left, rc->top,  \
        rc->right-1, rc->bottom-1);

#define RAS_RESET(gps) if (gps->clip_ras.state) { \
        gps->clip_ras.ras.reset(); \
    } \
    else \
        ras.reset (); 

#define RAS_ADD_PATH(gps, path) if (gps->clip_ras.state) {  \
                gps->clip_ras.ras.add_path(path);           \
            }                                               \
            else {                                          \
                ras.add_path (path);                        \
            }

#define AGG_RENDER(v, ras, sl, ren_comp, color) if (v){                                \
                   agg::render_scanlines_aa_solid(ras, sl, ren_comp, color);           \
                }                                                                      \
                else{                                                                  \
                   agg::render_scanlines_bin_solid(ras, sl, ren_comp, color);          \
                }
                                                                                                      
#define AGG_RENDER_CLIP(gps, v, ras, sl, ren_comp, color)                              \
    if (gps->clip_ras.state) {                                                         \
        typedef agg::scanline_u8_am<agg::alpha_mask_gray8> scanline_type;              \
        scanline_type sl(gps->clip_ras.alpha_mask);                                    \
        AGG_RENDER((gps->path_hints == MP_PATH_RENDER_HINT_ANTIALIAS_ON),              \
                gps->clip_ras.ras, sl, ren_comp, color);                               \
    }                                                                                  \
    else {                                                                             \
        AGG_RENDER((gps->path_hints == MP_PATH_RENDER_HINT_ANTIALIAS_ON),              \
                ras, sl, ren_comp, color);                                             \
    }

#define AGG_RENDER2(v, ras, sl, ren_comp, alloc, span_gen) if (v){                     \
                   agg::render_scanlines_aa(ras, sl, ren_comp, alloc, span_gen);       \
                }                                                                      \
                else{                                                                  \
                   agg::render_scanlines_bin(ras, sl, ren_comp, alloc, span_gen);      \
                }

#define AGG_RENDER2_CLIP(gps, v, ras, sl, ren_comp, sa, sc)                            \
            if (gps->clip_ras.state) {                                                 \
                typedef agg::scanline_u8_am<agg::alpha_mask_gray8> scanline_type;      \
                scanline_type sl(gps->clip_ras.alpha_mask);                            \
                AGG_RENDER2(v, gps->clip_ras.ras, sl, ren_comp, sa, sc);               \
            }                                                                          \
            else                                                                       \
                AGG_RENDER2(v, ras, sl, ren_comp, sa, sc);  

/* houhh 20090512, can not use inline, else the ren_pixf will be error to
 * pixfmt_alpha_blend_rgba when use rgb565.*/

//inline void draw_path_by_nostroke(CTX_DRAW_PATH* pctx, 
static void draw_path_by_nostroke(CTX_DRAW_PATH* pctx, 
        agg::rgba8& color, agg::trans_affine& mot, const RECT* rc)
{
    int i = 0;
    PIXFMT_CUSTOM_TYPE      ren_pixf(pctx->gps->rendering_buff);
    RENDERER_CUSTOM_TYPE    ren_comp(ren_pixf);
    PRIMITIVES_RENDERER     ren(ren_comp);
    OUTLINE_RASTERIZER      ras(ren);

    SET_COMP_OP(pctx->gps, ren_pixf);
    SET_CLIP_BOX(ren_comp, rc);
    ren.line_color(color);
    self_stroke_draft<agg::path_storage> s(pctx->path->m_agg_ps, 1);

    while (i < pctx->path->id)
    {
        pctx->path->m_agg_ps.rewind( pctx->path->path_id[i] );
        agg::conv_transform<agg::path_storage, agg::trans_affine>
            trans (pctx->path->m_agg_ps, mot);
        ras.add_path (trans);
        i ++;
    }

    agg::conv_transform<self_stroke_draft<agg::path_storage>, agg::trans_affine>
            trans (s, mot);
    ras.add_path (trans);
}

inline float sign (float num)    { return ( num < 0.0 ) ? -1 : 1; }

inline void 
rect_to_polar (double* ang, double* dist, float x, 
                float y, float centerX, float centerY, 
                float scaleX = 1.0, float scaleY = 1.0)
{
    double half_pi = 2.0 * atan( 1.0 );    /**< Constant value of PI */

    // Calculate the offsets:
    double dx = ( x - centerX ) * scaleX;
    double dy = ( y - centerY ) * scaleY;

    // The angle is calculated as the inverse tangent of dy/dx.  If
    // dx == 0, then the angle is either +/- 90 degrees, based on the
    // sign of dy.
    if ( dx == 0.0 )
    {
        *ang = half_pi * sign( dy );
    }
    else
        *ang = atan2 ( dy, dx );
    *dist = dx;
}

namespace agg
{   

    //--------------------------------------------------------------------
    class span_conv_brightness_alpha_rgb8
    {   
        public: 
            typedef rgba8 color_type;
            typedef int8u alpha_type;

            enum array_size_e
            {
                array_size = 1 
            };

            span_conv_brightness_alpha_rgb8(const alpha_type* alpha_array) :
                m_alpha_array(alpha_array)
        {
        }

            void prepare() {}
            void generate(color_type* span, int x, int y, unsigned len) const
            {   
                if (m_alpha_array [0] != 255)
                {
                    do
                    {
                        if (span->a != 0)
                            //span->a = m_alpha_array[span->r + span->g + span->b];
                            span->a = m_alpha_array [0];
                        ++span;
                    }
                    while(--len); 
                }
            }       

        private:
            const alpha_type* m_alpha_array;
    };      

} 

template<class Array>
static void fill_color_array(Array& array, LinearGradientNode* p_node, int num)
{
    unsigned int i;
    LinearGradientNode* p_prev;
    LinearGradientNode* p_next;

    p_prev = p_node;
    p_next = p_node->p_next; 

    if (p_prev->f_pos == p_next->f_pos)
    {
        p_prev = p_next;
        p_next = p_next->p_next;
    }

    for (i = 0; i < array.size(); ++i)
    {
        if ((i != 0) && (i % ((int)(p_next->f_pos * array.size ())) == 0))
        {
            p_prev = p_next;
            if (p_next->p_next)
                p_next = p_next->p_next;

            if (p_prev->f_pos == p_next->f_pos)
            {
                p_prev = p_next;
                p_next = p_next->p_next;
            }
        }

        agg::rgba8 start (MPGetRValue (p_prev->color),\
                MPGetGValue (p_prev->color),\
                MPGetBValue (p_prev->color),\
                MPGetAValue (p_prev->color)); 
        agg::rgba8 end (MPGetRValue (p_next->color),\
                MPGetGValue (p_next->color),\
                MPGetBValue (p_next->color),\
                MPGetAValue (p_next->color));
        array[i] = start.gradient (end, ((i - p_prev->f_pos * array.size ())
                / double ((p_next->f_pos - p_prev->f_pos) * array.size ())));
    }
}

static void
copy(HDC hdc, Uint8* pixels, int pitch, int bpp, const RECT* rc, void* ctx)
{
    CTX_COPY *pctx = (CTX_COPY *)ctx;

    PIXFMT_BLEND_PRE    pixf_pre(pctx->dst->rendering_buff);
    RENDERER_BLEND_PRE  rb_pre(pixf_pre);
    PIXFMT              pixf_src(pctx->src->rendering_buff);

    SET_CLIP_BOX(rb_pre, rc);

    if (pctx->blend)
#ifdef COMP_DRAW
        rb_pre.blend_from(pixf_src);
#else
        rb_pre.copy_from(pixf_src);
#endif
    else
        rb_pre.copy_from(pixf_src);
}

static void
clear(HDC hdc, Uint8* pixels, int pitch, int bpp, const RECT* rc, void* ctx)
{
    agg::rasterizer_scanline_aa<>   ras;
    agg::scanline_u8                sl;
    CTX_CLEAR* pctx = (CTX_CLEAR *)ctx;

    agg::rgba8 color(MPGetRValue(pctx->color), 
                     MPGetGValue(pctx->color), 
                     MPGetBValue(pctx->color),
                     MPGetAValue(pctx->color));

    ras.reset ();
    PIXFMT screen_pixf (pctx->gps->rendering_buff);
    RENDERER_BASE rb_screen (screen_pixf);

    SET_CLIP_BOX(rb_screen, rc);
    //agg::render_scanlines_aa_solid(ras, sl, rb_screen, color);
    rb_screen.clear_clip_box (color);
}


static void
draw_path(HDC hdc, Uint8* pixels, int pitch, int bpp, const RECT* rc, void* ctx)
{
    CTX_DRAW_PATH* pctx = (CTX_DRAW_PATH *)ctx;

    agg::rasterizer_scanline_aa<>   ras;
    agg::scanline_u8                sl;

    PIXFMT_CUSTOM_TYPE      ren_pixf(pctx->gps->rendering_buff);
    RENDERER_CUSTOM_TYPE    ren_comp(ren_pixf);

    agg::rgba8 color (MPGetRValue(pctx->pen->rgba), 
                      MPGetGValue(pctx->pen->rgba), 
                      MPGetBValue(pctx->pen->rgba),
                      MPGetAValue(pctx->pen->rgba));
    /* reset rasterizer. */
    RAS_RESET(pctx->gps);

    POINT pt={0, 0};
    LPtoSP(hdc, &pt);
    agg::trans_affine mot;
    mot.multiply(pctx->mot);
    mot.translate(pt.x, pt.y);

    /*add by tjb*/
    if (pctx->pen->width <= 1) {
        draw_path_by_nostroke(pctx, color, mot, rc);
        return;
    }


    DASH_TYPE dash_path (pctx->path->m_agg_ps);
    if (pctx->pen->num_dashes > 0) {
        unsigned int i = 0;
        for (i = 0; i < pctx->pen->num_dashes; i += 2) {
            if ((i+1) < pctx->pen->num_dashes) {
                dash_path.add_dash (pctx->pen->dash[i], pctx->pen->dash[i+1]);
            }
            else {
                /* default set dash_gap to 5.*/
                dash_path.add_dash (pctx->pen->dash[i], 5);
            }
        }
        dash_path.dash_start (pctx->pen->dash_phase);

        /* set pen line join_style and cap_style. */
        STROKE_DASH_TYPE stroke_path (dash_path);
        stroke_path.width (pctx->pen->width);

        switch (pctx->pen->line_cap_e) {
            case CAP_BUTT:
            case CAP_SQUARE:
                stroke_path.line_cap (agg::butt_cap);
                break;
            case CAP_ROUND:
                stroke_path.line_cap (agg::round_cap);
                break;
                //case CAP_SQUARE:
                //    stroke_path.line_cap (agg::square_cap);
                //    break;
        }
        switch (pctx->pen->line_join_e) {
            case JOIN_MITER:
                stroke_path.line_join (agg::miter_join);
                break;
            case JOIN_ROUND:
                stroke_path.line_join (agg::round_join);
                break;
            case JOIN_BEVEL:
                stroke_path.line_join (agg::bevel_join);
                break;
            case JOIN_MILTER_REVERT:
                stroke_path.line_join (agg::miter_join_revert);
                break;
            case JOIN_MILTER_ROUND:
                stroke_path.line_join (agg::miter_join_round);
                break;
        }

        int n = 0;
        while (n < pctx->path->id)
        {
            pctx->path->m_agg_ps.rewind( pctx->path->path_id[n] );
            agg::conv_transform<agg::path_storage, agg::trans_affine>
                trans (pctx->path->m_agg_ps, mot);
            RAS_ADD_PATH(pctx->gps, trans);
            n ++;
        }

        agg::conv_transform<STROKE_DASH_TYPE, agg::trans_affine>
            trans (stroke_path, mot);

        RAS_ADD_PATH(pctx->gps, trans);
    }
    else
    {
        STROKE_STORAGE_TYPE stroke_path (pctx->path->m_agg_ps);
        stroke_path.width (pctx->pen->width);

        switch (pctx->pen->line_cap_e) {
            case CAP_BUTT:
            case CAP_SQUARE:
                stroke_path.line_cap (agg::butt_cap);
                break;
            case CAP_ROUND:
                stroke_path.line_cap (agg::round_cap);
                break;
                //case CAP_SQUARE:
                //    stroke_path.line_cap (agg::square_cap);
                //    break;
        }
        switch (pctx->pen->line_join_e) {
            case JOIN_MITER:
                stroke_path.line_join (agg::miter_join);
                break;
            case JOIN_ROUND:
                stroke_path.line_join (agg::round_join);
                break;
            case JOIN_BEVEL:
                stroke_path.line_join (agg::bevel_join);
                break;
            case JOIN_MILTER_REVERT:
                stroke_path.line_join (agg::miter_join_revert);
                break;
            case JOIN_MILTER_ROUND:
                stroke_path.line_join (agg::miter_join_round);
                break;
        }

        /* add path to rasterizer. */
        int n = 0;
        while (n < pctx->path->id) {
            pctx->path->m_agg_ps.rewind( pctx->path->path_id[n] );
            agg::conv_transform<agg::path_storage, agg::trans_affine>
                trans (pctx->path->m_agg_ps, mot);

#if 0
            if (pctx->gps->clip_ras.state) {
                pctx->gps->clip_ras.ras.add_path(trans);
            }
            else {
                ras.add_path (trans);
            }
#endif
            RAS_ADD_PATH(pctx->gps, trans);
            n ++;
        }

        agg::conv_transform<STROKE_STORAGE_TYPE, agg::trans_affine>
            trans (stroke_path, mot);

        RAS_ADD_PATH(pctx->gps, trans);
    }

    if (pctx->gps->smoothing_mode == MP_SMOOTHING_QUALITY)
        ras.gamma (agg::gamma_multiply ());

    SET_COMP_OP(pctx->gps, ren_pixf);
    SET_CLIP_BOX(ren_comp, rc);

#if 0
    if (pctx->gps->clip_ras.state) {
        typedef agg::scanline_u8_am<agg::alpha_mask_gray8> scanline_type;
        scanline_type sl(pctx->gps->clip_ras.alpha_mask);

        AGG_RENDER((pctx->gps->path_hints == MP_PATH_RENDER_HINT_ANTIALIAS_ON),
                pctx->gps->clip_ras.ras, sl, ren_comp, color);
    }
    else {
        AGG_RENDER((pctx->gps->path_hints == MP_PATH_RENDER_HINT_ANTIALIAS_ON),
                ras, sl, ren_comp, color);
    }
#endif
    AGG_RENDER_CLIP(pctx->gps, 
            (pctx->gps->path_hints == MP_PATH_RENDER_HINT_ANTIALIAS_ON),
            ras, sl, ren_comp, color);
}

static void
fill_lineargradient_forwardxy (MPGraphics *gps, MPBrush *brush, 
                                MPPath *path, agg::trans_affine& mot, RECT *rc)
{
    LinearGradientBrush* g_brush = (LinearGradientBrush*)brush->p_brush; 
    double m_left, m_right, m_top, m_bottom;

    m_left = (double) (g_brush->gradient_rect.left);
    m_right = (double) (g_brush->gradient_rect.right);
    m_top = (double) (g_brush->gradient_rect.top);
    m_bottom = (double) (g_brush->gradient_rect.bottom);

    PIXFMT_CUSTOM_TYPE      ren_pixf(gps->rendering_buff);
    RENDERER_CUSTOM_TYPE    ren_comp(ren_pixf);

    GRADIENT_X  gradient_func;                   
    agg::trans_affine   gradient_mtx;                    

    double angle, length;
    double tx, ty;
    double sx, sy;
    double rotate = mot.rotation (); 
    mot.translation (&tx, &ty);
    mot.scaling (&sx, &sy);

    rect_to_polar (&angle, &length, m_right, m_bottom, m_left, m_top, sx, sy);
    length = cos (angle) * length;
    angle = 2.0 * atan( 1.0 ) - angle;    /**< Constant value of PI */
    angle = agg::rad2deg (angle);

    double m_center_x, m_center_y;
    m_center_x = (double)((m_right + m_left)/2);
    m_center_y = (double)((m_top + m_bottom)/2);

    gradient_mtx.reset ();
    gradient_mtx.translate (-m_center_x, -m_center_y);
    gradient_mtx.rotate (agg::deg2rad (angle) + rotate);
    mot.transform (&m_center_x, &m_center_y);
    gradient_mtx.translate (m_center_x, m_center_y);
    gradient_mtx.invert ();

    SPAN_INTERPOLATOR   span_interpolator (gradient_mtx); 
    SPAN_ALLOCATOR      span_allocator;                  
    COLOR_ARRAY         color_array;                     

    //ARGB* p_gradient_colors = g_brush->gradient_colors;
    //int num = g_brush->gradient_color_num;
    
    //fill_color_array (color_array, p_gradient_colors, num);
    fill_color_array (color_array, g_brush->gradient_add, g_brush->gradient_add_num);


    SPAN_GRADIENT_X     span_gradient (span_interpolator, 
                                            gradient_func, 
                                            color_array,
                                            m_left, m_right);

    SET_CLIP_BOX(ren_comp, rc);

    RENDERER_GRADIENT_X ren_gradient (ren_comp, span_allocator, span_gradient);

    agg::rasterizer_scanline_aa<> ras;
    agg::scanline_u8 sl;

    RAS_RESET(gps);
    if (gps->clip_ras.state) {
        gps->clip_ras.ras.filling_rule ((path->m_fill_mode == MP_PATH_FILL_MODE_ALTERNATE) ? 
                agg::fill_even_odd : agg::fill_non_zero);
    }
    else
        ras.filling_rule ((path->m_fill_mode == MP_PATH_FILL_MODE_ALTERNATE) ? 
                agg::fill_even_odd : agg::fill_non_zero);

    int i = 0;
    while (i < path->id)
    {
        path->m_agg_ps.rewind (path->path_id [i]);
        agg::conv_transform<agg::path_storage, agg::trans_affine>
            trans (path->m_agg_ps, mot);
        //ras.add_path (trans);
        RAS_ADD_PATH(gps, trans);
        i ++;
    }
    agg::conv_transform<agg::path_storage, agg::trans_affine>
                                            trans (path->m_agg_ps, mot);
    //ras.add_path (trans);
    RAS_ADD_PATH(gps, trans);

    SET_COMP_OP(gps, ren_pixf);
    //AGG_RENDER2((gps->path_hints == MP_PATH_RENDER_HINT_ANTIALIAS_ON), 
    //        ras, sl, ren_comp, span_allocator, span_gradient); 
    AGG_RENDER2_CLIP(gps, 
            (gps->path_hints == MP_PATH_RENDER_HINT_ANTIALIAS_ON), 
            ras, sl, ren_comp, span_allocator, span_gradient); 
}

static void
fill_lineargradient_backwardxy (MPGraphics * gps, 
        MPBrush * brush, MPPath * path, agg::trans_affine& mot, RECT *rc)
{
    LinearGradientBrush* g_brush = (LinearGradientBrush*)brush->p_brush; 
    double m_left, m_right, m_top,m_bottom;
    double m_center_x, m_center_y;

    m_left = (double) (g_brush->gradient_rect.left);
    m_right = (double) (g_brush->gradient_rect.right);
    m_top = (double) (g_brush->gradient_rect.top);
    m_bottom = (double) (g_brush->gradient_rect.bottom);

    PIXFMT_CUSTOM_TYPE      ren_pixf(gps->rendering_buff);
    RENDERER_CUSTOM_TYPE    ren_comp(ren_pixf);

    GRADIENT_X  gradient_func;                   
    agg::trans_affine   gradient_mtx;                    

    double angle, length;
    double tx, ty;
    double sx, sy;
    double rotate = mot.rotation (); 
    mot.translation (&tx, &ty);
    mot.scaling (&sx, &sy);

    rect_to_polar (&angle, &length, m_right, m_bottom, m_left, m_top, sx, sy);
    length = cos (angle) * length;
    angle = -(2.0 * atan( 1.0 ) - angle);    /**< Constant value of PI */
    angle = agg::rad2deg (angle);

    m_center_x = (double)((m_right + m_left)/2);
    m_center_y = (double)((m_top + m_bottom)/2);

    gradient_mtx.reset ();
    gradient_mtx.translate (-m_center_x, -m_center_y);
    gradient_mtx.rotate (agg::deg2rad (angle) + rotate);
    mot.transform (&m_center_x, &m_center_y);
    gradient_mtx.translate (m_center_x, m_center_y);
    gradient_mtx.invert ();

    SPAN_INTERPOLATOR       span_interpolator (gradient_mtx); 
    SPAN_ALLOCATOR          span_allocator;                  
    COLOR_ARRAY             color_array;                     

    SPAN_GRADIENT_X         span_gradient (span_interpolator, 
                                            gradient_func,     
                                            color_array,
                                            m_left, m_right);

    SET_CLIP_BOX(ren_comp, rc);
    RENDERER_GRADIENT_X ren_gradient (ren_comp, span_allocator, span_gradient);

    agg::rasterizer_scanline_aa<> ras;
    agg::scanline_u8 sl;

    //fill_color_array (color_array, p_gradient_colors, num);
    fill_color_array (color_array, g_brush->gradient_add, g_brush->gradient_add_num);

    RAS_RESET(gps);
    if (gps->clip_ras.state) {
        gps->clip_ras.ras.filling_rule ((path->m_fill_mode == MP_PATH_FILL_MODE_ALTERNATE) ? 
                agg::fill_even_odd : agg::fill_non_zero);
    }
    else {
        ras.filling_rule ((path->m_fill_mode == MP_PATH_FILL_MODE_ALTERNATE) ? 
                agg::fill_even_odd : agg::fill_non_zero);
    }

    int i = 0;
    while (i < path->id)
    {
        path->m_agg_ps.rewind (path->path_id [i]);
        agg::conv_transform<agg::path_storage, agg::trans_affine>
            trans (path->m_agg_ps, mot);
        //ras.add_path (trans);
        RAS_ADD_PATH(gps, trans);
        i ++;
    }
    agg::conv_transform<agg::path_storage, agg::trans_affine>
        trans (path->m_agg_ps, mot);
    //ras.add_path (trans);
    RAS_ADD_PATH(gps, trans);

    SET_COMP_OP(gps, ren_pixf);
    //AGG_RENDER2((gps->path_hints == MP_PATH_RENDER_HINT_ANTIALIAS_ON), 
    //        ras, sl, ren_comp, span_allocator, span_gradient); 
    AGG_RENDER2_CLIP(gps, 
            (gps->path_hints == MP_PATH_RENDER_HINT_ANTIALIAS_ON), 
            ras, sl, ren_comp, span_allocator, span_gradient); 
}

static void
fill_lineargradient_y(MPGraphics *gps, MPBrush *brush, 
        MPPath *path, agg::trans_affine& mot, RECT *rc)
{
    LinearGradientBrush* g_brush = (LinearGradientBrush*)brush->p_brush; 
    double m_top, m_bottom, m_left, m_right;

    m_top = (double) (g_brush->gradient_rect.top);
    m_bottom = (double) (g_brush->gradient_rect.bottom);
    m_left = (double) (g_brush->gradient_rect.left);
    m_right = (double) (g_brush->gradient_rect.right);

    PIXFMT_CUSTOM_TYPE      ren_pixf(gps->rendering_buff);
    RENDERER_CUSTOM_TYPE    ren_comp(ren_pixf);

    GRADIENT_Y              gradient_func;
    agg::trans_affine       gradient_mtx;
    SPAN_INTERPOLATOR       span_interpolator(gradient_mtx);
    SPAN_ALLOCATOR          span_allocator;
    COLOR_ARRAY             color_array;
    //ARGB* p_gradient_colors = g_brush->gradient_colors;
    //int num = g_brush->gradient_color_num;

    if (m_left == m_right)
    {
        double x1, y1, x2, y2;
        if (path->id != 0) {
            agg::pod_array_adaptor<unsigned> path_idx(path->path_id, 128);

            agg::bounding_rect (path->m_agg_ps,
                    path_idx, 0,
                    path->id, &x1, &y1, &x2, &y2);
        }
        else
        {
            agg::bounding_rect_single(path->m_agg_ps, 0,
                    &x1, &y1, &x2, &y2);
        }
        m_left = x1;
        m_right = x2;
    }

    gradient_mtx.reset ();
    gradient_mtx.translate ((double)(-(m_right + m_left)/2), 
                            (double)(-(m_top + m_bottom)/2));
    gradient_mtx.premultiply(mot);
    gradient_mtx.translate ((double)((m_right + m_left)/2), 
                            (double)((m_top + m_bottom)/2));
    gradient_mtx.invert ();

    //fill_color_array (color_array, p_gradient_colors, num);
    fill_color_array (color_array, g_brush->gradient_add, g_brush->gradient_add_num);

    SPAN_GRADIENT_Y         span_gradient (span_interpolator, 
                                            gradient_func, 
                                            color_array,
                                            m_top, m_bottom);

    SET_CLIP_BOX(ren_comp, rc);

    RENDERER_GRADIENT_Y ren_gradient (ren_comp, span_allocator, span_gradient);

    agg::rasterizer_scanline_aa<> ras;
    agg::scanline_u8 sl;

    RAS_RESET(gps);
    if (gps->clip_ras.state) {
        gps->clip_ras.ras.filling_rule ((path->m_fill_mode == MP_PATH_FILL_MODE_ALTERNATE) ? 
                agg::fill_even_odd : agg::fill_non_zero);
    }
    else
        ras.filling_rule ((path->m_fill_mode == MP_PATH_FILL_MODE_ALTERNATE) ? 
                agg::fill_even_odd : agg::fill_non_zero);

    int i = 0;
    while (i < path->id)
    {
        path->m_agg_ps.rewind (path->path_id [i]);
        agg::conv_transform<agg::path_storage, agg::trans_affine>
            trans (path->m_agg_ps, mot);
        //ras.add_path (trans);
        RAS_ADD_PATH(gps, trans);
        i ++;
    }

    agg::conv_transform<agg::path_storage, agg::trans_affine>
                                trans (path->m_agg_ps, mot);
    //ras.add_path (trans);
    RAS_ADD_PATH(gps, trans);

    SET_COMP_OP(gps, ren_pixf);
    //AGG_RENDER2((gps->path_hints == MP_PATH_RENDER_HINT_ANTIALIAS_ON), 
    //        ras, sl, ren_comp, span_allocator, span_gradient); 
    AGG_RENDER2_CLIP(gps, 
            (gps->path_hints == MP_PATH_RENDER_HINT_ANTIALIAS_ON), 
            ras, sl, ren_comp, span_allocator, span_gradient); 
}


static void
fill_solidcolor(MPGraphics *gps, MPBrush *brush, MPPath *path, 
        agg::trans_affine& mot, RECT *rc)
{
    int i = 0;
    SolidBrush* p_solid_brush = (SolidBrush*) brush->p_brush;

    agg::rasterizer_scanline_aa<> ras;
    agg::scanline_u8 sl;

    PIXFMT_CUSTOM_TYPE      ren_pixf (gps->rendering_buff);
    RENDERER_CUSTOM_TYPE    ren_comp (ren_pixf);

    //ras.reset ();
    RAS_RESET(gps);
    if (gps->clip_ras.state) {
        gps->clip_ras.ras.filling_rule((path->m_fill_mode == MP_PATH_FILL_MODE_ALTERNATE) ? 
                agg::fill_even_odd : agg::fill_non_zero);
    }
    else 
        ras.filling_rule((path->m_fill_mode == MP_PATH_FILL_MODE_ALTERNATE) ? 
                agg::fill_even_odd : agg::fill_non_zero);

    while (i < path->id) {
        path->m_agg_ps.rewind (path->path_id [i]);
        agg::conv_transform<agg::path_storage, agg::trans_affine>
            trans (path->m_agg_ps, mot);
        //ras.add_path (trans);
        RAS_ADD_PATH(gps, trans);
        i ++;
    }
    agg::conv_transform<agg::path_storage, agg::trans_affine>
        trans (path->m_agg_ps, mot);

    //ras.add_path (trans);
    RAS_ADD_PATH(gps, trans);

    if (gps->smoothing_mode == MP_SMOOTHING_QUALITY)
        ras.gamma (agg::gamma_multiply ());

    agg::rgba8 color (MPGetRValue(p_solid_brush->rgba),
                                 MPGetGValue(p_solid_brush->rgba),
                                 MPGetBValue(p_solid_brush->rgba),
                                 MPGetAValue(p_solid_brush->rgba));

    SET_COMP_OP(gps, ren_pixf);
    SET_CLIP_BOX(ren_comp, rc);
    //AGG_RENDER((gps->path_hints == MP_PATH_RENDER_HINT_ANTIALIAS_ON), 
    //        ras, sl, ren_comp, color);
    AGG_RENDER_CLIP(gps, 
            (gps->path_hints == MP_PATH_RENDER_HINT_ANTIALIAS_ON), 
            ras, sl, ren_comp, color);
}

static void
fill_pathgradient_single_color (MPGraphics *gps, MPBrush *brush, 
        MPPath *path, agg::trans_affine& mot, RECT *rc)
{
    PathGradientBrush* p_brush = (PathGradientBrush*) brush->p_brush;
    agg::scanline_u8    sl;
    GRADIENT_LUT        m_gradient_lut;
    GAMMA_LUT           m_gamma_lut;
    SPAN_ALLOCATOR      m_alloc;
    agg::rasterizer_scanline_aa<>   ras;
    int width, height;

    PIXFMT_CUSTOM_TYPE      ren_pixf(gps->rendering_buff);
    RENDERER_CUSTOM_TYPE    ren_comp(ren_pixf);
#if 0
    PIXFMT          pixf(gps->rendering_buff);
    RENDERER_BASE   rb(pixf);
#endif

    width = RECTW (p_brush->rect); 
    height = RECTH (p_brush->rect); 

    double center_x = (double)(p_brush->rect.left + width / 2);
    double center_y = (double)(p_brush->rect.top + height / 2);
    double radial;
    int i;
    int total_num = p_brush->surround_rgba_num;

    if (total_num == 0)
        return;

    unsigned int r, g, b, a;
    ARGB center_rgba = p_brush->center_rgba;
    ARGB* p_surround_rgba = p_brush->surround_rgba;

    m_gradient_lut.remove_all();
    m_gamma_lut.gamma (1.8);

    r = (unsigned int) (MPGetRValue (center_rgba));
    g = (unsigned int) (MPGetGValue (center_rgba));
    b = (unsigned int) (MPGetBValue (center_rgba));
    a = (unsigned int) (MPGetAValue (center_rgba));

    m_gradient_lut.add_color(0.0, 
                    agg::rgba8_gamma_dir(agg::rgba8 (r, g, b, a), m_gamma_lut));
        
    for (i = 1; i <= total_num; i++)
    {
        double scale = (double)i;
        scale = scale / total_num;

        r = (unsigned int) (MPGetRValue (p_surround_rgba [i - 1]));
        g = (unsigned int) (MPGetGValue (p_surround_rgba [i - 1]));
        b = (unsigned int) (MPGetBValue (p_surround_rgba [i - 1]));
        a = (unsigned int) (MPGetAValue (p_surround_rgba [i - 1]));
        m_gradient_lut.add_color (scale, 
                agg::rgba8_gamma_dir(agg::rgba8 (r, g, b, a),   m_gamma_lut));
    }

    m_gradient_lut.build_lut ();

    if (width > height)
        radial = (double)width;
    else
        radial = (double)height;

    //ras.reset();
    RAS_RESET(gps);
    if (gps->clip_ras.state) {
        gps->clip_ras.ras.filling_rule(
                (path->m_fill_mode == MP_PATH_FILL_MODE_ALTERNATE)? 
                agg::fill_even_odd : agg::fill_non_zero);
    }
    else
        ras.filling_rule(
                (path->m_fill_mode == MP_PATH_FILL_MODE_ALTERNATE)? 
                agg::fill_even_odd : agg::fill_non_zero);

    int j = 0;
    while (j < path->id) {
        path->m_agg_ps.rewind (path->path_id [i]);
        agg::conv_transform<agg::path_storage, agg::trans_affine>
            trans (path->m_agg_ps, mot);
        //ras.add_path (trans);
        RAS_ADD_PATH(gps, trans);
        j ++;
    }

    agg::conv_transform<agg::path_storage, agg::trans_affine>
                    trans (path->m_agg_ps, mot);

    //ras.add_path (trans);
    RAS_ADD_PATH(gps, trans);

    GRADIENT_RADIAL         gradient_func (radial, 0, 0);
    GRADIENT_ADAPTOR_RADIAL gradient_adaptor (gradient_func);
    agg::trans_affine       gradient_mtx;
    double trans_x, trans_y;

    gradient_mtx.reset();
    mot.translation (&trans_x, &trans_y);
    gradient_mtx.translate (-trans_x, -trans_y);
    gradient_mtx.premultiply (mot);
    mot.transform (&center_x, &center_y);
    gradient_mtx.translate (center_x, center_y);
    gradient_mtx.invert();

    SPAN_INTERPOLATOR   span_interpolator (gradient_mtx);
    SPAN_GRADIENT_RADIAL    span_gradient (span_interpolator, gradient_adaptor,
            m_gradient_lut, 0, radial);

    SET_CLIP_BOX(ren_comp, rc);
    SET_COMP_OP(gps, ren_pixf);
    //agg::render_scanlines_aa (ras, sl, 
    //        rb, m_alloc, span_gradient);
    //AGG_RENDER2((gps->path_hints == MP_PATH_RENDER_HINT_ANTIALIAS_ON), 
    //        ras, sl, ren_comp, m_alloc, span_gradient); 
    AGG_RENDER2_CLIP(gps, 
            (gps->path_hints == MP_PATH_RENDER_HINT_ANTIALIAS_ON), 
            ras, sl, ren_comp, m_alloc, span_gradient); 
    return;
}

static void
fill_pathgradient_multi_color (MPGraphics *gps, MPBrush *brush, 
        MPPath *path, agg::trans_affine& mot, RECT *rc)
{
    PathGradientBrush* p_brush = (PathGradientBrush*) brush->p_brush;
    unsigned int i;
    double cneter_pt_r, cneter_pt_g, cneter_pt_b, cneter_pt_a;
    double prev_pt_r, prev_pt_g, prev_pt_b, prev_pt_a;
    double next_pt_r, next_pt_g, next_pt_b, next_pt_a;
    double x1, y1, x2, y2; 
    double cx, cy;
    BOOL b_gradient = FALSE;

    PIXFMT_CUSTOM_TYPE      ren_pixf(gps->rendering_buff);
    RENDERER_CUSTOM_TYPE    ren_comp(ren_pixf);

    ARGB center_rgba = p_brush->center_rgba;
    ARGB* p_surround_rgba = p_brush->surround_rgba;

    cneter_pt_r = (double) (MPGetRValue (center_rgba) / 256);
    cneter_pt_g = (double) (MPGetGValue (center_rgba) / 256);
    cneter_pt_b = (double) (MPGetBValue (center_rgba) / 256);
    cneter_pt_a = (double) (MPGetAValue (center_rgba) / 256);

    cx = (double) p_brush->center_point.x;
    cy = (double) p_brush->center_point.y;

    if (p_brush->surround_rgba_num == 1)
        return;

    mot.transform (&cx, &cy);

    agg::scanline_u8                sl;
    agg::rasterizer_scanline_aa<>   ras;
    SPAN_ALLOCATOR          span_alloc;
    SPAN_GOURAUD            span_gen;

    SET_CLIP_BOX(ren_comp, rc);

    for(i = 1; i <= path->m_agg_ps.total_vertices(); i++)
    {
        b_gradient = FALSE;
        if (i == path->m_agg_ps.total_vertices ())
        {
            path->m_agg_ps.vertex (i - 1, &x1, &y1);
            path->m_agg_ps.vertex (0, &x2, &y2);
        }
        else
        {
            path->m_agg_ps.vertex (i - 1, &x1, &y1);
            path->m_agg_ps.vertex (i, &x2, &y2);
        }

        if (i < p_brush->surround_rgba_num)
        {
            b_gradient = TRUE; 
            prev_pt_r = (double) (MPGetRValue (p_surround_rgba [i - 1]));
            prev_pt_g = (double) (MPGetGValue (p_surround_rgba [i - 1]));
            prev_pt_b = (double) (MPGetBValue (p_surround_rgba [i - 1]));
            prev_pt_a = (double) (MPGetAValue (p_surround_rgba [i - 1]));

            prev_pt_r = prev_pt_r / 255;
            prev_pt_g = prev_pt_g / 255;
            prev_pt_b = prev_pt_b / 255;
            prev_pt_a = prev_pt_a / 255;

            next_pt_r = (double) (MPGetRValue (p_surround_rgba [i]));
            next_pt_g = (double) (MPGetGValue (p_surround_rgba [i]));
            next_pt_b = (double) (MPGetBValue (p_surround_rgba [i]));
            next_pt_a = (double) (MPGetAValue (p_surround_rgba [i]));

            next_pt_r = next_pt_r / 255;
            next_pt_g = next_pt_g / 255;
            next_pt_b = next_pt_b / 255;
            next_pt_a = next_pt_a / 255;
        }
        else
        {
            if (i >= p_brush->surround_rgba_num)
            {
                b_gradient = TRUE; 
                prev_pt_r = (double) (MPGetRValue (p_surround_rgba [p_brush->surround_rgba_num - 1]));
                prev_pt_g = (double) (MPGetGValue (p_surround_rgba [p_brush->surround_rgba_num - 1]));
                prev_pt_b = (double) (MPGetBValue (p_surround_rgba [p_brush->surround_rgba_num - 1]));
                prev_pt_a = (double) (MPGetAValue (p_surround_rgba [p_brush->surround_rgba_num - 1]));

                prev_pt_r = prev_pt_r / 255;
                prev_pt_g = prev_pt_g / 255;
                prev_pt_b = prev_pt_b / 255;
                prev_pt_a = prev_pt_a / 255;

                next_pt_r = (double) (MPGetRValue (p_surround_rgba [0]));
                next_pt_g = (double) (MPGetGValue (p_surround_rgba [0]));
                next_pt_b = (double) (MPGetBValue (p_surround_rgba [0]));
                next_pt_a = (double) (MPGetAValue (p_surround_rgba [0]));

                next_pt_r = next_pt_r / 255;
                next_pt_g = next_pt_g / 255;
                next_pt_b = next_pt_b / 255;
                next_pt_a = next_pt_a / 255;
            }
            else
            {
                break;
            }
        }

        if (b_gradient == TRUE) 
        {
            span_gen.colors( 
                agg::rgba(prev_pt_r, prev_pt_g, prev_pt_b, prev_pt_a),
                agg::rgba(next_pt_r, next_pt_g, next_pt_b, next_pt_a),
                agg::rgba(cneter_pt_r, cneter_pt_g, cneter_pt_b, cneter_pt_a));

            mot.transform (&x1, &y1);
            mot.transform (&x2, &y2);
            span_gen.triangle (x1, y1, x2, y2, cx, cy, 0.17);

            //ras.reset ();
            //ras.add_path (span_gen);
            RAS_RESET(gps);
            RAS_ADD_PATH(gps, span_gen);
            if (gps->clip_ras.state) {
                gps->clip_ras.ras.filling_rule((path->m_fill_mode == MP_PATH_FILL_MODE_ALTERNATE)? 
                        agg::fill_even_odd : agg::fill_non_zero);
            }
            else
                ras.filling_rule((path->m_fill_mode == MP_PATH_FILL_MODE_ALTERNATE)? 
                        agg::fill_even_odd : agg::fill_non_zero);

            SET_COMP_OP(gps, ren_pixf);
            //AGG_RENDER2((gps->path_hints == MP_PATH_RENDER_HINT_ANTIALIAS_ON), 
            //        ras, sl, ren_comp, span_alloc, span_gen); 
            AGG_RENDER2_CLIP(gps, 
                    (gps->path_hints == MP_PATH_RENDER_HINT_ANTIALIAS_ON), 
                    ras, sl, ren_comp, span_alloc, span_gen); 
        }
    }

    return;
}

static void
fill_lineargradient_x(MPGraphics *gps, MPBrush *brush, 
        MPPath *path, agg::trans_affine& mot, RECT *rc)
{
    LinearGradientBrush* g_brush = (LinearGradientBrush*)brush->p_brush; 
    double m_left, m_right, m_top, m_bottom;

    m_left = (double) (g_brush->gradient_rect.left);
    m_right = (double) (g_brush->gradient_rect.right);
    m_top = (double) (g_brush->gradient_rect.top);
    m_bottom = (double) (g_brush->gradient_rect.bottom);

    if (m_top == m_bottom)
    {
        double x1, y1, x2, y2;
        if (path->id != 0) {
            agg::pod_array_adaptor<unsigned> path_idx(path->path_id, 128);

            agg::bounding_rect (path->m_agg_ps,
                    path_idx, 0,
                    path->id, &x1, &y1, &x2, &y2);
        }
        else
        {
            agg::bounding_rect_single(path->m_agg_ps, 0,
                    &x1, &y1, &x2, &y2);
        }
        m_top = y1;
        m_bottom = y2;
    }

    agg::trans_affine   gradient_mtx;                    
    gradient_mtx.reset ();
    gradient_mtx.translate ((double)(-(m_right + m_left)/2), (double)(-(m_top + m_bottom)/2));
    gradient_mtx.premultiply (mot);
    gradient_mtx.translate ((double)((m_right + m_left)/2), (double)((m_top + m_bottom)/2));
    gradient_mtx.invert ();

    PIXFMT_CUSTOM_TYPE      ren_pixf(gps->rendering_buff);
    RENDERER_CUSTOM_TYPE    ren_comp(ren_pixf);

    GRADIENT_X  gradient_func;                   
    SPAN_INTERPOLATOR       span_interpolator(gradient_mtx); 
    SPAN_ALLOCATOR          span_allocator;                  
    COLOR_ARRAY             color_array;                     
    //ARGB* p_gradient_colors = g_brush->gradient_colors;
    //int num = g_brush->gradient_color_num;

    //fill_color_array (color_array, p_gradient_colors, num);
    fill_color_array (color_array, g_brush->gradient_add, g_brush->gradient_add_num);

    SPAN_GRADIENT_X         span_gradient (span_interpolator, 
            gradient_func, 
            color_array,
            m_left, m_right);

    SET_CLIP_BOX(ren_comp, rc);

    RENDERER_GRADIENT_X ren_gradient(ren_comp, span_allocator, span_gradient);

    agg::rasterizer_scanline_aa<>   ras;
    agg::scanline_u8                sl;

    if (gps->clip_ras.state) {
        gps->clip_ras.ras.filling_rule((path->m_fill_mode == MP_PATH_FILL_MODE_ALTERNATE)? 
                agg::fill_even_odd : agg::fill_non_zero);
    }
    else {
        ras.filling_rule((path->m_fill_mode == MP_PATH_FILL_MODE_ALTERNATE)? 
                agg::fill_even_odd : agg::fill_non_zero);
    }

    int i = 0;
    while (i < path->id)
    {
        path->m_agg_ps.rewind (path->path_id [i]);
        agg::conv_transform<agg::path_storage, agg::trans_affine>
            trans (path->m_agg_ps, mot);

        //ras.add_path (trans);
        RAS_ADD_PATH(gps, trans);
        i ++;
    }
    agg::conv_transform<agg::path_storage, agg::trans_affine>
                            trans (path->m_agg_ps, mot);
    //ras.add_path (trans);
    RAS_ADD_PATH(gps, trans);

    SET_COMP_OP(gps, ren_pixf);
    //AGG_RENDER2((gps->path_hints == MP_PATH_RENDER_HINT_ANTIALIAS_ON), 
    //        ras, sl, ren_comp, span_allocator, span_gradient); 
    AGG_RENDER2_CLIP(gps, 
            (gps->path_hints == MP_PATH_RENDER_HINT_ANTIALIAS_ON), 
            ras, sl, ren_comp, span_allocator, span_gradient); 
}

static void
fill_lineargradient (MPGraphics *gps, MPBrush *brush, MPPath *path, 
       agg::trans_affine& mot,  RECT *rc)
{
    LinearGradientBrush* p_brush = (LinearGradientBrush*)brush->p_brush; 

    switch (p_brush->gradient_type)
    {
    case MP_LINEAR_GRADIENT_MODE_HORIZONTAL:
        return fill_lineargradient_x(gps, brush, path, mot, rc);
    case MP_LINEAR_GRADIENT_MODE_VERTICAL:
        return fill_lineargradient_y(gps, brush, path, mot, rc);
    case MP_LINEAR_GRADIENT_MODE_FORWARDDIAGONAL:
        return fill_lineargradient_forwardxy(gps, brush, path, mot, rc);
    case MP_LINEAR_GRADIENT_MODE_BACKWARDDIAGONAL:
        return fill_lineargradient_backwardxy(gps, brush, path, mot, rc);
    }
    return;
}

static void
fill_path(HDC hdc, Uint8* pixels, int pitch, int bpp, const RECT* rc, void* ctx)
{
    CTX_FILL_PATH* pctx = (CTX_FILL_PATH *)ctx;
    POINT pt={0, 0};
    LPtoSP(hdc, &pt);
    agg::trans_affine mot;
    mot.multiply(pctx->mot);
    mot.translate(pt.x, pt.y);

    switch (pctx->brush->brush_type)
    {
        case MP_BRUSH_TYPE_SOLIDCOLOR:   
            return fill_solidcolor (pctx->gps, pctx->brush, pctx->path, 
                                    mot, (RECT*)rc);
        case MP_BRUSH_TYPE_HATCHFILL:
             break;
        case MP_BRUSH_TYPE_TEXTUREFILL:
             break;
        case MP_BRUSH_TYPE_PATHGRADIENT:
        {
             PathGradientBrush* p_brush = 
                            (PathGradientBrush*) pctx->brush->p_brush;

             if (RECTW(p_brush->rect) && RECTH(p_brush->rect))
                 return fill_pathgradient_single_color (pctx->gps, 
                                    pctx->brush, pctx->path, mot, (RECT*)rc);
             else
                 return fill_pathgradient_multi_color (pctx->gps, 
                                    pctx->brush, pctx->path, mot, (RECT*)rc);
        }
        case MP_BRUSH_TYPE_LINEARGRADIENT:
            return fill_lineargradient(pctx->gps, pctx->brush, 
                                            pctx->path, mot, (RECT*)rc);
    }
}

static void
draw_image_with_pts(HDC hdc, Uint8* pixels, int pitch, int bpp, 
        const RECT* rc, void* ctx)
{
    CTX_IMAGE_PTS *pctx = (CTX_IMAGE_PTS *)ctx;
    POINT pt={0, 0};
    LPtoSP(hdc, &pt);
    agg::trans_affine mot;
    mot.multiply(pctx->mot);
    mot.translate(pt.x, pt.y);

    agg::image_filter_bilinear  filter_kernel;
    agg::image_filter_lut       filter(filter_kernel, false);
    IMG_SPAN_ALLOCATOR_DEFAULT  sa;
    agg::scanline_u8    sl;

    PIXFMT_CUSTOM_TYPE      ren_pixf(pctx->gps->rendering_buff);
    RENDERER_CUSTOM_TYPE    ren_comp(ren_pixf);

    IMG_PIXFMT          img_pixf(pctx->gps->rendering_img[pctx->idx]);
    agg::rasterizer_scanline_aa<> ras;

    MPPOINT trans_point [4];
    double store_pts [8];
    int i;

    /* reset ras.*/
    RAS_RESET(pctx->gps);

    SET_CLIP_BOX(ren_comp, rc);

    for (i = 0; i < pctx->count; i++) {
        trans_point [i].x = pctx->pt[i].x;
        trans_point [i].y = pctx->pt[i].y;
        store_pts [i * 2] = pctx->pt[i].x;
        store_pts [i * 2 + 1] = pctx->pt[i].y;
    }

    if (pctx->count < 4 || 
        (pctx->gps->interpolation_mode == MP_INTERPOLATION_MODE_AFFINE))
    {
        trans_point [3].x = pctx->pt[2].x - (pctx->pt[1].x - pctx->pt[0].x);
        trans_point [3].y = pctx->pt[0].y + (pctx->pt[2].y - pctx->pt[1].y);
        store_pts [6] = trans_point [i].x;
        store_pts [7] = trans_point [i].y;
    }

    for (i = 0; i < 4; i++)
        mot.transform (&store_pts [2*i],
                &store_pts [2*i + 1]);

    for (i = 0; i < 4; i++) {
        double d_trans_x = (double) trans_point [i].x;
        double d_trans_y = (double) trans_point [i].y;

        mot.transform (&d_trans_x, &d_trans_y);
        trans_point [i].x = (float)d_trans_x;
        trans_point [i].y = (float)d_trans_y;
    }

    if (pctx->gps->clip_ras.state) {
        pctx->gps->clip_ras.ras.move_to_d (trans_point [0].x, trans_point [0].y);
        pctx->gps->clip_ras.ras.line_to_d (trans_point [1].x, trans_point [1].y);
        pctx->gps->clip_ras.ras.line_to_d (trans_point [2].x, trans_point [2].y);
        pctx->gps->clip_ras.ras.line_to_d (trans_point [3].x, trans_point [3].y);
    }
    else {
        ras.move_to_d (trans_point [0].x, trans_point [0].y);
        ras.line_to_d (trans_point [1].x, trans_point [1].y);
        ras.line_to_d (trans_point [2].x, trans_point [2].y);
        ras.line_to_d (trans_point [3].x, trans_point [3].y);
    }

    PBITMAP p_suf_img = pctx->gps->surf_img [pctx->idx];
    unsigned char brightness_alpha_array[agg::span_conv_brightness_alpha_rgb8::array_size];

    
    i = 0;

    while (i < agg::span_conv_brightness_alpha_rgb8::array_size)
    {
        brightness_alpha_array[i] =
            agg::int8u(pctx->gps->img_alpha);
        i ++;
    }
    agg::span_conv_brightness_alpha_rgb8 color_alpha(brightness_alpha_array);

    switch (pctx->gps->interpolation_mode)
    {
        case MP_INTERPOLATION_MODE_BILINEAR:
            {
                agg::trans_bilinear img_bi(&store_pts [0], 0, 0,
                        p_suf_img->bmWidth, p_suf_img->bmHeight);

                AIMG_ACCESSOR_TYPE      ac_img (img_pixf);
                BILINEAR_INTERPOLATOR   bi_img (img_bi);
                ASPAN_GEN_BILINEAR      sg_img (ac_img, bi_img, filter);

                typedef agg::span_converter <ASPAN_GEN_BILINEAR, 
                        agg::span_conv_brightness_alpha_rgb8> span_conv;

                span_conv sc (sg_img, color_alpha);
                SET_COMP_OP(pctx->gps, ren_pixf);

                //AGG_RENDER2((pctx->gps->path_hints == MP_PATH_RENDER_HINT_ANTIALIAS_ON), 
                //        ras, sl, ren_comp, sa, sc); 
                AGG_RENDER2_CLIP(pctx->gps, 
                        (pctx->gps->path_hints == MP_PATH_RENDER_HINT_ANTIALIAS_ON), 
                        ras, sl, ren_comp, sa, sc); 
            }
            break;
        case MP_INTERPOLATION_MODE_AFFINE:
            {
                agg::trans_affine img_affine (&store_pts [0], 0, 0,
                        p_suf_img->bmWidth, p_suf_img->bmHeight);

                AIMG_ACCESSOR_TYPE   acc_img (img_pixf);
                AFFINE_INTERPOLATOR  affine_img (img_affine);
                ASPAN_GEN_AFFINE     sg_img (acc_img, affine_img, filter);

                typedef agg::span_converter <ASPAN_GEN_AFFINE, 
                        agg::span_conv_brightness_alpha_rgb8> span_conv;

                span_conv sc (sg_img, color_alpha);

                //agg::render_scanlines_aa (ras, sl, rb_screen_pre, sa, sg_img);
                SET_COMP_OP(pctx->gps, ren_pixf);
                //AGG_RENDER2((pctx->gps->path_hints == MP_PATH_RENDER_HINT_ANTIALIAS_ON), 
                //        ras, sl, ren_comp, sa, sc); 
                AGG_RENDER2_CLIP(pctx->gps, 
                        (pctx->gps->path_hints == MP_PATH_RENDER_HINT_ANTIALIAS_ON), 
                        ras, sl, ren_comp, sa, sc); 
            }
            break;
        default:
            {
                agg::trans_perspective img_per(&store_pts [0], 0, 0,
                        p_suf_img->bmWidth, p_suf_img->bmHeight);

                AIMG_ACCESSOR_TYPE        ac_img (img_pixf);
                PERSPECTIVE_INTERPOLATOR per_img (img_per);
                ASPAN_GEN_PERSPECTIVE     sg_img (ac_img, per_img, filter);
                typedef agg::span_converter <ASPAN_GEN_PERSPECTIVE, 
                        agg::span_conv_brightness_alpha_rgb8> span_conv;

                span_conv sc (sg_img, color_alpha);
                //agg::render_scanlines_aa (ras, sl, rb_screen_pre, sa, sg_img);
                SET_COMP_OP(pctx->gps, ren_pixf);

                //AGG_RENDER2((pctx->gps->path_hints == MP_PATH_RENDER_HINT_ANTIALIAS_ON), 
                //        ras, sl, ren_comp, sa, sc); 
                AGG_RENDER2_CLIP(pctx->gps, 
                        (pctx->gps->path_hints == MP_PATH_RENDER_HINT_ANTIALIAS_ON), 
                        ras, sl, ren_comp, sa, sc); 
            }
            break;
    }
}

static void
draw_image_with_path(HDC hdc, Uint8* pixels, int pitch, int bpp, 
        const RECT* rc, void* ctx)
{
    CTX_IMAGE_PATH *pctx = (CTX_IMAGE_PATH *) ctx;
    POINT pt={0, 0};
    LPtoSP(hdc, &pt);
    agg::trans_affine mot;
    mot.multiply(pctx->mot);
    mot.translate(pt.x, pt.y);

    agg::image_filter_bilinear  filter_kernel;
    agg::image_filter_lut       filter (filter_kernel, false);
    IMG_SPAN_ALLOCATOR_DEFAULT  sa;

    agg::rasterizer_scanline_aa<> ras;
    agg::scanline_u8        sl;

    PIXFMT_CUSTOM_TYPE      ren_pixf(pctx->gps->rendering_buff);
    RENDERER_CUSTOM_TYPE    ren_comp(ren_pixf);

    double x1, y1, x2, y2;

    if (pctx->path->id != 0) {
        agg::pod_array_adaptor<unsigned> path_idx(pctx->path->path_id, 128);

        agg::bounding_rect (pctx->path->m_agg_ps,
                path_idx, 0,
                pctx->path->id, &x1, &y1, &x2, &y2);
    }
    else
    {
        agg::bounding_rect_single(pctx->path->m_agg_ps, 0,
                &x1, &y1, &x2, &y2);
    }

    SET_CLIP_BOX(ren_comp, rc);

    IMG_PIXFMT img_pixf (pctx->gps->rendering_img [pctx->idx]);
    double store_pts [8];

    store_pts [0] = x1;
    store_pts [1] = y1;

    store_pts [2] = x2;
    store_pts [3] = store_pts [1];

    store_pts [4] = store_pts [2];
    store_pts [5] = y2;

    store_pts [6] = store_pts [0];
    store_pts [7] = store_pts [5];

    for (int i = 0; i < 4; i++) {
        mot.transform (&store_pts [2*i],
                &store_pts [2*i + 1]);
    }

    //ras.reset ();
    RAS_RESET(pctx->gps);

    PBITMAP p_suf_img = pctx->gps->surf_img [pctx->idx];
    int i = 0;
    while (i < pctx->path->id) {
        pctx->path->m_agg_ps.rewind (pctx->path->path_id [i]);
        agg::conv_transform<agg::path_storage, agg::trans_affine>
                            trans (pctx->path->m_agg_ps, mot);
        //ras.add_path (trans);
        RAS_ADD_PATH(pctx->gps, trans);
        i ++;
    }
    agg::conv_transform<agg::path_storage, agg::trans_affine>
                trans (pctx->path->m_agg_ps, mot);
    //ras.add_path (trans);
    RAS_ADD_PATH(pctx->gps, trans);

    unsigned char brightness_alpha_array[agg::span_conv_brightness_alpha_rgb8::array_size];

    i = 0;
    while (i < agg::span_conv_brightness_alpha_rgb8::array_size)
    {
        brightness_alpha_array[i] =
            agg::int8u(pctx->gps->img_alpha);
        i ++;
    }
    agg::span_conv_brightness_alpha_rgb8 color_alpha(brightness_alpha_array);

    switch (pctx->gps->interpolation_mode)
    {
        case MP_INTERPOLATION_MODE_BILINEAR:
        {
            agg::trans_bilinear img_bilinear (&store_pts [0], 0, 0,
                    p_suf_img->bmWidth, p_suf_img->bmHeight);

            AIMG_ACCESSOR_TYPE       acc_img (img_pixf);
            BILINEAR_INTERPOLATOR   bili_img (img_bilinear);
            ASPAN_GEN_BILINEAR       sg_img(acc_img, bili_img, filter);

            typedef agg::span_converter <ASPAN_GEN_BILINEAR, 
                    agg::span_conv_brightness_alpha_rgb8> span_conv;

            span_conv sc (sg_img, color_alpha);
            SET_COMP_OP(pctx->gps, ren_pixf);

#if 0
            if (pctx->gps->clip_ras.state) {
                typedef agg::scanline_u8_am<agg::alpha_mask_gray8> scanline_type;              
                scanline_type sl(gps->clip_ras.alpha_mask);                                    
                AGG_RENDER2((gps->path_hints == MP_PATH_RENDER_HINT_ANTIALIAS_ON),              
                        gps->clip_ras.ras, sl, ren_comp, sa, sc);                               
            }
            else
                AGG_RENDER2((pctx->gps->path_hints == MP_PATH_RENDER_HINT_ANTIALIAS_ON), 
                        ras, sl, ren_comp, sa, sc); 
#endif
            AGG_RENDER2_CLIP(pctx->gps, 
                    (pctx->gps->path_hints == MP_PATH_RENDER_HINT_ANTIALIAS_ON), 
                    ras, sl, ren_comp, sa, sc); 
            break;
        }
        case MP_INTERPOLATION_MODE_AFFINE:
        {
            agg::trans_affine img_affine (&store_pts [0], 0, 0,
                    p_suf_img->bmWidth, p_suf_img->bmHeight);

            AIMG_ACCESSOR_TYPE      acc_img (img_pixf);
            AFFINE_INTERPOLATOR     affine_img (img_affine);
            ASPAN_GEN_AFFINE        sg_img (acc_img, affine_img, filter);

            typedef agg::span_converter <ASPAN_GEN_AFFINE, 
                    agg::span_conv_brightness_alpha_rgb8> span_conv;

            span_conv sc (sg_img, color_alpha);
            SET_COMP_OP(pctx->gps, ren_pixf);
            //AGG_RENDER2((pctx->gps->path_hints == MP_PATH_RENDER_HINT_ANTIALIAS_ON), 
            //        ras, sl, ren_comp, sa, sc); 
            AGG_RENDER2_CLIP(pctx->gps, 
                    (pctx->gps->path_hints == MP_PATH_RENDER_HINT_ANTIALIAS_ON), 
                    ras, sl, ren_comp, sa, sc); 
            break;
        }
        default:
        {
            agg::trans_perspective img_per(&store_pts [0], 0, 0,
                    p_suf_img->bmWidth, p_suf_img->bmHeight);

            AIMG_ACCESSOR_TYPE acc_img (img_pixf);
            PERSPECTIVE_INTERPOLATOR per_img (img_per);
            ASPAN_GEN_PERSPECTIVE sg_img (acc_img, per_img, filter);

            typedef agg::span_converter <ASPAN_GEN_PERSPECTIVE, 
                    agg::span_conv_brightness_alpha_rgb8> span_conv;

            span_conv sc (sg_img, color_alpha);
            SET_COMP_OP(pctx->gps, ren_pixf);

            //AGG_RENDER2((pctx->gps->path_hints == MP_PATH_RENDER_HINT_ANTIALIAS_ON), 
            //        ras, sl, ren_comp, sa, sc); 
            AGG_RENDER2_CLIP(pctx->gps, 
                    (pctx->gps->path_hints == MP_PATH_RENDER_HINT_ANTIALIAS_ON), 
                    ras, sl, ren_comp, sa, sc); 
            break;
        }
    }
}

#ifdef _MGPLUS_FONT_FT2
static void
draw_glyph(HDC hdc, Uint8* pixels, int pitch, int bpp, 
        const RECT* rc, void* ctx)
{
    CTX_DRAW_GLYPH *pctx = (CTX_DRAW_GLYPH *) ctx;
    POINT pt={0, 0};
    LPtoSP(hdc, &pt);
    agg::trans_affine mot;
    mot.multiply(pctx->mot);
    mot.translate(pt.x, pt.y);

    PATH_ADAPTOR          path_adaptor;

    PIXFMT_CUSTOM_TYPE    ren_pixf(pctx->gps->rendering_buff);
    RENDERER_CUSTOM_TYPE  ren_comp(ren_pixf);

    agg::rgba8            color(MPGetRValue(pctx->color), 
                                MPGetGValue(pctx->color), 
                                MPGetBValue(pctx->color), 
                                MPGetAValue(pctx->color));

    SET_CLIP_BOX(ren_comp, rc);

    pctx->font->m_fman->init_embedded_adaptors(
            (agg::glyph_cache*)pctx->lpdata, pctx->x, pctx->y);

    switch (pctx->lpdata->data_type){
        case GLYPH_DATA_MONO:
        {
            path_adaptor.init((agg::int8u*)pctx->lpdata->data, 
                    pctx->lpdata->data_size, pctx->x, pctx->y, 1.0);

            agg::conv_transform<PATH_ADAPTOR, agg::trans_affine>
                trans (path_adaptor, mot);

            double x, y;
            unsigned cmd;
            trans.rewind(0);
            int i = 0;
            while(!((cmd = trans.vertex(&x, &y)) == agg::path_cmd_stop)) {
                //path_adaptor.modify_vertex(i, &x, &y, cmd);
                i ++;
            }

            agg::render_scanlines_bin_solid(pctx->font->m_fman->mono_adaptor(), 
                    pctx->font->m_fman->mono_scanline(), ren_comp, color);
            break;
        }

        case GLYPH_DATA_GRAY8:
        {
            path_adaptor.init((agg::int8u*)pctx->lpdata->data, 
                    pctx->lpdata->data_size, pctx->x, pctx->y, 1.0);

            agg::conv_transform<PATH_ADAPTOR, agg::trans_affine>
                trans (path_adaptor, mot);

            double x, y;
            unsigned cmd;
            trans.rewind(0);
            while (!((cmd = trans.vertex(&x, &y)) == agg::path_cmd_stop));

            AGG_RENDER(1, pctx->font->m_fman->gray8_adaptor(), 
                    pctx->font->m_fman->gray8_scanline(), ren_comp, color);
            break;
        }

        case GLYPH_DATA_OUTLINE:
        {
            agg::scanline_u8 sl;
            agg::rasterizer_scanline_aa<> ras;

            //ras.reset();
            RAS_RESET(pctx->gps);
            path_adaptor.init((agg::int8u*)pctx->lpdata->data, 
                    pctx->lpdata->data_size, pctx->x, pctx->y, 1.0);

            agg::conv_transform<PATH_ADAPTOR, agg::trans_affine>
                trans (path_adaptor, mot);

            //ras.add_path (trans);
            RAS_ADD_PATH(pctx->gps, trans);

            AGG_RENDER_CLIP(pctx->gps, 
                    (pctx->gps->text_hints == MP_TEXT_RENDER_HINT_ANTIALIAS_ON),
                    ras, sl, ren_comp, color);
        }

        default:
        {
            _MG_PRINTF ("mGPlus>API: not handled glyph type: %x.\n", pctx->lpdata->data_type);
        }
    }
}
#endif
