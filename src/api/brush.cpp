/*
 ** $Id: brush.cpp 10885 2008-12-05 12:01:39Z dengkexi$
 **
 ** brush.cpp: Implementation of create and delete brush, set
 ** and get brush type and content.
 **
 ** Copyright (C) 2003 ~ 2008 Feynman Software.
 ** Copyright (C) 2000 ~ 2002 Wei Yongming.
 **
 ** Create date: 2008/12/02
 */
#include <string.h>
#include <assert.h>
#include "brush.h"

#define SAFE_CHECK_PARAMETER(pointer)  \
    if(pointer==NULL) return MP_INVALID_PARAMETER;


HBRUSH MGPlusBrushCreate (MPBrushType type)
{
    MPBrush *brush = new MPBrush;

    if (!brush)
        return MP_INV_HANDLE;

    /* init */
    brush->brush_type = type;
    brush->p_brush = NULL;

    switch (type) {
        case MP_BRUSH_TYPE_SOLIDCOLOR:
            brush->p_brush = new SolidBrush;
            if (brush->p_brush) {
                memset ((void *)brush->p_brush, 0x00, sizeof(SolidBrush));
            }
            break;
        case MP_BRUSH_TYPE_HATCHFILL:    
            brush->p_brush = new HatchBrush;
            if (brush->p_brush) {
                memset ((void *)brush->p_brush, 0x00, sizeof(HatchBrush));
            }
            break;
        case MP_BRUSH_TYPE_TEXTUREFILL:    
            brush->p_brush = new TextureBrush;
            if (brush->p_brush) {
                memset ((void *)brush->p_brush, 0x00, sizeof(TextureBrush));
            }
            break;
        case MP_BRUSH_TYPE_PATHGRADIENT:    
            {
                PathGradientBrush* p_path_gradient_brush;
                p_path_gradient_brush = new PathGradientBrush;
                if (p_path_gradient_brush) {
                    memset ((void *)p_path_gradient_brush, 0x00, sizeof(PathGradientBrush));
                    p_path_gradient_brush->surround_rgba = NULL;
                    p_path_gradient_brush->surround_rgba_num = 0;
                    brush->p_brush=(void*)p_path_gradient_brush;
                    p_path_gradient_brush->rect.left = 0;
                    p_path_gradient_brush->rect.right = 0;
                    p_path_gradient_brush->rect.top = 0;
                    p_path_gradient_brush->rect.bottom = 0;
                }
            }
            break;
        case MP_BRUSH_TYPE_LINEARGRADIENT:    
            {
                LinearGradientBrush* p_linear_gradient_brush;

                brush->p_brush = new LinearGradientBrush;
                if (brush->p_brush) {
                    memset ((void *)brush->p_brush, 0x00, sizeof (LinearGradientBrush));
                    p_linear_gradient_brush = (LinearGradientBrush*)brush->p_brush;

                    //p_linear_gradient_brush->gradient_colors = NULL;
                    //p_linear_gradient_brush->gradient_color_num = 0;
                    p_linear_gradient_brush->gradient_add = NULL;
                    p_linear_gradient_brush->gradient_add_num = 0;
                }
            }
            break;
    }

    if (brush->p_brush)
        return (HBRUSH) brush;

    delete brush;
    return MP_INV_HANDLE;
}

