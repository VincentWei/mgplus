/*    
** a widget of button .
** Copyright (C) 2002~2009  Feynman Software
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

#include <sys/time.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <math.h>

#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>
#include <minigui/control.h>

#include "newlistview.h"
#include "animate.h"

typedef struct _NEW_LISTVIEW_DATA
{
    HDC     wnd_dc;
    HDC     render_dc;
    int     idx_select;
    int     idx_focus;
    int     interval;
    int     frame_num;
    int     item_w;
    int     item_h;
    int     item_num;
    char    **item_txt;
    BITMAP  bmp_focus;
    BITMAP  bmp_select;
}NEW_LISTVIEW_DATA;
typedef NEW_LISTVIEW_DATA* NEW_LISTVIEW_DATA_P;


typedef struct _focus_move_animate 
{
    HWND hwnd;
    int sy;
    int stepy;
    int frame_num;
}FOCUSMOVEANIMATE;


BOOL InitNewlistview(HWND hWnd, int w, int h, int interval, int frame_num, int item_num, 
        char (*txt)[], char *focus, char *select)
{
    if (!txt || !focus || !select || !item_num)
        return FALSE;

    NEW_LISTVIEW_DATA_P data = (NEW_LISTVIEW_DATA_P)GetWindowAdditionalData(hWnd);
    if(!data)
        return FALSE;

    LoadBitmapFromFile(data->wnd_dc, &(data->bmp_focus), focus);
    LoadBitmapFromFile(data->wnd_dc, &(data->bmp_select), select);

    if (!w)
        data->item_w = data->bmp_focus.bmWidth;
    else
        data->item_w = w;
    if (!h)
        data->item_h = data->bmp_focus.bmHeight;
    else
        data->item_h = h;

    data->interval = interval;
    data->frame_num = frame_num;
    data->item_num = item_num;
    data->item_txt = txt;

    return TRUE;
}

BOOL ResetListviewTxt(HWND hwnd, int item_num, char (*txt)[])
{
    NEW_LISTVIEW_DATA_P data = 
        (NEW_LISTVIEW_DATA_P)GetWindowAdditionalData(hwnd);
    if(!data)
        return FALSE;

    data->item_num = item_num;
    data->item_txt = txt;
    InvalidateRect(hwnd, NULL, TRUE);
    return TRUE;
}

void InitFocusMoveAnimate(FOCUSMOVEANIMATE *info, 
        HWND hwnd, int sy, int dy, int frame_num)
{
    div_t tmp;

    if(!info)
        return;
    info->hwnd = hwnd;
    info->sy = sy;
    tmp = div(dy-sy, frame_num);
    info->stepy = tmp.quot;
    info->frame_num = frame_num;
}

void move_focus(int index, void *context)
{
    HDC hdc;
    RECT rc;
    FOCUSMOVEANIMATE *info = (FOCUSMOVEANIMATE *)context;
    NEW_LISTVIEW_DATA_P data = 
        (NEW_LISTVIEW_DATA_P)GetWindowAdditionalData(info->hwnd);

    if (info->stepy > 0) /*move downward*/
        SetRect(&rc, 
                0,
                info->sy+(index-1)*info->stepy,
                data->item_w, 
                info->sy+index*info->stepy+data->item_h);
    else
        SetRect(&rc, 
                0,
                info->sy+index*info->stepy,
                data->item_w, 
                info->sy+(index-1)*info->stepy+data->item_h);

    /*draw background and text*/
    BitBlt(data->wnd_dc, 
        rc.left, rc.top, RECTW(rc), RECTH(rc),
        data->render_dc, 
        rc.left, rc.top, 0);

    /*draw focus picture*/
    FillBoxWithBitmap(data->render_dc,
            0, 
            info->sy+index*info->stepy, 
            data->item_w, data->item_h, &data->bmp_focus);

    hdc = GetClientDC(info->hwnd);
    BitBlt(data->render_dc, 
        rc.left, rc.top, RECTW(rc), RECTH(rc),
        hdc, 
        rc.left, rc.top, 0);
    ReleaseDC(hdc);
}

