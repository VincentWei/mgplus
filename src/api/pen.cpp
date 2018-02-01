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
 ** pen.cpp: Implementation of create and delete pen.
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
