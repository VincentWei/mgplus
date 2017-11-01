/*
 ** $Id: opt_rgb24.cpp 10885 2008-12-05 12:01:39Z tangjianbin$
 **
 ** opt_rgb24.cpp: Implementation of rgb24 draw and fill path. 
 **
 ** Copyright (C) 2003 ~ 2008 Feynman Software.
 ** Copyright (C) 2000 ~ 2002 Wei Yongming.
 **
 ** Create date: 2009/03/22
 */
#include "mgplus.h"

#ifdef _MGPLUS_PIXFMT_RGB24

#include "agg_pixfmt_rgb.h"
#include "agg_pixfmt_rgba.h"
#include "agg_renderer_base.h"
#include "agg_conv_stroke.h"
#include "agg_scanline_u.h"
#include "agg_trans_bilinear.h"
#include "agg_ellipse.h"
#include "agg_gamma_lut.h"
#include "agg_gradient_lut.h"
#include "agg_span_gradient.h"
#include "agg_gamma_ctrl.h"

#include "pen.h"
#include "path.h"
#include "graphics.h"
#include <math.h>

#undef COMP_DRAW

typedef agg::pixfmt_rgb24               PIXFMT;
typedef agg::pixfmt_argb32              IMG_PIXFMT;
typedef agg::pixfmt_rgb24_pre           PIXFMT_PRE;
typedef agg::order_rgb                  ORDER;
typedef agg::int32u                     PIXEL_TYPE;

typedef agg::span_interpolator_linear<agg::trans_bilinear>      BILINEAR_INTERPOLATOR;
typedef agg::span_interpolator_linear<agg::trans_affine>        AFFINE_INTERPOLATOR;
typedef agg::span_interpolator_linear<agg::trans_perspective>   PERSPECTIVE_INTERPOLATOR;

typedef agg::image_accessor_clone<PIXFMT> IMG_ACCESSOR_TYPE;
typedef agg::span_image_filter_rgb_2x2<IMG_ACCESSOR_TYPE, 
                                        BILINEAR_INTERPOLATOR> SPAN_GEN_BILINEAR;
typedef agg::span_image_filter_rgb_2x2<IMG_ACCESSOR_TYPE, 
                                        AFFINE_INTERPOLATOR> SPAN_GEN_AFFINE;
typedef agg::span_image_filter_rgb_2x2<IMG_ACCESSOR_TYPE, 
                                        PERSPECTIVE_INTERPOLATOR> SPAN_GEN_PERSPECTIVE;

#include "opt_basic.cpp"

agg_draw_op draw_rgb24 = 
{
    copy,
    clear,
    draw_path,
    fill_path,
    draw_image_with_pts,
    draw_image_with_path,
#ifdef _MGPLUS_FONT_FT2
    draw_glyph
#endif
};

#endif
