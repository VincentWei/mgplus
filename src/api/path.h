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
 ** This file includes MPPath and MPFont struct define. 
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
