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
 ** This file includes all brush type struct define. 
 **
 ** Create date: 2008/12/02
 */
#ifndef MGPLUS_BRUSH_H
#define MGPLUS_BRUSH_H

#include "agg_path_storage.h"
#include "agg_basics.h"
#include "agg_curves.h"
#include "agg_conv_bspline.h"
#include "agg_conv_stroke.h"

#include "mgplus.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief
 */
typedef enum _MPWrapMode {
    MP_WRAP_MODE_TILE,                  // just fill the image as normal
    MP_WRAP_MODE_TILE_FLIP_X,    // fill the image on the horizontal at the next col
    MP_WRAP_MODE_TILE_FLIP_Y,    // fill the image on the vertical at the next row
    MP_WRAP_MODE_TILE_FLIP_XY,  // fill the image on the horizontal and vertical step by step
    MP_WRAP_MODE_TILE_FLIP =  MP_WRAP_MODE_TILE_FLIP_XY,
    MP_WRAP_MODE_CLAMP              // fill the image on the center
} MPWrapMode;

/**
 * \brief
 */
typedef struct _SolidBrush {
    /* single brush color.*/
    ARGB rgba;
} SolidBrush;

/**
 * \brief
 */
typedef struct _HatchBrush {
    /* background single brush color.*/
    ARGB back_rgba;            
    /* fore single brush color.*/
    ARGB fore_rgba;             
    /* HatchBrush style.*/
    //MPHatchStyle  hatch_style;   
} HatchBrush;

/**
 * \brief
 */
typedef struct _TextureBrush {
    BITMAP *image;
    MPWrapMode wrap_mode;
} TextureBrush;

/**
 * \brief
 */
typedef struct _PathGradientBrush
{
    /* center_rgba from the point.*/
    MPPOINT  center_point;       
    /* center color.*/
    ARGB     center_rgba;        
    RECT     rect;
    /* surround  color.*/
    ARGB*    surround_rgba;      
    unsigned int      surround_rgba_num;
} PathGradientBrush;

/**
 * \brief
 */
typedef struct LinearGradientNode {
    ARGB color;
    float f_pos;
    struct LinearGradientNode* p_next; 
} _LinearGradientNode;

/**
 * \brief
 */
typedef struct LinearGradientBrush {
    /* gradient type.*/
    MPLinearGradientMode    gradient_type;  
    /* gradient rect.*/
    RECT    gradient_rect;                  
    /* start color.*/
    //ARGB*   gradient_colors;                
    //int     gradient_color_num;
    LinearGradientNode* gradient_add;
    //float*  gradient_color_pos;
    int     gradient_add_num;
    //int     flag;
} _LinearGradientBrush;

/**
 * \brief
 */
typedef struct _MPBrush {
    MPBrushType    brush_type;
    /* the point to other struct.*/
    void           *p_brush;    
} MPBrush;

#ifdef __cplusplus
}
#endif
#endif
