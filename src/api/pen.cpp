 /*
 ** $Id: pen.cpp 10885 2008-12-05 12:01:39Z dengkexi$
 **
 ** pen.cpp: Implementation of create and delete pen.
 **
 ** Copyright (C) 2003 ~ 2008 Feynman Software.
 ** Copyright (C) 2000 ~ 2002 Wei Yongming.
 **
 ** Create date: 2008/12/02
 */
#include <string.h>

#include "pen.h"

HPEN MGPlusPenCreate (int width, ARGB rgba)
{
    MPPen* pen = new MPPen;

    if (!pen)
        return MP_INV_HANDLE;

    memset ((void *)pen, 0x00, sizeof (MPPen));

    if (width < 1)
        width = 1;

    pen->width = width;
    pen->rgba = rgba;

    /* set default line_join and line_cap.*/
    pen->line_join_e = JOIN_MITER;
    pen->line_cap_e  = CAP_BUTT;

    return (HPEN)pen;
}

MPStatus MGPlusPenSetColor (HPEN pen, ARGB rgba)
{
    MPPen *ppen= (MPPen *)pen;
    if (!ppen)
        return MP_GENERIC_ERROR;
    ppen->rgba = rgba;
    return MP_OK;
}

MPStatus MGPlusPenSetJoinStyle (HPEN pen, LINE_JOIN_E line_join)
{
    MPPen *ppen= (MPPen *)pen;
    if (!ppen)
        return MP_GENERIC_ERROR;

    ppen->line_join_e = line_join;

    return MP_OK;
}

MPStatus MGPlusPenSetCapStyle (HPEN pen, LINE_CAP_E line_cap)
{
    MPPen *ppen= (MPPen *)pen;
    if (!ppen)
        return MP_GENERIC_ERROR;

    ppen->line_cap_e  = line_cap;

    return MP_OK;
}

MPStatus MGPlusPenSetDashes (HPEN pen, int dash_phase, 
        const unsigned char* dash_list, int dash_len)
{
    int i = 0;
    MPPen *ppen= (MPPen *)pen;

    if (!ppen || !dash_list || dash_len < 0)
        return MP_GENERIC_ERROR;

    ppen->dash = new unsigned char[dash_len];

    for (i = 0; i < dash_len; i++) {
        ppen->dash[i] = dash_list[i];
    }

    ppen->num_dashes  = dash_len;
    ppen->dash_phase  = dash_phase;

    return MP_OK;
}

MPStatus MGPlusPenSetWidth (HPEN pen, int width)
{
    MPPen *ppen= (MPPen *)pen;

    if (!ppen)
        return MP_GENERIC_ERROR;

    if (width < 1) width = 1;

    ppen->width = width;

    return MP_OK;
}

MPStatus MGPlusPenDelete (HPEN pen)
{
    MPPen *ppen=(MPPen *)pen;

    if (!ppen)
        return MP_GENERIC_ERROR;

    if (ppen->dash) {
        delete [] ppen->dash;
    }

    delete ppen;

    return MP_OK;
}
