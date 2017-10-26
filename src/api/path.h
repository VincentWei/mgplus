/*
 ** $Id: path.h 10885 2008-12-05 12:01:39Z tangjianbin$
 **
 ** This file includes MPPath and MPFont struct define. 
 **
 ** Copyright (C) 2003 ~ 2008 Feynman Software.
 ** Copyright (C) 2000 ~ 2002 Wei Yongming.
 **
 ** Create date: 2008/12/02
 */
#ifndef MGPLUS_PATH_H
#define MGPLUS_PATH_H

#include "mgplus.h"

#include "agg_path_storage.h"
#include "agg_basics.h"
#include "agg_curves.h"
#include "agg_arc.h"
#include "agg_conv_bspline.h"
#include "agg_conv_stroke.h"
#include "agg_path_storage_integer.h"
#include "agg_rounded_rect.h"
#include "agg_conv_curve.h"
#ifdef _MGPLUS_FONT_FT2
#include "agg_font_freetype.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_PATHID 128

typedef struct _MPPath {
    MPFillMode          m_fill_mode;            // fill mode
    agg::path_storage   m_agg_ps;               // AGG store path
    unsigned            path_id [MAX_PATHID];
    int                 id;
    agg::trans_affine   matrix;
} MPPath;

#ifdef _MGPLUS_FONT_FT2
typedef agg::font_engine_freetype_int32 font_engine_type;
typedef agg::font_cache_manager<font_engine_type> font_manager_type;

typedef struct _MPFont
{
    font_engine_type*  m_feng;
    font_manager_type* m_fman;
    char*              fontname;
} MPFont;
#endif

BOOL is_path_closed (MPPath* path);

#ifdef __cplusplus
}
#endif

#endif