MPStatus MGPlusBrushDelete (HBRUSH brush)
{
    MPBrush* m_brush = (MPBrush*) brush;
    SAFE_CHECK_PARAMETER (m_brush);
    SAFE_CHECK_PARAMETER (m_brush->p_brush);

    if (m_brush->brush_type == MP_BRUSH_TYPE_PATHGRADIENT)
    {
        PathGradientBrush* p_path_gradient_brush = 
                        (PathGradientBrush*) m_brush->p_brush;

        if (p_path_gradient_brush->surround_rgba_num)
        {
            delete[] p_path_gradient_brush->surround_rgba;
            p_path_gradient_brush->surround_rgba = NULL;
        }
        delete (PathGradientBrush *)m_brush->p_brush;
        m_brush->p_brush = NULL;
    }
    else if (m_brush->brush_type == MP_BRUSH_TYPE_LINEARGRADIENT) {
        LinearGradientBrush* p_linear_gradient_brush = 
            (LinearGradientBrush*) m_brush->p_brush;

        if (p_linear_gradient_brush->gradient_add_num) {
            LinearGradientNode* p_node = p_linear_gradient_brush->gradient_add;
            LinearGradientNode* p_next = p_node->p_next;

            while (p_node) {
                delete p_node;
                p_node = p_next;

                if (p_node)
                    p_next = p_next->p_next;
            }

            p_linear_gradient_brush->gradient_add = NULL;
            p_linear_gradient_brush->gradient_add_num = 0;
        }

        delete (LinearGradientBrush *)m_brush->p_brush;
        m_brush->p_brush = NULL;
    }

    if (m_brush->p_brush != NULL) {
        if (m_brush->brush_type == MP_BRUSH_TYPE_SOLIDCOLOR) {
            delete (SolidBrush *)m_brush->p_brush;
        }
        else if (m_brush->brush_type == MP_BRUSH_TYPE_HATCHFILL) {
            delete (HatchBrush *)m_brush->p_brush;
        }
        else if (m_brush->brush_type == MP_BRUSH_TYPE_TEXTUREFILL) {  
            delete (TextureBrush *)m_brush->p_brush;
        }
        else {
            assert(0);
        }
        m_brush->p_brush = NULL;
    }

    delete m_brush;
    return MP_OK;
}

MPStatus MGPlusSetSolidBrushColor (HBRUSH brush, ARGB rgba)
{
    MPBrush* m_brush = (MPBrush*) brush;

    SAFE_CHECK_PARAMETER (m_brush);
    SAFE_CHECK_PARAMETER (m_brush->p_brush);

    SolidBrush* p_solidbrush = (SolidBrush*) m_brush->p_brush;
    p_solidbrush->rgba = rgba;

    return MP_OK;
}

MPStatus MGPlusGetSolidBrushColor (HBRUSH brush, ARGB* rgba)
{
    MPBrush* m_brush = (MPBrush*) brush;

    SAFE_CHECK_PARAMETER (m_brush);
    SAFE_CHECK_PARAMETER (m_brush->p_brush);

    SolidBrush* p_solidbrush = (SolidBrush*) m_brush->p_brush;
    *rgba = p_solidbrush->rgba;

    return MP_OK;
}

MPStatus MGPlusSetTextureBrushImage (HBRUSH brush, BITMAP* bitmap)
{
    MPBrush* m_brush = (MPBrush*) brush;

    SAFE_CHECK_PARAMETER (m_brush);
    SAFE_CHECK_PARAMETER (m_brush->p_brush);
    SAFE_CHECK_PARAMETER (bitmap);

    TextureBrush* p_texture_brush = (TextureBrush*) m_brush->p_brush;
    p_texture_brush->image = bitmap;

    return MP_OK;
}

#if 0
MPStatus MGPlusSetTextureBrushWrapMode (HBRUSH brush, MPWrapMode wrapmode)
{
    MPBrush* m_brush = (MPBrush*) brush;

    SAFE_CHECK_PARAMETER (m_brush);
    SAFE_CHECK_PARAMETER (m_brush->p_brush);

    TextureBrush* p_texture_brush = (TextureBrush*) m_brush->p_brush;
    p_texture_brush->wrap_mode = wrapmode;

    return MP_OK;
}

MPStatus MGPlusGetTextureBrushWrapMode (HBRUSH brush, MPWrapMode* wrapmode)
{
    MPBrush* m_brush = (MPBrush*) brush;
    SAFE_CHECK_PARAMETER (m_brush);
    SAFE_CHECK_PARAMETER (m_brush->p_brush);
    SAFE_CHECK_PARAMETER (wrapmode);

    TextureBrush* p_texture_brush = (TextureBrush*) m_brush->p_brush;
    (*wrapmode) = p_texture_brush->wrap_mode;

    return MP_OK;
}
#endif

MPStatus MGPlusSetHatchBrushColor (HBRUSH brush, ARGB fore_rgba, ARGB back_rgba)
{
    MPBrush* m_brush = (MPBrush*) brush;

    SAFE_CHECK_PARAMETER (m_brush);
    SAFE_CHECK_PARAMETER (m_brush->p_brush);

    HatchBrush* p_hatch_brush = (HatchBrush*) m_brush->p_brush;

    p_hatch_brush->fore_rgba = fore_rgba;
    p_hatch_brush->back_rgba = back_rgba;

    return MP_OK;
}