int switchalpha[] =
{ 120, 180, 210, 220, 255};

typedef struct _swtich_info
{
    HWND hwnd;
    NEW_LISTVIEW_DATA_P data;
    RECT rc;
    BITMAP bmp;
}SWITCH_INFO;

void switch_status(int index, void *context)
{
    HDC hdc;
    //RECT rc;
    SWITCH_INFO *info = (SWITCH_INFO *)context;
    NEW_LISTVIEW_DATA_P data = 
        (NEW_LISTVIEW_DATA_P)info->data;

#if 0
    SetRect(&rc, 0, 
            data->idx_select*data->item_h, 
            data->item_w, 
            (data->idx_select+1)*data->item_h);

	SetMemDCAlpha(data->wnd_dc, MEMDC_FLAG_SRCALPHA, switchalpha[index-1]);
    hdc = GetClientDC(info->hwnd);
	BitBlt(data->wnd_dc, rc.left, rc.top, RECTW(rc), RECTH(rc), 
            hdc, rc.left, rc.top, 0);
    ReleaseDC(hdc);
#else
    info->bmp.bmAlpha = 255 - switchalpha[index -1];

	SetMemDCAlpha(data->wnd_dc, MEMDC_FLAG_SRCALPHA, switchalpha[index-1]);
    hdc = GetClientDC(info->hwnd);
	BitBlt(data->wnd_dc, info->rc.left, info->rc.top, 
            RECTW(info->rc), RECTH(info->rc), 
            hdc, info->rc.left, info->rc.top, 0);
    ReleaseDC(hdc);

#endif
}

void focus_disappear(int index, void *context)
{
    HDC hdc;
    SWITCH_INFO *info = (SWITCH_INFO *)context;
    NEW_LISTVIEW_DATA_P data = 
        (NEW_LISTVIEW_DATA_P)info->data;

    info->bmp.bmAlpha = 255 - switchalpha[index -1];
    BitBlt(data->wnd_dc, info->rc.left, info->rc.top,
            RECTW(info->rc), RECTH(info->rc), 
            data->render_dc, info->rc.left, info->rc.top, 0);
    FillBoxWithBitmap(data->render_dc, info->rc.left, info->rc.top,
            RECTW(info->rc), RECTH(info->rc), &info->bmp);

    hdc = GetClientDC(info->hwnd);
	BitBlt(data->render_dc, info->rc.left, info->rc.top, 
            RECTW(info->rc), RECTH(info->rc), 
            hdc, info->rc.left, info->rc.top, 0);
    ReleaseDC(hdc);
}

void InitSwitchAnimate(SWITCH_INFO *info, HWND hwnd, 
        NEW_LISTVIEW_DATA_P data, RECT *rc)
{
    HDC hdc;

    info->hwnd = hwnd;
    info->data = data;
    CopyRect(&info->rc, rc);
    hdc = GetClientDC(hwnd);
    GetBitmapFromDC(hdc, rc->left, rc->top,
            RECTWP(rc), RECTHP(rc), &info->bmp);
    ReleaseDC(hdc);
    info->bmp.bmType |= BMP_TYPE_ALPHACHANNEL;
}

void FreeSwitchAnimate(SWITCH_INFO *info)
{
    UnloadBitmap(&info->bmp);
}

void InvaleteListFocus(HWND hwnd, int old_focus)
{
    RECT rc;
    NEW_LISTVIEW_DATA_P data = 
        (NEW_LISTVIEW_DATA_P)GetWindowAdditionalData(hwnd);
    int menu_focus = data->idx_focus;

    SetBrushColor(data->wnd_dc, PIXEL_black);

    if (menu_focus != -1){
        SetRect(&rc, 0, 
                old_focus*data->item_h, 
                data->item_w,
                (old_focus+1)*data->item_h);

        FillBox(data->wnd_dc, rc.left, rc.top-1, RECTW(rc), RECTH(rc)+3);
        DrawText(data->wnd_dc, data->item_txt[old_focus], 
                    -1,
                    &rc, DT_SINGLELINE|DT_CENTER|DT_VCENTER);
    }
}

