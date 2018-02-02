/*    
** new listview control program.
** Copyright (C) 2002~2009  FMSoft
** Current maintainer: Tangjianbin.

** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.

** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.

** You should have received a copy of the GNU General Public License along
** with this program; if not, write to the Free Software Foundation, Inc.,
** 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#ifndef NEW_LISTVIEW_H
#define NEW_LISTVIEW_H

#define NEW_LISTVIEW    "newlistview"

#define NLS_2STATE      0x00000001L

#define NLN_ENTER           1
#define NLN_LOSTFOCUS       2
#define NLN_GETFOCUS        3
#define NLN_SELECTCHANGE    4
#define NLN_FOCUSIDXCHANG   5

#define NLMSG_GETSELECTED   MSG_USER+1
#define NLMSG_GETFOCUSIDX   MSG_USER+2

extern BOOL  RegisterNewlistview(void);
extern void  UnregisterNewlistview(void);

BOOL InitNewlistview(HWND hWnd, int w, int h, int interval, int frame_num, int item_num, 
        char (*txt)[], char *focus, char *select);

BOOL ResetListviewTxt(HWND hwnd, int item_num, char (*txt)[]);

#endif