MPStatus MGPlusGetHatchBrushColor (HBRUSH brush, 
        ARGB* fore_rgba, ARGB* back_rgba)
{    
    MPBrush* m_brush = (MPBrush*) brush;

    SAFE_CHECK_PARAMETER (m_brush);
    SAFE_CHECK_PARAMETER (m_brush->p_brush);
    SAFE_CHECK_PARAMETER (fore_rgba);
    SAFE_CHECK_PARAMETER (back_rgba);

    HatchBrush* p_hatch_brush = (HatchBrush*) m_brush->p_brush;

    (*fore_rgba) = p_hatch_brush->fore_rgba;
    (*back_rgba) = p_hatch_brush->back_rgba;

    return MP_OK;
}

#if 0
MPStatus MGPlusSetHatchBrushStyle (HBRUSH brush, MPHatchStyle type)
{
    MPBrush* m_brush = (MPBrush*) brush;

    SAFE_CHECK_PARAMETER (m_brush);
    SAFE_CHECK_PARAMETER (m_brush->p_brush);

    HatchBrush* p_hatch_brush = (HatchBrush*) m_brush->p_brush;
    p_hatch_brush->hatch_style = type;

    return MP_OK;
}

MPStatus MGPlusGetHatchBrushStyle (HBRUSH brush, MPHatchStyle* type)
{
    MPBrush* m_brush = (MPBrush*) brush;

    SAFE_CHECK_PARAMETER (m_brush);
    SAFE_CHECK_PARAMETER (m_brush->p_brush);
    SAFE_CHECK_PARAMETER (type);

    HatchBrush* p_hatch_brush = (HatchBrush*) m_brush->p_brush;
    (*type) = p_hatch_brush->hatch_style;

    return MP_OK;
}
#endif

MPStatus 
MGPlusSetPathGradientBrushCenterPoint (HBRUSH brush, MPPOINT* point)
{
    MPBrush* m_brush = (MPBrush*) brush;

    SAFE_CHECK_PARAMETER (m_brush);
    SAFE_CHECK_PARAMETER (m_brush->p_brush);
    SAFE_CHECK_PARAMETER (point);

    PathGradientBrush* p_path_gradient_brush = 
                            (PathGradientBrush*) m_brush->p_brush;

    p_path_gradient_brush->center_point.x = point->x;
    p_path_gradient_brush->center_point.y = point->y;

    return MP_OK;
}


MPStatus 
MGPlusSetPathGradientBrushCenterColor (HBRUSH brush, ARGB rgba)
{
    MPBrush* m_brush = (MPBrush*) brush;

    SAFE_CHECK_PARAMETER (m_brush);
    SAFE_CHECK_PARAMETER (m_brush->p_brush);

    PathGradientBrush* p_path_gradient_brush = 
                            (PathGradientBrush*) m_brush->p_brush;

    p_path_gradient_brush->center_rgba = rgba;
    return MP_OK;
}

MPStatus 
MGPlusSetPathGradientBrushSurroundColors (HBRUSH brush, 
        ARGB* rgba, int count)
{
    MPBrush* m_brush = (MPBrush*) brush;
    ARGB *p_surround_rgba = NULL;
    int i;

    SAFE_CHECK_PARAMETER (m_brush);
    SAFE_CHECK_PARAMETER (rgba);
    SAFE_CHECK_PARAMETER (m_brush->p_brush);

    if (count == 0)
        return MP_GENERIC_ERROR;

    PathGradientBrush* p_path_gradient_brush = 
                        (PathGradientBrush*) m_brush->p_brush;

    if (p_path_gradient_brush->surround_rgba_num)
    {
        delete [] p_path_gradient_brush->surround_rgba;
    }

    p_surround_rgba = new ARGB[count];

    if (!p_surround_rgba)
        return MP_GENERIC_ERROR;

    memset ((void *)p_surround_rgba, 0x00, count);
    
    for (i = 0; i < count; i++) {
        p_surround_rgba [i] = rgba[i];
        p_surround_rgba [i] = rgba[i];
    }

    p_path_gradient_brush->surround_rgba = p_surround_rgba;
    p_path_gradient_brush->surround_rgba_num = count;

    return MP_OK;
}