static int NewlistviewProc(HWND hwnd, int message, WPARAM wParam, LPARAM lParam)
{
    switch (message) 
    {
        case MSG_CREATE:
        {
            HDC hdc;
            NEW_LISTVIEW_DATA_P data = NULL;

            data = (NEW_LISTVIEW_DATA_P) calloc(1, sizeof(NEW_LISTVIEW_DATA));
            if (!data)
            {
                printf ("Create NEW_LISTVIEW error!\n");
                return 1;
            }
            data->idx_select = -1;
            data->idx_focus = -1;

            hdc = GetClientDC(hwnd);
            data->wnd_dc =CreateCompatibleDC(hdc);
            data->render_dc = CreateCompatibleDC(hdc);
            ReleaseDC(hdc);
            SetWindowAdditionalData(hwnd, (DWORD)data);
            break;
        }
        case MSG_CLOSE:
        {
            NEW_LISTVIEW_DATA_P data = 
                (NEW_LISTVIEW_DATA_P)GetWindowAdditionalData(hwnd);
            UnloadBitmap(&data->bmp_focus);
            UnloadBitmap(&data->bmp_select);
            ReleaseDC(data->wnd_dc);
            ReleaseDC(data->render_dc);
            free(data);
            data = NULL;
            break;
        }
        case MSG_PAINT:
        {
            int i;
            RECT rc;
            HDC hdc;
            //const CLIPRGN* inv_rgn = (const CLIPRGN*) lParam;

            NEW_LISTVIEW_DATA_P data = 
                (NEW_LISTVIEW_DATA_P)GetWindowAdditionalData(hwnd);

            SetBrushColor(data->wnd_dc, GetWindowBkColor(hwnd));
            GetClientRect(hwnd, &rc);
            FillBox(data->wnd_dc, rc.left, rc.top, 
                    RECTW(rc), RECTH(rc));

            SelectFont(data->wnd_dc, GetWindowFont(hwnd));
            SetTextColor(data->wnd_dc, PIXEL_lightwhite);
            SetBkMode(data->wnd_dc, BM_TRANSPARENT);

            if (GetWindowStyle(hwnd)&NLS_2STATE && 
                data->idx_select != -1)
            {
                FillBoxWithBitmap(data->wnd_dc, 0,
                                data->idx_select*data->item_h,
                                data->item_w, data->item_h, 
                                &data->bmp_select);
            }

            for(i=0;i<data->item_num;i++)
            {
                SetRect(&rc, 0, i*data->item_h, 
                        data->item_w,
                        (i+1)*data->item_h);

                DrawText(data->wnd_dc, data->item_txt[i], 
                        -1,
                        &rc, DT_SINGLELINE|DT_CENTER|DT_VCENTER);
            }

            if( data->idx_focus != -1)
            {
                FillBoxWithBitmap(data->wnd_dc, 0,
                                data->idx_focus*data->item_h,
                                data->item_w, data->item_h, 
                                &(data->bmp_focus));
            }
            else if(data->idx_select != -1 && 
                    !(GetWindowStyle(hwnd)&NLS_2STATE))
            {
                FillBoxWithBitmap(data->wnd_dc, 0,
                                data->idx_select*data->item_h,
                                data->item_w, data->item_h, 
                                &data->bmp_focus);
            }

            hdc = BeginPaint(hwnd);
            BitBlt(data->wnd_dc, 0, 0, 0, 0, hdc, 0, 0, 0);
            EndPaint(hwnd, hdc);
            return 0;
        }
        case MSG_KEYUP:
        {
            int old_focus;
            int sy, dy;
            FOCUSMOVEANIMATE info={0};
            int scancode = (int)wParam;
            NEW_LISTVIEW_DATA_P data = 
                    (NEW_LISTVIEW_DATA_P)GetWindowAdditionalData(hwnd);

            switch(scancode)
            {
                case SCANCODE_CURSORBLOCKUP:
                {
                    old_focus = data->idx_focus;
                    if (data->idx_focus <= 0)
                        break;
                    data->idx_focus--;
                    sy = old_focus*data->item_h;
                    dy = data->idx_focus*data->item_h;
                    InvaleteListFocus(hwnd, old_focus);

                    InitFocusMoveAnimate(&info, hwnd, sy, dy, data->frame_num);
                    RunAnimate(data->interval, data->frame_num, 
                            move_focus, (void *)&info);

                    NotifyParent(hwnd, GetDlgCtrlID(hwnd), NLN_FOCUSIDXCHANG);
                    break;
                }
                case SCANCODE_CURSORBLOCKDOWN:
                {
                    old_focus = data->idx_focus;
                    if (data->idx_focus == -1 ||
                        data->idx_focus >= data->item_num-1)
                        break;
                    data->idx_focus++;
                    sy = old_focus*data->item_h;
                    dy = data->idx_focus*data->item_h;
                    InvaleteListFocus(hwnd, old_focus);

                    InitFocusMoveAnimate(&info, hwnd, sy, dy, data->frame_num);
                    RunAnimate(data->interval, data->frame_num, 
                            move_focus, (void *)&info);

                    //FocusMoveAnimate(hwnd, data->interval, sy, dy, data->frame_num);
                    NotifyParent(hwnd, GetDlgCtrlID(hwnd), NLN_FOCUSIDXCHANG);
                    break;
                }
                case SCANCODE_ENTER:
                {
                    if(data->idx_focus != -1)
                    {
                        RECT rc;
                        SWITCH_INFO sinfo={0};

                        data->idx_select = data->idx_focus;
                        data->idx_focus = -1;

                        if (GetWindowStyle(hwnd)&NLS_2STATE)
                        {
                            FillBoxWithBitmap(data->wnd_dc, 0,
                                        data->idx_select*data->item_h,
                                        data->item_w, data->item_h, 
                                        &data->bmp_select);
                            SetRect(&rc, 0, data->idx_select*data->item_h,
                                    data->item_w, 
                                    (data->idx_select+1)*data->item_h);

                            DrawText(data->wnd_dc, data->item_txt[data->idx_select], 
                                    -1,
                                    &rc, DT_SINGLELINE|DT_CENTER|DT_VCENTER);

                            InitSwitchAnimate(&sinfo, hwnd, data, &rc);
                            RunAnimate(data->interval, sizeof(switchalpha)/sizeof(int *), 
                                    switch_status, (void *)&sinfo);
                            FreeSwitchAnimate(&sinfo);
                        }

                        NotifyParent(hwnd, GetDlgCtrlID(hwnd), NLN_ENTER);
                    }
                    break;
                }
                case SCANCODE_ESCAPE:
                {
                    if (data->idx_focus != -1)
                    {
                        RECT rc;
                        SWITCH_INFO sinfo={0};

                        if (GetWindowStyle(hwnd)&NLS_2STATE && 
                            data->idx_select != -1)
                        {
                            FillBoxWithBitmap(data->wnd_dc, 0,
                                        data->idx_focus*data->item_h,
                                        data->item_w, data->item_h, 
                                        &data->bmp_select);
                        }else{
                            FillBox(data->wnd_dc, 0,
                                        data->idx_focus*data->item_h,
                                        data->item_w, data->item_h);
                        }

                        SetRect(&rc, 0, data->idx_focus*data->item_h,
                                data->item_w, 
                                (data->idx_focus+1)*data->item_h);

                        DrawText(data->wnd_dc, data->item_txt[data->idx_focus], 
                                    -1,
                                    &rc, DT_SINGLELINE|DT_CENTER|DT_VCENTER);

#if 1
                        InitSwitchAnimate(&sinfo, hwnd, data, &rc);
                        RunAnimate(data->interval, sizeof(switchalpha)/sizeof(int *), 
                                focus_disappear, (void *)&sinfo);
                        FreeSwitchAnimate(&sinfo);
#else
                        HDC hdc = GetClientDC(hwnd);
                        BitBlt(data->wnd_dc, rc.left, rc.top, RECTW(rc),
                                RECTH(rc), hdc, rc.left, rc.top, 0);
                        ReleaseDC(hdc);
#endif
                        data->idx_focus = -1;
                        data->idx_select = -1;

                        //InvalidateRect(hwnd, NULL, FALSE);
                        NotifyParent(hwnd, GetDlgCtrlID(hwnd), NLN_LOSTFOCUS);
                    }
                    break;
                }
                default:
                    break;
            }
            break;
        }
        case MSG_SETFOCUS:
        {
            SWITCH_INFO info={0};
            RECT rc;
            NEW_LISTVIEW_DATA_P data = 
                (NEW_LISTVIEW_DATA_P)GetWindowAdditionalData(hwnd);

            if (data->idx_focus != -1)
                break;

            if (data->idx_select != -1){
                data->idx_focus = data->idx_select;
                data->idx_select = -1;
            }
            else
                data->idx_focus = 0;

            SetRect(&rc, 0, 
                    data->idx_focus*data->item_h, 
                    data->item_w,
                    (data->idx_focus+1)*data->item_h);

            //InvalidateRect(hwnd, &rc, FALSE);
            FillBox(data->wnd_dc, rc.left, rc.top,
                   RECTW(rc), RECTH(rc));

            DrawText(data->wnd_dc, data->item_txt[data->idx_focus], 
                    -1,
                    &rc, DT_SINGLELINE|DT_CENTER|DT_VCENTER);

            FillBoxWithBitmap(data->wnd_dc, 0,
                    data->idx_focus*data->item_h,
                    data->item_w, data->item_h, 
                    &data->bmp_focus);

            InitSwitchAnimate(&info, hwnd, data, &rc);
            RunAnimate(data->interval, sizeof(switchalpha)/sizeof(int *), 
                            switch_status, (void *)&info);
            FreeSwitchAnimate(&info);

            //NotifyParent(hwnd, GetDlgCtrlID(hwnd), NLN_GETFOCUS);
            break;
        }
        case MSG_KILLFOCUS:
        {
            //RECT rc;
            NEW_LISTVIEW_DATA_P data = 
                (NEW_LISTVIEW_DATA_P)GetWindowAdditionalData(hwnd);

            if (data->idx_focus == -1)
                break;
#if 0
            if (data->idx_select != -1)
            {
                SetRect(&rc, 0, 
                    data->idx_select*data->item_h, 
                    data->item_w,
                    (data->idx_select+1)*data->item_h);
                //data->idx_select = -1;
            }
            else if (data->idx_focus != -1)
            {
                SetRect(&rc, 0, 
                    data->idx_focus*data->item_h, 
                    data->item_w,
                    (data->idx_focus+1)*data->item_h);
                data->idx_focus = -1;
            }
#else
            data->idx_focus = -1;
#endif
            InvalidateRect(hwnd, NULL, FALSE);
            //NotifyParent(hwnd, GetDlgCtrlID(hwnd), NLN_LOSTFOCUS);
            break;
        }
        case NLMSG_GETSELECTED:
        {
            NEW_LISTVIEW_DATA_P data = 
                (NEW_LISTVIEW_DATA_P)GetWindowAdditionalData(hwnd);
            return data->idx_select;
        }
        case NLMSG_GETFOCUSIDX:
        {
            NEW_LISTVIEW_DATA_P data = 
                (NEW_LISTVIEW_DATA_P)GetWindowAdditionalData(hwnd);
            return data->idx_focus;
        }
        default:
            break;
    }

    return DefaultControlProc (hwnd, message, wParam, lParam);
}

BOOL RegisterNewlistview(void)
{
    WNDCLASS MyClass;

    MyClass.spClassName = NEW_LISTVIEW;
    MyClass.dwStyle     = WS_NONE;
    MyClass.dwExStyle   = WS_EX_NONE;
    MyClass.hCursor     = GetSystemCursor (IDC_ARROW);
    MyClass.iBkColor    = COLOR_black;
    MyClass.WinProc     = NewlistviewProc;
    return RegisterWindowClass (&MyClass);
}

void UnregisterNewlistview(void)
{
    UnregisterWindowClass (NEW_LISTVIEW);
}

