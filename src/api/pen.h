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
 ** This file includes MPPen struct define. 
 **
 ** Create date: 2008/12/02
 */
#ifndef MGPLUS_PEN_H
#define MGPLUS_PEN_H

#include "mgplus.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _MPPen {
    /*draw color.*/
    ARGB           rgba;  
    /* the pen's width.*/
    int            width; 
    int            line_join_e;
    int            line_cap_e;
    unsigned char* dash;
    unsigned int   num_dashes;
    int            dash_phase;
    double         miter_limit;
} MPPen;

#ifdef __cplusplus
}
#endif
#endif