MPStatus 
MGPlusSetPathGradientBrushSurroundRect (HBRUSH brush, RECT* rect)
{
    MPBrush* m_brush = (MPBrush*) brush;

    SAFE_CHECK_PARAMETER (m_brush);
    SAFE_CHECK_PARAMETER (m_brush->p_brush);
    SAFE_CHECK_PARAMETER (rect);

    if (rect->left == rect->right || rect->top == rect->bottom)
        return MP_GENERIC_ERROR;

    PathGradientBrush* p_path_gradient_brush = 
                            (PathGradientBrush*) m_brush->p_brush;

    if (rect->left < rect->right)
    {
        p_path_gradient_brush->rect.left = rect->right;
        p_path_gradient_brush->rect.right = rect->left;
    }
    else
    {
        p_path_gradient_brush->rect.left = rect->left;
        p_path_gradient_brush->rect.right = rect->right;
    }

    if (rect->top > rect->bottom)
    {
        p_path_gradient_brush->rect.top = rect->bottom;
        p_path_gradient_brush->rect.bottom = rect->top;
    }
    else
    {
        p_path_gradient_brush->rect.top = rect->top;
        p_path_gradient_brush->rect.bottom = rect->bottom;
    }

    return MP_OK;
}

MPStatus MGPlusSetLinearGradientBrushMode (HBRUSH brush, 
        MPLinearGradientMode mode)
{
    MPBrush* m_brush = (MPBrush*) brush;

    SAFE_CHECK_PARAMETER (m_brush);
    SAFE_CHECK_PARAMETER (m_brush->p_brush);

    LinearGradientBrush* p_gradient_brush = 
                        (LinearGradientBrush*) m_brush->p_brush;
    p_gradient_brush->gradient_type = mode;

    return MP_OK;
}

MPStatus MGPlusGetLinearGradientBrushMode (HBRUSH brush, 
        MPLinearGradientMode* mode)
{
    MPBrush* m_brush = (MPBrush*) brush;

    SAFE_CHECK_PARAMETER (m_brush);
    SAFE_CHECK_PARAMETER (m_brush->p_brush);
    SAFE_CHECK_PARAMETER (mode);

    LinearGradientBrush* p_gradient_brush = 
                            (LinearGradientBrush*) m_brush->p_brush;
    *mode = p_gradient_brush->gradient_type;

    return MP_OK;
}

MPStatus 
MGPlusSetLinearGradientBrushRect (HBRUSH brush, RECT* rect)
{
    MPBrush* m_brush = (MPBrush*) brush;

    SAFE_CHECK_PARAMETER (m_brush);
    SAFE_CHECK_PARAMETER (m_brush->p_brush);
    SAFE_CHECK_PARAMETER (rect);

    LinearGradientBrush* p_gradient_brush = 
                        (LinearGradientBrush*) m_brush->p_brush;

    p_gradient_brush->gradient_rect.left = rect->left;
    p_gradient_brush->gradient_rect.right = rect->right;
    p_gradient_brush->gradient_rect.top = rect->top;
    p_gradient_brush->gradient_rect.bottom = rect->bottom;

    return MP_OK;
}

MPStatus MGPlusLinearGradientBrushGetColors (HBRUSH brush, ARGB* color,
        float* position)
{
    MPBrush* m_brush = (MPBrush*) brush;
    LinearGradientNode* p_node;
    
    SAFE_CHECK_PARAMETER (m_brush);

    if (!color)
        return MP_GENERIC_ERROR;

    LinearGradientBrush* p_gradient_brush = 
                            (LinearGradientBrush*) m_brush->p_brush;

    p_node = p_gradient_brush->gradient_add;

    if (!p_node){
        return MP_GENERIC_ERROR;
    }
    else {
        int i = 0;
        while (p_node) {
            color[i] = p_node->color;
            position[i++] = p_node->f_pos;
            p_node = p_node->p_next; 
        }
    }

    return MP_OK;
}

int MGPlusLinearGradientBrushGetColorNumber (HBRUSH brush)
{
    MPBrush* m_brush = (MPBrush*) brush;

    if (m_brush) {
        LinearGradientBrush* p_gradient_brush = 
            (LinearGradientBrush*) m_brush->p_brush;
        return p_gradient_brush->gradient_add_num;
    }
    return 0;
}


