/*
 ** $Id: brush.h 10885 2008-12-05 12:01:39Z tangjianbin$
 **
 ** This file includes all brush type struct define. 
 **
 ** Copyright (C) 2003 ~ 2008 Feynman Software.
 ** Copyright (C) 2000 ~ 2002 Wei Yongming.
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

typedef enum _MPWrapMode
{
    MP_WRAP_MODE_TILE,                  // just fill the image as normal
    MP_WRAP_MODE_TILE_FLIP_X,    // fill the image on the horizontal at the next col
    MP_WRAP_MODE_TILE_FLIP_Y,    // fill the image on the vertical at the next row
    MP_WRAP_MODE_TILE_FLIP_XY,  // fill the image on the horizontal and vertical step by step
    MP_WRAP_MODE_TILE_FLIP =  MP_WRAP_MODE_TILE_FLIP_XY,
    MP_WRAP_MODE_CLAMP              // fill the image on the center
}MPWrapMode;

struct SolidBrush
{
    /* single brush color.*/
    ARGB rgba;
};

struct HatchBrush
{
    /* background single brush color.*/
    ARGB back_rgba;            
    /* fore single brush color.*/
    ARGB fore_rgba;             
    /* HatchBrush style.*/
    //MPHatchStyle  hatch_style;   
};

struct TextureBrush
{
    BITMAP* image;
    MPWrapMode wrap_mode;
};

struct PathGradientBrush
{
    /* center_rgba from the point.*/
    MPPOINT  center_point;       
    /* center color.*/
    ARGB     center_rgba;        
    RECT     rect;
    /* surround  color.*/
    ARGB*   surround_rgba;      
    unsigned int      surround_rgba_num;
};

struct LinearGradientNode
{
    ARGB color;
    float f_pos;
    struct LinearGradientNode* p_next; 
};

struct LinearGradientBrush
{
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
};

struct MPBrush
{
    MPBrushType    brush_type;
    /* the point to other struct.*/
    void* p_brush;    
};

#ifdef __cplusplus
}
#endif
#endif