MPStatus MGPlusLinearGradientBrushAddColor (HBRUSH brush, ARGB color, float position)
{
    MPBrush* m_brush = (MPBrush*) brush;
    LinearGradientNode* p_search_node;
    
    SAFE_CHECK_PARAMETER (m_brush);

    if (position < 0 || position > 1)
        return MP_GENERIC_ERROR;

    LinearGradientBrush* p_gradient_brush = 
                            (LinearGradientBrush*) m_brush->p_brush;

    LinearGradientNode* p_node = new LinearGradientNode;

    if (!p_node)
        return MP_GENERIC_ERROR;

    p_node->color = color;
    p_node->f_pos = position; 
    p_node->p_next = NULL;

    p_search_node = p_gradient_brush->gradient_add;

    if (!p_search_node){
        p_gradient_brush->gradient_add = p_node;
        p_gradient_brush->gradient_add_num ++;
    }
    else {
        LinearGradientNode* prev = NULL;
        while (p_search_node) {
            if (p_node->f_pos == p_search_node->f_pos) {
                p_search_node->color = p_node->color;
                delete p_node;
                break;
            }
            else if ((p_node->f_pos > p_search_node->f_pos) 
                    && (p_search_node->p_next
                    && (p_node->f_pos < (p_search_node->p_next)->f_pos)))
            {
                LinearGradientNode* p = p_search_node->p_next;
                p_search_node->p_next = p_node;
                p_node->p_next = p;
                p_gradient_brush->gradient_add_num ++;
                break;
            }
            else if (p_node->f_pos < p_search_node->f_pos) {
                if (prev){
                    prev->p_next = p_node;
                    p_node->p_next = p_search_node;
                }
                else {
                    prev = p_gradient_brush->gradient_add;
                    p_gradient_brush->gradient_add = p_node;
                    p_node->p_next = prev;
                }
                p_gradient_brush->gradient_add_num ++;
                break;
            }
            else if (!p_search_node->p_next) {
                p_search_node->p_next = p_node;
                p_gradient_brush->gradient_add_num ++;
                break;
            }
            prev = p_search_node;
            p_search_node = p_search_node->p_next; 
        }
    }

    return MP_OK;
}


MPStatus MGPlusSetLinearGradientBrushColorsEx (HBRUSH brush, 
        ARGB* colors, int count, float* position)
{
    MPBrush* m_brush = (MPBrush*) brush;
    int i;

    SAFE_CHECK_PARAMETER (m_brush);
    SAFE_CHECK_PARAMETER (m_brush->p_brush);
    SAFE_CHECK_PARAMETER (colors);

    if (count <= 0)
        return MP_GENERIC_ERROR;

    {
        LinearGradientBrush* p_linear_gradient_brush = 
                            (LinearGradientBrush*) m_brush->p_brush;

        if (p_linear_gradient_brush->gradient_add_num)
        {
            LinearGradientNode* p_node = p_linear_gradient_brush->gradient_add;
            LinearGradientNode* p_next = p_node->p_next;
            while (p_node) {
                delete p_node;
                p_node = p_next;
                if (p_node)
                    p_next = p_next->p_next;
            }
            p_linear_gradient_brush->gradient_add = NULL;
            p_linear_gradient_brush->gradient_add_num = 0;
        }
    }
    for (i = 0; i < count; i ++)
        MGPlusLinearGradientBrushAddColor (brush, colors[i], position[i]);

    return MP_OK;
}

MPStatus MGPlusSetLinearGradientBrushColors (HBRUSH brush,
        ARGB* colors, int count)
{
    float* p_color_pos = NULL;
    int i;
    MPBrush* m_brush = (MPBrush*) brush;

    SAFE_CHECK_PARAMETER (m_brush);
    SAFE_CHECK_PARAMETER (m_brush->p_brush);
    SAFE_CHECK_PARAMETER (colors);

    if (count <= 0)
        return MP_GENERIC_ERROR;

    p_color_pos = new float[count];

    if (!p_color_pos) {
        return MP_GENERIC_ERROR;
    }

    for (i = 0; i < count; i ++) {
        p_color_pos [i] = (float)((double)i / (double)(count - 1));
    }

    MGPlusSetLinearGradientBrushColorsEx (brush, colors, count, p_color_pos);

    delete [] p_color_pos;
    return MP_OK;
}
