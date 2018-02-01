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
** lf_fashion.c: The fashion LF implementation file.
*/

#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <map>

#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>
#include <minigui/control.h>

#include "mgplus.h"
#ifndef _MGRM_THREADS
#include <pthread.h>
#endif

#ifndef _MGPLUS_LFRDR_FASHION
#define _MGPLUS_LFRDR_FASHION
#endif

#define TEST_FLAG 0
#if TEST_FLAG
#include <utilx/scope_timer.hpp>
#define FUNCTION_SCOPE_TIMING() FUNC_SCOPE_TIMING(450)
#else
#define FUNCTION_SCOPE_TIMING() 
#endif

#ifdef _MGPLUS_LFRDR_FASHION

#define BTN_STATUS_NUM      8
#define CAP_BTN_INTERVAL    2

#define V_DIFF              0.2
#define FULL_V              (itofix(1))
#define UNDEFINED_HUE       (itofix(-1))

#define CHECKNOT_RET_ERR(cond) \
        if (!(cond)) return -1

#define CHECK_RET_VAL(cond, val) \
        if (cond) return val

#define IS_BORDER_VISIBLE(win_info) \
        ((win_info)->dwStyle & WS_BORDER || \
        (win_info)->dwStyle & WS_THINFRAME || \
        (win_info)->dwStyle & WS_THICKFRAME) 

#define IS_CAPTION_VISIBLE(win_info) \
        ((win_info)->dwStyle & WS_CAPTION)

#define IS_MINIMIZEBTN_VISIBLE(win_info) \
        ((win_info)->dwStyle & WS_CAPTION && \
         (win_info)->dwStyle & WS_MINIMIZEBOX)

#define IS_MAXIMIZEBTN_VISIBLE(win_info) \
        ((win_info)->dwStyle & WS_CAPTION && \
         (win_info)->dwStyle & WS_MAXIMIZEBOX)

#define IS_CAPICON_VISIBLE(win_info) \
        ((win_info)->dwStyle & WS_CAPTION && (win_info)->hIcon)

#define IS_CLOSEBTN_VISIBLE(win_info) \
        ((win_info)->dwStyle & WS_CAPTION && \
        !((win_info)->dwExStyle & WS_EX_NOCLOSEBOX))

#define IS_MENUBAR_VISIBLE(win_info) ((win_info)->hMenu)

#define IS_VSCROLL_VISIBLE(win_info) \
        ((win_info)->dwStyle & WS_VSCROLL &&  \
         !((win_info)->vscroll.status & SBS_HIDE))

#define IS_HSCROLL_VISIBLE(win_info) \
        ((win_info)->dwStyle & WS_HSCROLL &&  \
         !((win_info)->hscroll.status & SBS_HIDE))

#define IS_LEFT_VSCOLLBAR(win_info) \
        ((win_info)->dwExStyle & WS_EX_LEFTSCROLLBAR)

#define DWINDLE_RECT(rc) \
    do \
    { \
        (rc).left++;      \
        (rc).top++;       \
        (rc).right--;     \
        (rc).bottom--;    \
    }while (0)

/* For trackbar control */
/* use these definition when drawing trackbar */
#define TB_BORDER               2

#define WIDTH_HORZ_SLIDER       24
#define HEIGHT_HORZ_SLIDER      12

#define WIDTH_VERT_SLIDER       12
#define HEIGHT_VERT_SLIDER      24

#define WIDTH_VERT_RULER        6
#define HEIGHT_HORZ_RULER       6

#define MPMakeARGB(r, g, b, a)    (((DWORD)((BYTE)(b))) | ((DWORD)((BYTE)(g)) << 8) \
               | ((DWORD)((BYTE)(r)) << 16) | ((DWORD)((BYTE)(a)) << 24))

/* please keep it even for good appearance */
#define LEN_TICK                4
#define GAP_TICK_SLIDER         6

extern WINDOW_ELEMENT_RENDERER wnd_rdr_fashion;

static int get_window_border(HWND hWnd, int dwStyle, int win_type);
static int calc_we_metrics(HWND hWnd, LFRDR_WINSTYLEINFO* style_info, int which);

static inline void linear_gradient_draw (HDC hdc, MPLinearGradientMode mode, 
        RECT *rc, gal_pixel *pixel, int pixel_num)
{
    FUNCTION_SCOPE_TIMING();
    HPATH path;
    HBRUSH brush;
    HGRAPHICS graphic = MGPlusGraphicCreate (RECTWP(rc), RECTHP (rc));

    if (!graphic)
        return;
    BitBlt (hdc, rc->left, rc->top, RECTWP(rc), RECTHP (rc),
            MGPlusGetGraphicDC (graphic), 0, 0, 0);

    brush = MGPlusBrushCreate (MP_BRUSH_TYPE_LINEARGRADIENT); 
    if (!brush){
        MGPlusGraphicDelete (graphic);
        return;
    }
    path = MGPlusPathCreate (MP_PATH_FILL_MODE_WINDING);
    if (!path) {
        MGPlusGraphicDelete (graphic);
        MGPlusBrushDelete (brush);
        return;
    }

    RECT rc_tmp;
    rc_tmp.left = rc_tmp.top = 0;
    rc_tmp.right = RECTWP (rc);
    rc_tmp.bottom = RECTHP (rc);

    MGPlusSetLinearGradientBrushMode (brush, mode);
    MGPlusSetLinearGradientBrushRect (brush, &rc_tmp);
    MGPlusSetLinearGradientBrushColors (brush, (ARGB*)pixel, pixel_num); 

    MGPlusPathAddRectangleI (path, 0, 0, RECTWP(rc), RECTHP(rc));
    MGPlusFillPath (graphic, brush, path); 

    MGPlusGraphicSave (graphic, hdc, 0, 0, 0, 0, 
                        rc->left, rc->top);

    MGPlusPathDelete (path);
    MGPlusBrushDelete (brush);
    MGPlusGraphicDelete (graphic);
}

static inline int lf_get_win_type (HWND hWnd)
{
    if(IsDialog(hWnd))
        return LFRDR_WINTYPE_DIALOG;
    else if(IsMainWindow(hWnd))
        return LFRDR_WINTYPE_MAINWIN;
    else if(IsControl(hWnd))
        return LFRDR_WINTYPE_CONTROL;

    return LFRDR_WINTYPE_UNKNOWN;
}

static inline int get_window_caption (HWND hWnd)
{
    return calc_we_metrics (hWnd, NULL, LFRDR_METRICS_CAPTION_H);
}

static inline int get_window_menubar (HWND hWnd)
{
    return calc_we_metrics (hWnd, NULL, LFRDR_METRICS_MENU_H);
}

static inline int get_window_scrollbar (HWND hWnd, BOOL is_vertival)
{
    if (is_vertival) {
        return calc_we_metrics (hWnd, NULL, LFRDR_METRICS_VSCROLL_W);
    }
    else {
        return calc_we_metrics (hWnd, NULL, LFRDR_METRICS_HSCROLL_H);
    }
}

static inline void erase_bkgnd (HWND hWnd, HDC hdc, const RECT *rect)
{
    FUNCTION_SCOPE_TIMING();
    gal_pixel old_color;
    if (hWnd != HWND_NULL)
        old_color = SetBrushColor (hdc, GetWindowBkColor (hWnd));
    else
        old_color = SetBrushColor (hdc, 
                GetWindowElementPixel (HWND_DESKTOP, WE_BGC_DESKTOP));

    FillBox (hdc, rect->left, rect->top, RECTWP(rect), RECTHP(rect));
    SetBrushColor (hdc, old_color);
}

/*initialize and terminate interface*/
static int init (PWERENDERER renderer) 
{
    FUNCTION_SCOPE_TIMING();
   /*get information from MiniGUI.cfg file*/
    InitWindowElementAttrs (renderer);
    renderer->refcount = 0;

    /*we_fonts*/
    renderer->we_fonts[0] = GetSystemFont (SYSLOGFONT_CAPTION);
    renderer->we_fonts[1] = GetSystemFont (SYSLOGFONT_MENU);
    renderer->we_fonts[2] = GetSystemFont (SYSLOGFONT_WCHAR_DEF);
    renderer->we_fonts[3] = GetSystemFont (SYSLOGFONT_WCHAR_DEF);

    /*icon*/
    if (!InitRendererSystemIcon (renderer->name, 
                renderer->we_icon[0], renderer->we_icon[1]))
        return -1;
    
    if (!RegisterSystemBitmap (HDC_SCREEN, renderer->name, SYSBMP_RADIOBUTTON))
        return -1;

    if (!RegisterSystemBitmap (HDC_SCREEN, renderer->name, SYSBMP_CHECKBUTTON))
        return -1;

    renderer->private_info = NULL;
    return 0;
}

static int deinit (PWERENDERER renderer)
{
    FUNCTION_SCOPE_TIMING();
    /* Destroy system icon. */
    TermRendererSystemIcon (renderer->we_icon[0], renderer->we_icon[1]);

    UnregisterSystemBitmap (HDC_SCREEN, renderer->name, SYSBMP_RADIOBUTTON);
    UnregisterSystemBitmap (HDC_SCREEN, renderer->name, SYSBMP_CHECKBUTTON);

    wnd_rdr_fashion.private_info = NULL;
    return 0;
}

/*
 * RGB2HSV:
 *      This function translate color in RGB space to in HSV space.
 * Author: XuguangWang
 * Date: 2007-11-22
 */
static void RGB2HSV (fixed r, fixed g, fixed b, fixed* h, fixed* s, fixed* v)
{
    FUNCTION_SCOPE_TIMING();
    fixed min; 
    fixed max; 
    fixed delta;
    fixed tmp;

    /*change r g b to [0, 1]*/
    r = fixdiv (r, itofix(255));
    g = fixdiv (g, itofix(255));
    b = fixdiv (b, itofix(255));

    tmp = MIN (r, g);
    min = MIN (tmp, b);
    tmp = MAX (r, g);
    max = MAX (tmp, b);

    *v = max; // v
    delta = max - min;

    if (max != 0)
        *s = fixdiv (delta, max);
    else {
        *s = 0;
        *h = UNDEFINED_HUE;
        return;
    }

    if (fixtof (delta) == 0)
    {
        *h = 0;
        *s = 0;
        return;
    }

    if (r == max)
        /*between yellow & magenta*/
        *h = fixdiv (fixsub (g, b), delta);
    else if( g == max )
        /*between cyan & yellow*/
        *h = fixadd (itofix (2), fixdiv (fixsub (b, r), delta));
    else 
        /*magenta & cyan*/
        *h = fixadd (itofix (4), fixdiv (fixsub (r, g), delta));

    /*degrees*/
    *h = fixmul (*h, itofix (60)); 

    if (*h < itofix (0))
        *h = fixadd (*h, itofix (360));
}

/*
 * HSV2RGB:
 *      This function translate color in HSV space to in RGB space.
 * Author: XuguangWang
 * Date: 2007-11-22
 */
static void HSV2RGB(fixed h, fixed s, fixed v, fixed* r, fixed* g, fixed* b)
{
    FUNCTION_SCOPE_TIMING();
    int i;
    fixed f, p, q, t;

    if (s == 0) {
        *r = fixmul (v, itofix (255));
        *g = fixmul (v, itofix (255));
        *b = fixmul (v, itofix (255));
        return;
    }

    /*sector 0 to 5*/
    h = fixdiv (h, itofix (60));
    i = (h >> 16) % 6;
    /*factorial part of h*/
    f = fixsub (h, itofix (i));
    p = fixmul (v, fixsub (itofix (1), s));
    /*q = v * (1 - s*f)*/
    q = fixmul (v, fixsub (itofix (1), fixmul (s, f)));
   
    /*t = v * (1 - s*(1-f))*/
    t = fixmul (v, fixsub (itofix (1), fixmul (s, fixsub (itofix(1), f))));
    
    switch (i) {
        case 0: 
            *r = fixmul (v, itofix (255)); 
            *g = fixmul (t, itofix (255)); 
            *b = fixmul (p, itofix (255));
            break;
        case 1:
            *r = fixmul (q, itofix (255));
            *g = fixmul (v, itofix (255));
            *b = fixmul (p, itofix (255));
            break;
        case 2:
            *r = fixmul (p, itofix (255));
            *g = fixmul (v, itofix (255));
            *b = fixmul (t, itofix (255));
            break;
        case 3:
            *r = fixmul (p, itofix (255));
            *g = fixmul (q, itofix (255));
            *b = fixmul (v, itofix (255));
            break;
        case 4:
            *r = fixmul (t, itofix (255));
            *g = fixmul (p, itofix (255));
            *b = fixmul (v, itofix (255));
            break;
        case 5:
            *r = fixmul (v, itofix (255));
            *g = fixmul (p, itofix (255));
            *b = fixmul (q, itofix (255));
            break;
    }
}
/*
 * gradient_color:
 *      calc a brighter or darker color           
 * color  : RGB color
 * flag   : LFRDR_3DBOX_COLOR_DARKER or LFRDR_3DBOX_COLOR_LIGHTER
 * degree : in [0, 255]
 * Author: zhounuohua
 * Date: 2008-06-04
 */
static DWORD gradient_color (DWORD color, int flag, int degree)
{
    FUNCTION_SCOPE_TIMING();
    fixed h;
    fixed s;
    fixed v;
    fixed r = itofix (GetRValue (color));
    fixed g = itofix (GetGValue (color));
    fixed b = itofix (GetBValue (color));
    UINT  a = GetAValue (color);
    /*RGB => HSV*/
    RGB2HSV (r, g, b, &h, &s, &v);

    /*chang V and S of HSV*/
    switch (flag) {
        case LFRDR_3DBOX_COLOR_DARKER:
            {
                v = fixsub (v, degree * v/255);
                if (v < 0)
                    v = 0;
            }
            break;

        case LFRDR_3DBOX_COLOR_LIGHTER:
            {
                if (v > FULL_V)
                    v = FULL_V;
                else
                    v = fixadd (v, degree * v/255);

                if (v > FULL_V) 
                    v = FULL_V;

                if (v == FULL_V) {
                    s = fixsub (s, degree * s/255);
                    if (s < 0)
                        s = 0;
                }
            }
            break;
        default:
            return color;
    }
    /*HSV => RGB*/
    HSV2RGB (h, s, v, &r, &g, &b);

    return MakeRGBA (fixtoi (r), fixtoi (g), fixtoi (b), a);
}

/*
 * gradient_color:
 *      calc a brighter or darker color           
 * color  : RGB color
 * flag   : LFRDR_3DBOX_COLOR_DARKER or LFRDR_3DBOX_COLOR_LIGHTER
 * degree : in [0, 255]
 * Author: zhounuohua
 * Date: 2008-06-04
 */
static DWORD mp_gradient_color (DWORD color, int flag, int degree)
{
    FUNCTION_SCOPE_TIMING();
    fixed h;
    fixed s;
    fixed v;
    fixed r = itofix (GetRValue (color));
    fixed g = itofix (GetGValue (color));
    fixed b = itofix (GetBValue (color));
    UINT  a = GetAValue (color);
    /*RGB => HSV*/
    RGB2HSV (r, g, b, &h, &s, &v);

    /*chang V and S of HSV*/
    switch (flag) {
        case LFRDR_3DBOX_COLOR_DARKER:
            {
                v = fixsub (v, degree * v/255);
                if (v < 0)
                    v = 0;
            }
            break;

        case LFRDR_3DBOX_COLOR_LIGHTER:
            {
                if (v > FULL_V)
                    v = FULL_V;
                else
                    v = fixadd (v, degree * v/255);

                if (v > FULL_V) 
                    v = FULL_V;

                if (v == FULL_V) {
                    s = fixsub (s, degree * s/255);
                    if (s < 0)
                        s = 0;
                }
            }
            break;
        default:
            return color;
    }
    /*HSV => RGB*/
    HSV2RGB (h, s, v, &r, &g, &b);

    //return MakeRGBA (fixtoi (r), fixtoi (g), fixtoi (b), a);
    return MPMakeARGB (fixtoi (r), fixtoi (g), fixtoi (b), a);
}

/*
 * calc_3dbox_color:
 *      calc a less brighter, much bright, less darker, or much darker           
 *      color of color which used in 3dbox.     
 * Author: XuguangWang
 * Date: 2007-11-22
 */
static DWORD calc_3dbox_color (DWORD color, int flag)
{
    int degree;
    switch (flag) {
        case LFRDR_3DBOX_COLOR_DARKER:
            degree = 10;
            break;
        case LFRDR_3DBOX_COLOR_DARKEST:
            flag = LFRDR_3DBOX_COLOR_DARKER;
            degree = 255;
            break;
        case LFRDR_3DBOX_COLOR_LIGHTER:
            degree = 10;
            break;
        case LFRDR_3DBOX_COLOR_LIGHTEST:
            flag = LFRDR_3DBOX_COLOR_LIGHTER;
            degree = 255;
            break;
        default:
            return 0;
    }
    return gradient_color (color, flag, degree);
}

static void 
draw_one_frame (HDC hdc, const RECT* rc, 
        DWORD lt_color, DWORD rb_color)
{
    FUNCTION_SCOPE_TIMING();
    SetPenColor (hdc, RGBA2Pixel (hdc, GetRValue (lt_color), 
                GetGValue (lt_color), GetBValue (lt_color), 
                GetAValue (lt_color)));

    MoveTo (hdc, rc->left, rc->bottom-1);
    LineTo (hdc, rc->left, rc->top);
    LineTo (hdc, rc->right-2, rc->top);

    SetPenColor (hdc, RGBA2Pixel (hdc, GetRValue (rb_color), 
                GetGValue (rb_color), GetBValue (rb_color), 
                GetAValue (rb_color)));

    MoveTo (hdc, rc->left+1, rc->bottom-1);
    LineTo (hdc, rc->right-1, rc->bottom-1);
    LineTo (hdc, rc->right-1, rc->top);
}

static int fill_iso_triangle (HDC hdc, DWORD color, 
                                POINT ap, POINT bp1, POINT bp2)
{ 
    FUNCTION_SCOPE_TIMING();
    int x1, x2, y1, y2;
    int xdelta, ydelta;
    int xinc, yinc;
    int rem;
    gal_pixel old_color;

    if(bp1.y != bp2.y && bp1.x != bp2.x) return -1;        

    x1 = ap.x; 
    y1 = ap.y; 
    x2 = bp1.x;
    y2 = bp1.y; 

    xdelta = x2 - x1;
    ydelta = y2 - y1;
    if (xdelta < 0) xdelta = -xdelta;
    if (ydelta < 0) ydelta = -ydelta;

    xinc = (x2 > x1) ? 1 : -1;
    yinc = (y2 > y1) ? 1 : -1;

    SetPixel (hdc, x1, y1, RGBA2Pixel (hdc, GetRValue (color),
                GetGValue (color), GetBValue (color), GetAValue (color)));
    old_color = SetPenColor (hdc, RGBA2Pixel (hdc, GetRValue (color),
                GetGValue (color), GetBValue (color), GetAValue (color)));

    if (xdelta >= ydelta) 
    {
        rem = xdelta >> 1;
        while (x1 != x2) 
        {
            x1 += xinc;
            rem += ydelta;
            if (rem >= xdelta) 
            {
                rem -= xdelta;
                y1 += yinc;
            }
            MoveTo (hdc, x1, y1);
            if(bp1.y == bp2.y)
                LineTo (hdc, x1 + (ap.x - x1)*2, y1);
            else
                LineTo (hdc, x1, y1 + (ap.y -y1)*2);
        }
    } 
    else 
    {
        rem = ydelta >> 1;
        while (y1 != y2) 
        {
            y1 += yinc;
            rem += xdelta;
            if (rem >= ydelta) 
            {
                rem -= ydelta;
                x1 += xinc;
            }
            MoveTo (hdc, x1, y1);
            if (bp1.y == bp2.y)
                LineTo (hdc, x1 + (ap.x - x1)*2, y1);
            else
                LineTo (hdc, x1, y1 + (ap.y -y1)*2);
        }
    }
    
    SetPenColor (hdc, old_color);
    
    return 0;
}

/*
 * draw_a frame for fashion button.
 */
static void draw_fashion_frame(HDC hdc, RECT rect, DWORD color)
{
    FUNCTION_SCOPE_TIMING();
    gal_pixel old_pen_color;
    
    rect.right--;
    rect.bottom--;
    
    old_pen_color = SetPenColor (hdc, 
            RGBA2Pixel (hdc, GetRValue (color), GetGValue (color), 
                GetBValue (color), GetAValue (color)));
    
    MoveTo (hdc, rect.left+1, rect.top);
    LineTo (hdc, rect.right-1, rect.top);
    MoveTo (hdc, rect.left, rect.top+1);
    LineTo (hdc, rect.left, rect.bottom-1);
    
    MoveTo (hdc, rect.left+1, rect.bottom);
    LineTo (hdc, rect.right-1, rect.bottom);
    MoveTo (hdc, rect.right, rect.bottom-1);
    LineTo (hdc, rect.right, rect.top+1);

    SetPenColor (hdc, old_pen_color);
}

static void 
draw_3dbox (HDC hdc, const RECT* pRect, DWORD color, DWORD flag)
{
    FUNCTION_SCOPE_TIMING();
    DWORD light_color;
    DWORD dark_color;
    RECT rc_tmp = *pRect;
    BOOL is_have_lighter_color = FALSE;
    gal_pixel old_brush_color;
    gal_pixel old_pen_color;

    /*much small rect*/
    if (RECTW(rc_tmp)<2 || RECTH(rc_tmp)<2)
        return;

    old_brush_color = GetBrushColor(hdc);
    old_pen_color = GetPenColor(hdc);
    
    /*draw outer frame*/
    light_color = calc_3dbox_color (color, LFRDR_3DBOX_COLOR_LIGHTEST);
    dark_color = calc_3dbox_color (color, LFRDR_3DBOX_COLOR_DARKEST);
    if ((flag & LFRDR_BTN_STATUS_MASK) == LFRDR_BTN_STATUS_PRESSED)
    {
        draw_one_frame(hdc, &rc_tmp, dark_color, light_color);
    }
    else
    {
        /*thick frame left-top is main color, to draw a outline*/
        if (flag & LFRDR_3DBOX_THICKFRAME)
            draw_one_frame(hdc, &rc_tmp, color, dark_color);
        else
            draw_one_frame(hdc, &rc_tmp, light_color, dark_color);
    }

    /*draw inner frame*/
    if ((flag & LFRDR_3DBOX_THICKFRAME) 
            && (RECTW(rc_tmp)>6 && RECTH(rc_tmp)>6)) {
        light_color = calc_3dbox_color (color, LFRDR_3DBOX_COLOR_LIGHTER);
        dark_color = calc_3dbox_color (color, LFRDR_3DBOX_COLOR_DARKER);
        is_have_lighter_color = TRUE;

        DWINDLE_RECT(rc_tmp);
        if ((flag & LFRDR_BTN_STATUS_MASK) == LFRDR_BTN_STATUS_PRESSED)
        {
            draw_one_frame(hdc, &rc_tmp, dark_color, light_color);
        }
        else
        {
            draw_one_frame(hdc, &rc_tmp, light_color, dark_color);
        }
    }

    if (flag & LFRDR_3DBOX_FILLED) {
        DWINDLE_RECT(rc_tmp);
        switch (flag & LFRDR_BTN_STATUS_MASK) {
            case LFRDR_BTN_STATUS_HILITE:
                if (!is_have_lighter_color)
                    light_color = calc_3dbox_color (color, 
                                                LFRDR_3DBOX_COLOR_LIGHTER);

                SetBrushColor(hdc, RGBA2Pixel(hdc, GetRValue(light_color), 
                            GetGValue(light_color), GetBValue(light_color), 
                            GetAValue(light_color)));
                break;

            case LFRDR_BTN_STATUS_DISABLED:
            default:
                SetBrushColor(hdc, RGBA2Pixel(hdc, GetRValue(color), 
                            GetGValue(color), GetBValue(color), 
                            GetAValue(color)));
        }
        FillBox(hdc,  rc_tmp.left, rc_tmp.top,
                RECTW(rc_tmp), RECTH(rc_tmp));
    }

    SetPenColor(hdc, old_pen_color);
    SetBrushColor(hdc, old_brush_color);
}

static void 
draw_radio (HDC hdc, const RECT* pRect, DWORD color, int status)
{
    FUNCTION_SCOPE_TIMING();
    int radius, center_x, center_y, w, h;
    gal_pixel color_pixel, color_lightest, color_old;
    DWORD new_c;

    //FIXME
    /*if (pRect == NULL || hdc == HDC_INVALID)
        return;*/
    if (pRect == NULL)
        return;

    w = pRect->right - pRect->left;
    h = pRect->bottom - pRect->top;
    /*draw nothing*/
    if (w < 6 || h < 6)
        return;

    color_pixel = RGBA2Pixel (hdc, GetRValue(color), GetGValue(color), 
                        GetBValue(color), GetAValue(color));
    new_c = calc_3dbox_color (color, LFRDR_3DBOX_COLOR_LIGHTEST);
    new_c = calc_3dbox_color (new_c, LFRDR_3DBOX_COLOR_LIGHTEST);
    new_c = calc_3dbox_color (new_c, LFRDR_3DBOX_COLOR_LIGHTEST);
    color_lightest = RGBA2Pixel (hdc, GetRValue(new_c), GetGValue(new_c), 
                        GetBValue(new_c), GetAValue(new_c));

    radius = w>h ? (h>>1)-1 : (w>>1)-1;
    center_x = pRect->left + (w>>1);
    center_y = pRect->top + (h>>1);

    color_old = SetBrushColor (hdc, color_pixel);

    if (status & LFRDR_MARK_HAVESHELL)
    {
        FillCircle (hdc, center_x, center_y, radius);
        SetBrushColor (hdc, color_lightest);
        FillCircle (hdc, center_x, center_y, radius-1);

        SetBrushColor (hdc, GetBkColor(hdc));
        FillCircle (hdc, center_x, center_y, radius-2);

        SetBrushColor (hdc, color_pixel);
    }

    if (status & LFRDR_MARK_ALL_SELECTED)
    {
        FillCircle (hdc, center_x, center_y, radius>>1);
        SetBrushColor (hdc, color_lightest);
        FillCircle (hdc, center_x-1, center_y-1, 1);
    }

    SetBrushColor (hdc, color_old);
    return;
}

static void 
draw_checkbox (HDC hdc, const RECT* pRect, DWORD color, int status)
{
    FUNCTION_SCOPE_TIMING();
    int i, w, h, side_len, boundary;
    int box_l, box_t, box_r, box_b;
    int cross_l, cross_t, cross_r, cross_b;
    int border_cut;
    gal_pixel color_pixel, pen_color_old, bru_color_old;

    if (pRect == NULL)
        return;
    
    w = pRect->right - pRect->left;
    h = pRect->bottom - pRect->top;

    /*Draw nothing.*/
    if (w < 6 || h < 6)
        return;

    side_len = w>=h ? h : w;
    boundary = w>=h ? (w-h)>>1: (h-w)>>1;
    border_cut = (side_len+1)>>3;
    
    color_pixel = RGBA2Pixel (hdc, GetRValue(color), GetGValue(color), 
                        GetBValue(color), GetAValue(color));
    pen_color_old = SetPenColor (hdc, color_pixel);
    
    if (w > h)
    {
        box_l = pRect->left + boundary;
        box_t = pRect->top;
        box_r = box_l + side_len-1;
        box_b = pRect->bottom-1;
    }
    else if (w < h)
    {
        box_l = pRect->left;
        box_t = pRect->top + boundary;
        box_r = pRect->right-1;
        box_b = box_t + side_len-1;
    }
    else
    {
        box_l = pRect->left;
        box_t = pRect->top;
        box_r = pRect->right-1;
        box_b = pRect->bottom-1;
    }
    
    cross_l = box_l + ((side_len+1)>>2);
    cross_t = box_t + ((side_len+1)>>2);
    cross_r = box_r - ((side_len+1)>>2);
    cross_b = box_b - ((side_len+1)>>2);
    
    /*Draw border.*/
    if (status & LFRDR_MARK_HAVESHELL)
    {
        for (i=(side_len+1)>>3; i>0; i--)
        {
            MoveTo (hdc, box_l, box_t + i-1);
            LineTo (hdc, box_r, box_t + i-1);

            MoveTo (hdc, box_r, box_b - i+1);
            LineTo (hdc, box_l, box_b - i+1);

            MoveTo (hdc, box_r-i+1, box_t);
            LineTo (hdc, box_r-i+1, box_b);
            
            MoveTo (hdc, box_l+i-1, box_b);
            LineTo (hdc, box_l+i-1, box_t);
          
        }
    }
    
    /*Draw cross*/
    if (status & LFRDR_MARK_ALL_SELECTED)
    {
        for (i=(side_len+1)>>3; i>0; i--)
        {
            MoveTo (hdc, cross_l+i-1, cross_b);
            LineTo (hdc, cross_r-((side_len>>3)-i+1), cross_t);
            
            MoveTo (hdc, cross_l+i-1, cross_t);
            LineTo (hdc, cross_r-((side_len>>3)-i+1), cross_b);
        }
    }
    else if (status & LFRDR_MARK_HALF_SELECTED)
    {
        bru_color_old = SetBrushColor (hdc, COLOR_lightgray);
        FillBox (hdc, box_l + border_cut, box_t + border_cut, 
                        (box_r-box_l) - (border_cut<<1)+1, 
                        (box_b-box_t) - (border_cut<<1)+1) ;

        for (i=(side_len+1)>>3; i>0; i--)
        {
            MoveTo (hdc, cross_l+i-1, cross_b);
            LineTo (hdc, cross_r-((side_len>>3)-i+1), cross_t);
            
            MoveTo (hdc, cross_l+i-1, cross_t);
            LineTo (hdc, cross_r-((side_len>>3)-i+1), cross_b);
        }
        SetBrushColor (hdc, bru_color_old);
    }
    
    SetPenColor (hdc, pen_color_old);
    
    return;
}

static void 
draw_checkmark (HDC hdc, const RECT* pRect, DWORD color, int status)
{
    FUNCTION_SCOPE_TIMING();
    int i, w, h, side_len, boundary;
    int box_l, box_t, box_r, box_b;
    int hook_l, hook_t, hook_r, hook_b;
    int border_cut;
    gal_pixel color_pixel, pen_color_old, bru_color_old;
    
    if (pRect == NULL)
        return;
    
    w = pRect->right - pRect->left;
    h = pRect->bottom - pRect->top;
    
    /*Draw nothing.*/
    if (w < 6 || h < 6)
        return;

    side_len = w>=h ? h : w;
    boundary = w>=h ? (w-h)>>1: (h-w)>>1;
    border_cut = (side_len+1)>>3;

    color_pixel = RGBA2Pixel (hdc, GetRValue(color), GetGValue(color), 
                        GetBValue(color), GetAValue(color));
    pen_color_old = SetPenColor (hdc, color_pixel);
    
    if (w > h)
    {
        box_l = pRect->left + boundary;
        box_t = pRect->top;
        box_r = box_l + side_len-1;
        box_b = pRect->bottom-1;
    }
    else if (w < h)
    {
        box_l = pRect->left;
        box_t = pRect->top + boundary;
        box_r = pRect->right-1;
        box_b = box_t + side_len-1;
    }
    else
    {
        box_l = pRect->left;
        box_t = pRect->top;
        box_r = pRect->right-1;
        box_b = pRect->bottom-1;
    }
    
  
    hook_l = box_l + (side_len>>2);
    hook_t = box_t + (side_len>>2);
    hook_r = box_r - (side_len>>2);
    hook_b = box_b - (side_len>>2);

    bru_color_old = SetBrushColor (hdc, GetBkColor (hdc));
    FillBox (hdc, box_l + border_cut, box_t + border_cut, 
                    (box_r-box_l) - (border_cut<<1)+1, 
                    (box_b-box_t) - (border_cut<<1)+1) ;
    SetBrushColor (hdc, bru_color_old);

    /*Draw border.*/
    if (status & LFRDR_MARK_HAVESHELL)
    {
        for (i=(side_len+1)>>3; i>0; i--)
        {
            MoveTo (hdc, box_l, box_t + i-1);
            LineTo (hdc, box_r, box_t + i-1);

            MoveTo (hdc, box_r, box_b - i+1);
            LineTo (hdc, box_l, box_b - i+1);

            MoveTo (hdc, box_r-i+1, box_t);
            LineTo (hdc, box_r-i+1, box_b);
            
            MoveTo (hdc, box_l+i-1, box_b);
            LineTo (hdc, box_l+i-1, box_t);
        }
    }
    
    /*Draw hook*/
    if (status & LFRDR_MARK_ALL_SELECTED)
    {
        for (i=(side_len+1)>>3; i>0; i--)
        {
            MoveTo (hdc, hook_l, hook_t+((side_len+1)>>3)+i-1);
            LineTo (hdc, hook_l+((side_len+1)>>3), hook_b-(((side_len+1)>>3)-i+1));
            LineTo (hdc, hook_r, hook_t+i-1);
        }
    }
    else if (status & LFRDR_MARK_HALF_SELECTED)
    {
        bru_color_old = SetBrushColor (hdc, COLOR_lightgray);
        FillBox (hdc, box_l + border_cut, box_t + border_cut, 
                        (box_r-box_l) - (border_cut<<1)+1, 
                        (box_b-box_t) - (border_cut<<1)+1) ;

        for (i=(side_len+1)>>3; i>0; --i)
        {
            MoveTo (hdc, hook_l, hook_t+((side_len+1)>>3)+i-1);
            LineTo (hdc, hook_l+((side_len+1)>>3), hook_b-(((side_len+1)>>3)-i+1));
            LineTo (hdc, hook_r, hook_t+i-1);
        }
        SetBrushColor (hdc, bru_color_old);
    }
    
    SetPenColor (hdc, pen_color_old);

    return;
}

static void 
draw_arrow(HWND hWnd, HDC hdc, const RECT* pRect, DWORD color, int status)
{
    FUNCTION_SCOPE_TIMING();

    DWORD       c1, c2;
    int         w, h;
    RECT rc_tmp = *pRect; 
    RECT rc_tmp2;
    RECT rc_tmp3;
    POINT       p1, p2, p3;
    gal_pixel   old_pen_color, pixels[2], pixels2[2];

    w = RECTW (*pRect);
    h = RECTH (*pRect);
    if(w <= 0 || h <= 0) return;

    HPATH path;
    HBRUSH brush;
    HGRAPHICS graphic = MGPlusGraphicCreate (w, h);
    if (!graphic)
        return;
    //DK: BitBlt for fill four corner point with arrow button.
    BitBlt (hdc, pRect->left, pRect->top, w, 1, MGPlusGetGraphicDC (graphic), 0, 0, 0);
    BitBlt (hdc, pRect->left, pRect->top, w, 1, MGPlusGetGraphicDC (graphic), 0, h - 1, 0);
    brush = MGPlusBrushCreate (MP_BRUSH_TYPE_LINEARGRADIENT); 
    if (!brush){
        MGPlusGraphicDelete (graphic);
        return;
    }
    path = MGPlusPathCreate (MP_PATH_FILL_MODE_WINDING);
    if (!path) {
        goto ret;
    }

    c1 = GetWindowElementAttr (hWnd, WE_BGCB_ACTIVE_CAPTION);
    if(status & LFRDR_ARROW_HAVESHELL)
    {
        c2 = gradient_color (c1, LFRDR_3DBOX_COLOR_LIGHTER, 80);
        switch (status & LFRDR_BTN_STATUS_MASK) 
        {
            case LFRDR_BTN_STATUS_HILITE:
                pixels[0] = mp_gradient_color(c2, LFRDR_3DBOX_COLOR_LIGHTER, 220);
                pixels[1] = mp_gradient_color(c1, LFRDR_3DBOX_COLOR_LIGHTER, 100);
                
                pixels2[0] = mp_gradient_color(c1, LFRDR_3DBOX_COLOR_LIGHTER, 100);
                pixels2[1] = mp_gradient_color(c1, LFRDR_3DBOX_COLOR_LIGHTER, 200);
                break; 
            case LFRDR_BTN_STATUS_PRESSED:
                pixels[0] = mp_gradient_color(c2, LFRDR_3DBOX_COLOR_LIGHTER, 200);
                pixels[1] = mp_gradient_color(c1, LFRDR_3DBOX_COLOR_DARKER, 100);
                
                pixels2[0] = mp_gradient_color(c1, LFRDR_3DBOX_COLOR_DARKER, 100);
                pixels2[1] = mp_gradient_color(c1, LFRDR_3DBOX_COLOR_LIGHTER, 100);
                break;
            case LFRDR_BTN_STATUS_DISABLED:
            case LFRDR_BTN_STATUS_NORMAL:
            default:
                pixels[0] = mp_gradient_color(c2, LFRDR_3DBOX_COLOR_LIGHTER, 250);
                pixels[1] = mp_gradient_color(c2, LFRDR_3DBOX_COLOR_LIGHTER, 10);
                
                pixels2[0] = mp_gradient_color(c2, LFRDR_3DBOX_COLOR_LIGHTER, 10);
                pixels2[1] = mp_gradient_color(c1, LFRDR_3DBOX_COLOR_LIGHTER, 150);
                break;
        }

        switch (status & LFRDR_ARROW_DIRECT_MASK)
        {
            case LFRDR_ARROW_UP:
            case LFRDR_ARROW_DOWN:
                {
                    rc_tmp2.top = 1;
                    rc_tmp2.left = 1;
                    rc_tmp2.right = RECTW (rc_tmp)/3;
                    rc_tmp2.bottom = rc_tmp.bottom - 1;

                    rc_tmp3.top = rc_tmp2.top;
                    rc_tmp3.left = rc_tmp2.right;
                    rc_tmp3.right = RECTW (rc_tmp) - 1;
                    rc_tmp3.bottom = rc_tmp2.bottom;

                    //draw left gradient
                    MGPlusSetLinearGradientBrushMode(brush, MP_LINEAR_GRADIENT_MODE_HORIZONTAL);
                    MGPlusSetLinearGradientBrushRect(brush, &rc_tmp2);
                    MGPlusSetLinearGradientBrushColors(brush, (ARGB*)pixels, 2); 

                    MGPlusPathAddRectangleI(path, rc_tmp2.left, rc_tmp2.top, RECTW (rc_tmp2), RECTH (rc_tmp2));
                    MGPlusFillPath(graphic, brush, path); 
                    MGPlusPathReset(path);

                    //draw right gradient
                    MGPlusSetLinearGradientBrushRect(brush, &rc_tmp3);
                    MGPlusSetLinearGradientBrushColors(brush, (ARGB*)pixels2, 2); 

                    MGPlusPathAddRectangleI (path, rc_tmp3.left, rc_tmp3.top, RECTW (rc_tmp3), RECTH (rc_tmp3));
                    MGPlusFillPath (graphic, brush, path); 

                    MGPlusGraphicSave (graphic, hdc, 0, 0,\
                            RECTW (rc_tmp), RECTH (rc_tmp),\
                            rc_tmp.left, rc_tmp.top);
                }
                break;
            case LFRDR_ARROW_LEFT:
            case LFRDR_ARROW_RIGHT:
                {
                    rc_tmp2.top = 1;
                    rc_tmp2.left = 1;
                    rc_tmp2.right = RECTW (*pRect) - 1;
                    rc_tmp2.bottom = rc_tmp2.top + RECTH (*pRect)/3;

                    rc_tmp3.top = rc_tmp2.bottom;
                    rc_tmp3.left = rc_tmp2.left;
                    rc_tmp3.right = rc_tmp2.right;
                    rc_tmp3.bottom = RECTH (*pRect)- 1;

                    //draw top gradient
                    MGPlusSetLinearGradientBrushMode (brush, 
                            MP_LINEAR_GRADIENT_MODE_VERTICAL);
                    MGPlusSetLinearGradientBrushRect (brush, &rc_tmp2);
                    MGPlusSetLinearGradientBrushColors (brush, (ARGB*)pixels, 2); 

                    MGPlusPathAddRectangleI (path, rc_tmp2.left, rc_tmp2.top, RECTW (rc_tmp2), RECTH (rc_tmp2));
                    MGPlusFillPath (graphic, brush, path); 
                    MGPlusPathReset (path);

                    //draw bottom gradient
                    MGPlusSetLinearGradientBrushRect (brush, &rc_tmp3);
                    MGPlusSetLinearGradientBrushColors (brush, (ARGB*)pixels2, 2); 

                    MGPlusPathAddRectangleI (path, rc_tmp3.left, rc_tmp3.top, RECTW (rc_tmp3), RECTH (rc_tmp3));
                    MGPlusFillPath (graphic, brush, path); 

                    MGPlusGraphicSave (graphic, hdc, 0, 0,\
                            RECTW (rc_tmp), RECTH (rc_tmp),\
                            rc_tmp.left, rc_tmp.top);
                }
                break;
            default:
                break;
        }
    }

    draw_fashion_frame(hdc, rc_tmp, 
            gradient_color (c1, LFRDR_3DBOX_COLOR_DARKEST, 110)); 

    switch(status & LFRDR_ARROW_DIRECT_MASK)
    {
        case LFRDR_ARROW_UP:
            p1.x = pRect->left  + (w>>1);
            p1.y = pRect->top   + (h/3);
            p2.x = pRect->left  + (w>>2);
            p2.y = pRect->bottom- (h/3) - 1;
            p3.x = pRect->right - (w>>2) - 1;
            p3.y = pRect->bottom- (h/3) -1;
            break;
        case LFRDR_ARROW_DOWN:
            p1.x = pRect->left  + (w>>1);
            p1.y = pRect->bottom- (h/3) - 1;
            p2.x = pRect->left  + (w>>2);
            p2.y = pRect->top   + (h/3);
            p3.x = pRect->right - (w>>2) - 1;
            p3.y = pRect->top   + (h/3);
            break;
        case LFRDR_ARROW_LEFT:
            p1.x = pRect->left  + (w/3);
            p1.y = pRect->top   + (h>>1);
            p2.x = pRect->right - (w/3) - 1;
            p2.y = pRect->top   + (h>>2);
            p3.x = pRect->right - (w/3) - 1;
            p3.y = pRect->bottom- (h>>2) - 1;
            break;
        case LFRDR_ARROW_RIGHT:
            p1.x = pRect->right - (w/3) - 1;
            p1.y = pRect->top   + (h>>1);
            p2.x = pRect->left  + (w/3);
            p2.y = pRect->top   + (h>>2);
            p3.x = pRect->left  + (w/3);
            p3.y = pRect->bottom- (h>>2) - 1;
            break;
        default :
            goto ret;
    }

    color = gradient_color (color, LFRDR_3DBOX_COLOR_LIGHTER, 10);
    if(status & LFRDR_ARROW_NOFILL) {
        old_pen_color = SetPenColor(hdc, RGBA2Pixel(hdc,GetRValue(color),
                    GetGValue(color), GetBValue(color), GetAValue(color)));
        MoveTo(hdc, p1.x, p1.y);
        LineTo(hdc, p2.x, p2.y);
        LineTo(hdc, p3.x, p3.y);
        LineTo(hdc, p1.x, p1.y);
        SetPenColor (hdc, old_pen_color);
    }
    else {
        fill_iso_triangle(hdc, color, p1, p2, p3);
    }
ret:
    MGPlusGraphicDelete (graphic);
    MGPlusBrushDelete (brush);
    MGPlusPathDelete (path);
}

static char* gui_GetIconFile(const char* rdr_name, char* file, char* _szValue)
{
    FUNCTION_SCOPE_TIMING();
    char szValue[MAX_NAME + 1];
    char *iconname;

    strcpy(szValue, "icon/");
    iconname = szValue + strlen(szValue);

    if (GetMgEtcValue (rdr_name, file,
                iconname, sizeof(szValue)-(iconname-szValue)) < 0 ) {
        _MG_PRINTF ("SYSRES: can't get %s's value from section %s in etc.\n",
                file, rdr_name);
        return NULL;
    }
    strcpy(_szValue, szValue);
    return _szValue;
}

/* defined but not used
static BOOL gui_LoadIconRes(HDC hdc, const char* rdr_name, char* file) 
{
    FUNCTION_SCOPE_TIMING();
    char szValue[MAX_NAME + 1];

    gui_GetIconFile(rdr_name, file, szValue);

    if (LoadResource(szValue, RES_TYPE_ICON, (DWORD)hdc) != NULL)
        return TRUE;

    fprintf(stderr, "SYSRES: can't get %s's value from section %s in etc.\n",
            file, rdr_name);

    return FALSE;
}
*/

static void 
draw_fold (HWND hWnd, HDC hdc, const RECT* pRect, 
        DWORD color, int status, int next)
{
    FUNCTION_SCOPE_TIMING();
    /* houhh 20090410, if the treeview with icon style.*/
    if (status & LFRDR_TREE_WITHICON) {
        int w, h;
        int centerX, centerY;
        char szValue[255];
        HICON hFoldIcon   = (HICON)RetrieveRes (gui_GetIconFile("skin", (char*)SYSICON_TREEFOLD, szValue));
        HICON hUnFoldIcon = (HICON)RetrieveRes (gui_GetIconFile("skin", (char*)SYSICON_TREEUNFOLD, szValue));

        w = RECTWP(pRect);
        h = RECTHP(pRect);

        if(w < 4 || h < 4) return;

        centerX = pRect->left + (w>>1);
        centerY = pRect->top + (h>>1);

        if (status & LFRDR_TREE_CHILD) {
            if (status & LFRDR_TREE_FOLD) {
                if (hFoldIcon)
                    DrawIcon (hdc, centerX, centerY - (h>>1),
                            h, h, hFoldIcon);
            }
            else {
                if (hUnFoldIcon)
                    DrawIcon (hdc, centerX, centerY - (h>>1),
                            h, h, hUnFoldIcon);
            }
        }
    }
    else {
        int flag;

        if (status & LFRDR_TREE_CHILD) {
            if (status & LFRDR_TREE_FOLD)
                flag = LFRDR_ARROW_RIGHT;
            else
                flag = LFRDR_ARROW_DOWN;
        }
        else 
            return;

        return draw_arrow (hWnd, hdc, pRect, color, flag);
    }
}

static void 
draw_focus_frame (HDC hdc, const RECT *pRect, DWORD color)
{
    FUNCTION_SCOPE_TIMING();
    int i;
    gal_pixel pixel;

    
    pixel = RGBA2Pixel(hdc, GetRValue(color), GetGValue(color), 
                            GetBValue(color), GetAValue(color));

    for(i = pRect->left; i < pRect->right; i++)
    {
        if(i & 0x01)
        {
            SetPixel(hdc, i, pRect->top, pixel);
            SetPixel(hdc, i, pRect->bottom, pixel);
        }
    }
    for(i = pRect->top; i < pRect->bottom; i++)
    {
        if(i & 0x01)
        {
            SetPixel(hdc, pRect->left, i ,pixel);
            SetPixel(hdc, pRect->right, i ,pixel);
        }
    }
}

static void draw_normal_item 
(HWND hWnd, HDC hdc, const RECT* pRect, DWORD color)
{
    gal_pixel old_bc;
    old_bc = SetBrushColor (hdc, DWORD2Pixel (hdc, color));
    FillBox (hdc, pRect->left, pRect->top, RECTWP (pRect), RECTHP (pRect));
    SetBrushColor (hdc, old_bc);
}

static void draw_hilite_item 
(HWND hWnd, HDC hdc, const RECT* pRect, DWORD color)
{
    FUNCTION_SCOPE_TIMING();
    gal_pixel pixels[3];
    int corner, l, t, r, b;
   
    pixels [0] = MPMakeARGB (GetRValue(color), GetGValue(color), GetBValue(color), GetAValue(color));
    pixels [1] = mp_gradient_color (color, LFRDR_3DBOX_COLOR_LIGHTER, 120);
    pixels[2] = pixels[0];

    linear_gradient_draw(hdc, MP_LINEAR_GRADIENT_MODE_VERTICAL, 
            (RECT*)pRect, pixels, 3); 

    /*draw border*/
    corner = 1;
    l = pRect->left;
    t = pRect->top;
    r = pRect->right;
    b = pRect->bottom;

    pixels [0] = (pixels [0] & 0xFF000000) | 
                ((pixels [0] & 0x000000FF) << 16) |
                (pixels [0] & 0x0000FF00) | 
                ((pixels [0] & 0x00FF0000) >> 16);

    SetPenColor (hdc, pixels[0]); 
    MoveTo (hdc, l + corner, t);
    LineTo (hdc, r - 1 - corner, t);
    LineTo (hdc, r - 1, t+corner);
    LineTo (hdc, r - 1, b - 1-corner);
    LineTo (hdc, r - 1-corner, b - 1);
    LineTo (hdc, l+corner, b - 1);
    LineTo (hdc, l, b - 1-corner);
    LineTo (hdc, l, t+corner);
    LineTo (hdc, l+corner, t);
}

static void draw_disabled_item 
(HWND hWnd, HDC hdc, const RECT* pRect, DWORD color)
{
    FUNCTION_SCOPE_TIMING();
    gal_pixel old_bc;
    old_bc = SetBrushColor (hdc, DWORD2Pixel (hdc, color));
    FillBox (hdc, pRect->left, pRect->top, RECTWP (pRect), RECTHP (pRect));
    SetBrushColor (hdc, old_bc);
}

static void draw_significant_item 
(HWND hWnd, HDC hdc, const RECT* pRect, DWORD color)
{
    gal_pixel old_bc;
    old_bc = SetBrushColor (hdc, DWORD2Pixel (hdc, color));
    FillBox (hdc, pRect->left, pRect->top, RECTWP (pRect), RECTHP (pRect));
    SetBrushColor (hdc, old_bc);
}

static void draw_normal_menu_item 
(HWND hWnd, HDC hdc, const RECT* pRect, DWORD color)
{
    gal_pixel old_bc;
    old_bc = SetBrushColor (hdc, DWORD2Pixel (hdc, color));
    FillBox (hdc, pRect->left, pRect->top, RECTWP (pRect), RECTHP (pRect));
    SetBrushColor (hdc, old_bc);
}

static void draw_hilite_menu_item 
(HWND hWnd, HDC hdc, const RECT* pRect, DWORD color)
{
    FUNCTION_SCOPE_TIMING();
    gal_pixel pixels[3];
    int l, t, r, b;

    pixels [1] = MPMakeARGB (GetRValue (color), GetGValue (color), GetBValue (color),
            GetAValue (color));
    pixels [0] = mp_gradient_color (color, LFRDR_3DBOX_COLOR_LIGHTER, 240);
    pixels [2] = mp_gradient_color (color, LFRDR_3DBOX_COLOR_LIGHTER, 240);

    linear_gradient_draw(hdc, MP_LINEAR_GRADIENT_MODE_VERTICAL,
            (RECT*)pRect, pixels, 3);

    /*draw border*/
    l = pRect->left;
    t = pRect->top;
    r = pRect->right;
    b = pRect->bottom;

    pixels[0] = DWORD2Pixel (hdc, gradient_color 
            (color, LFRDR_3DBOX_COLOR_DARKER, 140));
    SetPenColor(hdc, pixels[0]); 
    --r;
    --b;
    Rectangle (hdc, l, t, r, b);
    ++l;
    ++t;
    --r;
    --b;
    Rectangle (hdc, l, t, r, b);
}

static void draw_disabled_menu_item 
(HWND hWnd, HDC hdc, const RECT* pRect, DWORD color)
{
    FUNCTION_SCOPE_TIMING();
    gal_pixel old_bc;
    old_bc = SetBrushColor (hdc, DWORD2Pixel (hdc, color));
    FillBox (hdc, pRect->left, pRect->top, RECTWP (pRect), RECTHP (pRect));
    SetBrushColor (hdc, old_bc);
}

#define STATUS_GET_CHECK(status) ((status) & BST_CHECK_MASK)

#define GET_BTN_POSE_STATUS(status) ((status) & BST_POSE_MASK)

static void 
draw_push_button (HWND hWnd, HDC hdc, const RECT* pRect, 
        DWORD color1, DWORD color2, int status)
{
    FUNCTION_SCOPE_TIMING();

    DWORD   c1, tmpcolor;
    RECT    frRect, upRect, downRect;
    int     corner = 2;
    BOOL    flag = TRUE;
    gal_pixel uppixel[2], downpixel[2], old_pen_color;

    frRect.left = pRect->left;
    frRect.right = pRect->right - 1;
    frRect.top = pRect->top;
    frRect.bottom = pRect->bottom - 1;

    upRect.left = pRect->left + 1;
    upRect.right = frRect.right;
    upRect.top = pRect->top + 1;
    upRect.bottom = pRect->top + (RECTH(*pRect) >> 1) - 3;

    downRect.left = upRect.left;
    downRect.right = upRect.right;
    downRect.top = upRect.bottom;
    downRect.bottom = pRect->bottom - 1;

    old_pen_color = GetPenColor (hdc);
    c1 = GetWindowElementAttr (hWnd, WE_BGCB_ACTIVE_CAPTION);
    tmpcolor = gradient_color (c1, LFRDR_3DBOX_COLOR_LIGHTER, 80);

    if (status == BST_FOCUS) {
        flag = FALSE;
    } else if (STATUS_GET_CHECK (status) != BST_UNCHECKED) {
        if (STATUS_GET_CHECK (status) == BST_CHECKED) {
            /*Draw checked status: Use caption color*/
            uppixel [0] = mp_gradient_color (tmpcolor, LFRDR_3DBOX_COLOR_LIGHTER, 250);
            uppixel [1] = mp_gradient_color (tmpcolor, LFRDR_3DBOX_COLOR_LIGHTER, 70);

            downpixel [0] = mp_gradient_color (c1, LFRDR_3DBOX_COLOR_LIGHTER, 50);
            downpixel [1] = mp_gradient_color (c1, LFRDR_3DBOX_COLOR_LIGHTER, 150);
        } else if (STATUS_GET_CHECK (status) == BST_INDETERMINATE) {
            /*Use color1*/
            uppixel [1] = tmpcolor;
            uppixel [0] = uppixel [1];

            downpixel [0] = downpixel[1] = uppixel[0];
        } else {
            flag = FALSE;
        }
    } else {
        /*Draw normal status*/
        if (GET_BTN_POSE_STATUS(status) == BST_PUSHED) {
            uppixel [0] = mp_gradient_color (tmpcolor, LFRDR_3DBOX_COLOR_LIGHTER, 200);
            uppixel [1] = mp_gradient_color (tmpcolor, LFRDR_3DBOX_COLOR_LIGHTER, 70);

            downpixel [0] = mp_gradient_color (c1, LFRDR_3DBOX_COLOR_LIGHTER, 50);
            downpixel [1] = mp_gradient_color (c1, LFRDR_3DBOX_COLOR_LIGHTER, 150);

        } else if (GET_BTN_POSE_STATUS(status) == BST_HILITE) {
            /*Draw hilight status*/
            uppixel [0] = mp_gradient_color (tmpcolor, LFRDR_3DBOX_COLOR_LIGHTER, 250);
            uppixel [1] = mp_gradient_color (tmpcolor, LFRDR_3DBOX_COLOR_LIGHTER, 70);

            downpixel [0] = mp_gradient_color (c1, LFRDR_3DBOX_COLOR_LIGHTER, 90);
            downpixel [1] = mp_gradient_color (c1, LFRDR_3DBOX_COLOR_LIGHTER, 180);
        } else if (GET_BTN_POSE_STATUS(status) == BST_DISABLE) {
            uppixel [1] = tmpcolor;
            uppixel [0] = uppixel[1];

            downpixel [0] = downpixel[1] = uppixel[0];
        } else {
            flag = FALSE;
        }
    }

    if (!flag)
    {
        uppixel [0] = mp_gradient_color (tmpcolor, LFRDR_3DBOX_COLOR_LIGHTER, 250);
        uppixel [1] = mp_gradient_color (tmpcolor, LFRDR_3DBOX_COLOR_LIGHTER, 70);

        downpixel [0] = mp_gradient_color (c1, LFRDR_3DBOX_COLOR_LIGHTER, 50);
        downpixel [1] = mp_gradient_color (c1, LFRDR_3DBOX_COLOR_LIGHTER, 150);
    }

    HPATH path;
    HBRUSH brush;
    HGRAPHICS graphic = MGPlusGraphicCreate(RECTW(*pRect), RECTH(*pRect));
    if (!graphic)
        return;

    BitBlt (hdc, pRect->left, pRect->top, RECTW (*pRect),
            RECTH (*pRect), MGPlusGetGraphicDC (graphic),
            0, 0, 0);

    brush = MGPlusBrushCreate(MP_BRUSH_TYPE_LINEARGRADIENT); 
    if (!brush){
        MGPlusGraphicDelete(graphic);
        return;
    }

    path = MGPlusPathCreate(MP_PATH_FILL_MODE_WINDING);
    if (!path) {
        MGPlusGraphicDelete(graphic);
        MGPlusBrushDelete(brush);
        return;
    }

    MGPlusSetLinearGradientBrushMode(brush, MP_LINEAR_GRADIENT_MODE_VERTICAL);

    upRect.left = 1;
    upRect.right = frRect.right - pRect->left + 1;
    upRect.top = 1;
    upRect.bottom = (RECTH(*pRect) >> 1) - 3;

    downRect.left = upRect.left;
    downRect.right = upRect.right;
    downRect.top = upRect.bottom;
    downRect.bottom = RECTH (*pRect);

    MGPlusSetLinearGradientBrushRect (brush, &upRect);
    MGPlusSetLinearGradientBrushColors (brush, (ARGB*)uppixel, 2); 
    MGPlusPathAddRectangleI (path, upRect.left, upRect.top, RECTW (upRect), 
            RECTH (upRect));
    MGPlusFillPath (graphic, brush, path); 
    MGPlusPathReset (path);

    MGPlusSetLinearGradientBrushRect (brush, &downRect);
    MGPlusSetLinearGradientBrushColors (brush, (ARGB*)downpixel, 2); 
    MGPlusPathAddRectangleI (path, downRect.left, downRect.top, RECTW (downRect), 
            RECTH (downRect));
    MGPlusFillPath (graphic, brush, path); 

    MGPlusGraphicSave (graphic, hdc, 0, 0, 0, 0, 
            upRect.left - 1, upRect.top - 1);

    MGPlusPathDelete (path);
    MGPlusBrushDelete (brush);
    MGPlusGraphicDelete (graphic);

    //draw border top
    SetPenColor (hdc, DWORD2Pixel(hdc, c1));
    MoveTo (hdc, frRect.left + corner, frRect.top);
    LineTo (hdc, frRect.right - corner, frRect.top);
    LineTo (hdc, frRect.right, frRect.top+corner);

    //right
    LineTo (hdc, frRect.right, frRect.bottom-corner);
    LineTo (hdc, frRect.right-corner, frRect.bottom);

    //bottom
    LineTo (hdc, frRect.left+corner, frRect.bottom);
    LineTo (hdc, frRect.left, frRect.bottom-corner);

    //left
    LineTo (hdc, frRect.left, frRect.top+corner);
    LineTo (hdc, frRect.left+corner, frRect.top);

    SetPenColor (hdc, old_pen_color);
}

#define SIZE_CHECKBOX 13
static void 
_draw_check_button (HDC hdc, 
        const RECT* pRect, int status, const BITMAP* pBmp)
{
    FUNCTION_SCOPE_TIMING();
    int  w = RECTWP(pRect);
    int  h = RECTHP(pRect);
    int box_l = 0, box_t = 0;
    int off_h = 0, off_v = 0;

    if (w <= 0 || h <=0) return;

    box_l = pRect->left + (w >> 1) - (SIZE_CHECKBOX >> 1);
    box_t = pRect->top + (h >> 1) - (SIZE_CHECKBOX >> 1);
    if (w & 0x1) box_l += 1;
    if (h & 0x1) box_t += 1;

    if (box_l < pRect->left) {
        off_h = pRect->left - box_l;
        box_l = pRect->left;
    }

    if (box_t < pRect->top) {
        off_v = pRect->top - box_t;
        box_t = pRect->top;
    }

    // Target DC compatible with resource DC
    if (IsCompatibleDC(HDC_SCREEN, hdc))
    {
        FillBoxWithBitmapPart (hdc, box_l, box_t, 
                SIZE_CHECKBOX - (off_h << 1), SIZE_CHECKBOX - (off_v << 1), 0, 0, 
                pBmp, off_h, status * SIZE_CHECKBOX + off_v);
    }
    else
    {
        // DK [12/18/09]: For fix bug 4289.
        // Bug occurred when the bitmap of button is not compatible with hdc,
        // in this case, `FillBoxWithBitmapPart' can't output bitmap to DC.
        // Now we replace the bitmap with memory DC for fix the bug, cause of
        // the memory DC have more information about color depth and so on,
        // so it can be converted to the compatible DC with target DC. 
        HDC bmpToDC = CreateMemDCFromBitmap(HDC_SCREEN, pBmp);
        if (HDC_INVALID != bmpToDC)
        {

            BitBlt (bmpToDC, off_h, status * SIZE_CHECKBOX + off_v, 
                    SIZE_CHECKBOX - (off_h << 1), SIZE_CHECKBOX - (off_v << 1),
                    hdc, box_l, box_t, 0);
            DeleteMemDC(bmpToDC);
        }
    }
}

#define SIZE_RADIOBTN 13
static void 
_draw_radio_button (HWND hWnd, HDC hdc,
        const RECT* pRect, int status, const BITMAP* pBmp)
{
    FUNCTION_SCOPE_TIMING();
    // There is not necessary CreateMemDC everytime because that is poor 
    // efficiency，so code of below replaced with a new implementation.
#if 0
    int  w = RECTWP(pRect);
    int  h = RECTHP(pRect);
    int  box_l = 0, box_t = 0, box_r, box_b;
    int  bmp_w, bmp_h;
    gal_pixel bgc;

    bmp_w = pBmp->bmWidth; 
    bmp_h = pBmp->bmHeight / BTN_STATUS_NUM;

    // DK [12/18/09]: For fix bug 4289.
    // Bug occurred when the bitmap of button is not compatible with hdc,
    // in this case, `FillBoxWithBitmapPart' can't output bitmap to DC.
    // Now we replace the bitmap with memory DC for fix the bug, cause of
    // the memory DC have more information about color depth and so on,
    // so it can be converted to the compatible DC with target DC. 
    HDC bmpToDC = CreateCompatibleDCEx(HDC_SCREEN, bmp_w, pBmp->bmHeight);
    if (HDC_INVALID != bmpToDC)
    {
        if (FillBoxWithBitmap(bmpToDC, 0, 0, pBmp->bmWidth, pBmp->bmHeight, pBmp))
        {
            ConvertMemDC(bmpToDC, hdc, MEMDC_FLAG_SWSURFACE);
            // DK [12/18/09]: There is a bug of set color key whith memory DC:
            // If not invoke `SetMemDCAlpha' to set the alpha value at the same time,
            // the color key which set used 'SetMemDCColorKey' is useless, that is
            // why of the function 'SetMemDCAlpha' been here.
            SetMemDCAlpha(bmpToDC, MEMDC_FLAG_SRCALPHA, 255);
            SetMemDCColorKey(bmpToDC, MEMDC_FLAG_SRCCOLORKEY, GetPixel(bmpToDC, 0, 0));
            if (w >= bmp_w && h >= bmp_w) {
                box_l = pRect->left + ((w - bmp_w) >> 1);
                box_t = pRect->top + ((h - bmp_h) >> 1);

                /*parity check*/
                if (w & 0x1) box_l += 1;
                if (h & 0x1) box_t += 1;

                BitBlt(bmpToDC, 0, status * bmp_h, bmp_w, bmp_h, hdc, box_l, box_t, 0);
            }
            else {
                BitBlt(bmpToDC, 0, status * bmp_h, bmp_w, bmp_h, hdc, pRect->left, pRect->top, 0);
            }
        }
        DeleteMemDC(bmpToDC);
    }
#endif
    int  w = RECTWP(pRect);
    int  h = RECTHP(pRect);
    int  box_l = 0, box_t = 0, off_h = 0, off_v = 0;

    if (w <= 0 || h <=0) return;

    box_l = pRect->left + (w >> 1) - (SIZE_RADIOBTN >> 1);
    box_t = pRect->top + (h >> 1) - (SIZE_RADIOBTN >> 1);
    if (w & 0x1) box_l += 1;
    if (h & 0x1) box_t += 1;

    if (box_l < pRect->left) {
        off_h = pRect->left - box_l;
        box_l = pRect->left;
    }

    if (box_t < pRect->top) {
        off_v = pRect->top - box_t;
        box_t = pRect->top;
    }

    // Target DC compatible with resource DC
    if (IsCompatibleDC(HDC_SCREEN, hdc))
    {
        BITMAP* bmp = const_cast<BITMAP*>(pBmp);
        bmp->bmType = BMP_TYPE_COLORKEY;
        bmp->bmColorKey = GetPixelInBitmap(bmp, 0, 0);
        FillBoxWithBitmapPart (hdc, box_l, box_t, 
                SIZE_RADIOBTN - (off_h << 1), SIZE_RADIOBTN - (off_v << 1), 0, 0, 
                bmp, off_h, status * SIZE_RADIOBTN + off_v);
    }
    else
    {
        // DK [12/18/09]: For fix bug 4289.
        // Bug occurred when the bitmap of button is not compatible with hdc,
        // in this case, `FillBoxWithBitmapPart' can't output bitmap to DC.
        // Now we replace the bitmap with memory DC for fix the bug, cause of
        // the memory DC have more information about color depth and so on,
        // so it can be converted to the compatible DC with target DC. 
        HDC bmpToDC = CreateMemDCFromBitmap(HDC_SCREEN, pBmp);
        if (HDC_INVALID != bmpToDC)
        {
            // DK [12/18/09]: There is a bug of set color key whith memory DC:
            // If not invoke `SetMemDCAlpha' to set the alpha value at the same time,
            // the color key which set used 'SetMemDCColorKey' is useless, that is
            // why of the function 'SetMemDCAlpha' been here.
            SetMemDCAlpha(bmpToDC, MEMDC_FLAG_SRCALPHA, 255);
            SetMemDCColorKey(bmpToDC, MEMDC_FLAG_SRCCOLORKEY, GetPixel(bmpToDC, 0, 0));

            BitBlt (bmpToDC, off_h, status * SIZE_RADIOBTN + off_v, 
                    SIZE_RADIOBTN - (off_h << 1), SIZE_RADIOBTN - (off_v << 1),
                    hdc, box_l, box_t, 0);
            DeleteMemDC(bmpToDC);
        }
    }
}

static void 
draw_radio_button (HWND hWnd, HDC hdc, const RECT* pRect, int status)
{
    FUNCTION_SCOPE_TIMING();
    const BITMAP* bmp = 
        GetSystemBitmapEx (wnd_rdr_fashion.name, SYSBMP_RADIOBUTTON);
    if (NULL != bmp)
    {
        _draw_radio_button (hWnd, hdc, pRect, status, bmp);
    }
}

static void 
draw_check_button (HWND hWnd, HDC hdc, const RECT* pRect, int status)
{
    FUNCTION_SCOPE_TIMING();
    const BITMAP* bmp = 
        GetSystemBitmapEx (wnd_rdr_fashion.name, SYSBMP_CHECKBUTTON);

    if (bmp == NULL)
        printf ("LF_FASHION: check_button bmp is null\n");
    else
        _draw_check_button (hdc, pRect, status, bmp);
}

static int calc_capbtn_area (HWND hWnd, int which, RECT* we_area)
{
    int border;
    int cap_h;
    int capbtn_h;
    const WINDOWINFO* win_info = GetWindowInfo (hWnd);
    int icon_h;
    int icon_w;
    int win_w = win_info->right - win_info->left;

    cap_h = GetWindowElementAttr (hWnd, WE_METRICS_CAPTION);

    switch (which)
    {
        case HT_CLOSEBUTTON:            
            CHECKNOT_RET_ERR (IS_CLOSEBTN_VISIBLE (win_info));
            border = get_window_border (hWnd, 0, 0);
            capbtn_h = cap_h - (CAP_BTN_INTERVAL<<1);
            we_area->right = win_w - border - CAP_BTN_INTERVAL;
            we_area->left = we_area->right - capbtn_h;
            we_area->top = border + ((cap_h-capbtn_h)>>1); 
            we_area->bottom = we_area->top + capbtn_h;
            return 0;

        case HT_MAXBUTTON:
            CHECKNOT_RET_ERR (IS_MAXIMIZEBTN_VISIBLE (win_info));
            border = get_window_border (hWnd, 0, 0);
            capbtn_h = cap_h - (CAP_BTN_INTERVAL<<1);

            we_area->right = win_w - border - capbtn_h
                        - (CAP_BTN_INTERVAL<<1);
            we_area->left = we_area->right - capbtn_h;
            we_area->top = border + ((cap_h-capbtn_h)>>1); 
            we_area->bottom = we_area->top + capbtn_h;
            return 0;

        case HT_MINBUTTON:
            CHECKNOT_RET_ERR (IS_MINIMIZEBTN_VISIBLE (win_info));

            border = get_window_border (hWnd, 0, 0);
            capbtn_h = cap_h - (CAP_BTN_INTERVAL<<1);

            we_area->right = win_w - border - (capbtn_h<<1)
                            - CAP_BTN_INTERVAL*3;
            we_area->left = we_area->right - capbtn_h;
            we_area->top = border + ((cap_h-capbtn_h)>>1); 
            we_area->bottom = we_area->top + capbtn_h;
            return 0;

        case HT_ICON:
            CHECKNOT_RET_ERR (IS_CAPICON_VISIBLE (win_info));

            border = get_window_border (hWnd, 0, 0);

            GetIconSize (win_info->hIcon, &icon_w, &icon_h);

            /*icon is larger, zoomin*/
            if (icon_h > cap_h - (CAP_BTN_INTERVAL<<1))
            {
                icon_h = cap_h - (CAP_BTN_INTERVAL<<1);
                icon_w =  icon_w * icon_h / icon_h;
            }

            we_area->left = border + CAP_BTN_INTERVAL;
            we_area->right = we_area->left + icon_w;
            we_area->top = border + ((cap_h - icon_h) >> 1);
            we_area->bottom = we_area->top + icon_h;
            return 0;

        default:
            return -1;
    }
}

static int calc_hscroll_area(HWND hWnd, int which, RECT* we_area)
{
    FUNCTION_SCOPE_TIMING();
    int border;
    int scrollbar;
    int left_inner = 0;
    int right_inner = 0;
    const WINDOWINFO* win_info = GetWindowInfo (hWnd);
    const LFSCROLLBARINFO* sbar_info = &(win_info->hscroll);

    int win_w = win_info->right - win_info->left;
    int win_h = win_info->bottom - win_info->top;

    border = get_window_border (hWnd, 0, 0);
    scrollbar = GetWindowElementAttr(hWnd, WE_METRICS_SCROLLBAR);

    if (IS_VSCROLL_VISIBLE (win_info)) {
        if (IS_LEFT_VSCOLLBAR (win_info)) 
            left_inner = scrollbar;
        else
            right_inner = scrollbar;
    }

    we_area->bottom = win_h - border;
    we_area->top = we_area->bottom - scrollbar;

    switch (which) {
        case HT_HSCROLL:
            we_area->left = border + left_inner;
            we_area->right = win_w - border - right_inner;
            return 0;

        case HT_SB_HTHUMB:
            we_area->left = border + left_inner + scrollbar
                            + sbar_info->barStart;
            we_area->right = we_area->left + sbar_info->barLen;
            return 0;

        case HT_SB_LEFTARROW:
            we_area->left = border + left_inner;
            we_area->right = we_area->left + scrollbar;
            return 0;

        case HT_SB_RIGHTARROW:
            we_area->right = win_w - border - right_inner;
            we_area->left = we_area->right - scrollbar;
            return 0;

        case HT_SB_LEFTSPACE:
            we_area->left = border + left_inner + scrollbar;
            we_area->right = we_area->left + sbar_info->barStart;
            return 0;

        case HT_SB_RIGHTSPACE:
            we_area->left = border + left_inner + scrollbar
                            + sbar_info->barStart + sbar_info->barLen;
            we_area->right = win_w - border - right_inner - scrollbar;
            return 0;

        default:
            return -1;

    }

}

static int calc_vscroll_area (HWND hWnd, int which, RECT* we_area)
{
    FUNCTION_SCOPE_TIMING();
    int border;
    int cap_h;
    int menu_h;
    int scrollbar;
    int bottom_inner = 0;

    const WINDOWINFO* win_info = GetWindowInfo (hWnd);
    const LFSCROLLBARINFO* sbar_info = &(win_info->vscroll);

    int win_w = win_info->right - win_info->left;
    int win_h = win_info->bottom - win_info->top;

    border = get_window_border (hWnd, 0, 0);
    cap_h = get_window_caption (hWnd);
    menu_h = get_window_menubar (hWnd);
    scrollbar = GetWindowElementAttr (hWnd, WE_METRICS_SCROLLBAR);

    if (IS_HSCROLL_VISIBLE (win_info))
        bottom_inner = scrollbar;

    if (IS_LEFT_VSCOLLBAR (win_info)) {
        we_area->left = border;
        we_area->right = we_area->left + scrollbar;
    }
    else {
        we_area->right = win_w - border;
        we_area->left = we_area->right - scrollbar;
    }

    switch (which) {
        case HT_VSCROLL:
            we_area->top = border + cap_h + menu_h;
            we_area->bottom = win_h - border - bottom_inner ;
            return 0;

        case HT_SB_VTHUMB:
            we_area->top = border + cap_h + menu_h + scrollbar
                         + sbar_info->barStart;
            we_area->bottom = we_area->top + sbar_info->barLen;
            return 0;

        case HT_SB_UPARROW:
            we_area->top = border + cap_h + menu_h;
            we_area->bottom = we_area->top + scrollbar;
            return 0;

        case HT_SB_DOWNARROW:
            we_area->bottom = win_h - border - bottom_inner;
            we_area->top = we_area->bottom - scrollbar;
            return 0;

        case HT_SB_UPSPACE:
            we_area->top = border + cap_h + menu_h + scrollbar;
            we_area->bottom = we_area->top + sbar_info->barStart;
            return 0;

        case HT_SB_DOWNSPACE:
            we_area->top = border + cap_h + menu_h + scrollbar
                         + sbar_info->barStart + sbar_info->barLen;
            we_area->bottom = win_h - border - scrollbar - bottom_inner;
            return 0;

        default:
            return -1;

    }

}

static int get_window_border (HWND hWnd, int dwStyle, int win_type)
{
    FUNCTION_SCOPE_TIMING();
    int _dwStyle = dwStyle;
    int _type = win_type;

    if (hWnd != HWND_NULL) {
        _type = lf_get_win_type (hWnd);
        _dwStyle = GetWindowStyle (hWnd);
    }

    switch (_type)
    {
        case LFRDR_WINTYPE_MAINWIN:
        {
            if (_dwStyle & WS_BORDER
                ||_dwStyle & WS_THICKFRAME)
                return GetWindowElementAttr (hWnd, WE_METRICS_WND_BORDER) + 3;
            else if(_dwStyle & WS_THINFRAME)
                return GetWindowElementAttr (hWnd, WE_METRICS_WND_BORDER);
        }
        case LFRDR_WINTYPE_DIALOG:
        {
            if (_dwStyle & WS_BORDER
                ||_dwStyle & WS_THICKFRAME)
                return 3;
            else if(_dwStyle & WS_THINFRAME)
                return 0; 
        }
        case LFRDR_WINTYPE_CONTROL:
        {
            if (_dwStyle & WS_BORDER
                    || _dwStyle & WS_THINFRAME
                    || _dwStyle & WS_THICKFRAME)
                return 1; 
        }
    }

    return 0;
}

static int calc_we_metrics (HWND hWnd, 
            LFRDR_WINSTYLEINFO* style_info, int which)
{
    FUNCTION_SCOPE_TIMING();
    const WINDOWINFO* win_info;
    int         cap_h = 0;
    int         icon_w = 0;
    int         icon_h = 0;
    PLOGFONT    cap_font;
    int         win_width, win_height;
    int         btn_w = 0;

    switch (which & LFRDR_METRICS_MASK)
    {
        case LFRDR_METRICS_BORDER:
            if (style_info)
                return get_window_border (HWND_NULL, 
                        style_info->dwStyle, style_info->winType);
            else if (hWnd != HWND_NULL) 
                return get_window_border (hWnd, 0, 0);
            else
                return GetWindowElementAttr(hWnd, WE_METRICS_WND_BORDER);

        case LFRDR_METRICS_CAPTION_H:
        {
            int cap_h, _idx;

            if (style_info) {
                if (!(style_info->dwStyle & WS_CAPTION)) {
                    return 0;
                }

                _idx = WE_METRICS_CAPTION & WE_ATTR_INDEX_MASK;
                cap_h = wnd_rdr_fashion.we_metrics[_idx];
                return cap_h;
            }
            else if (hWnd != HWND_NULL) {
                win_info = GetWindowInfo(hWnd);
                if (!IS_CAPTION_VISIBLE (win_info))
                    return 0;
            }

            cap_h = GetWindowElementAttr (hWnd, WE_METRICS_CAPTION);
            return cap_h;
        }

        case LFRDR_METRICS_MENU_H:
        {
            int _idx;

            if (style_info) {
                _idx = WE_METRICS_MENU & WE_ATTR_INDEX_MASK;
                return wnd_rdr_fashion.we_metrics[_idx];
            }
            else if (hWnd != HWND_NULL) {
                win_info = GetWindowInfo(hWnd);
                if (!IS_MENUBAR_VISIBLE (win_info))
                    return 0;
            }

            return GetWindowElementAttr (hWnd, WE_METRICS_MENU);
        }

        case LFRDR_METRICS_ICON_H:
        {
            if (hWnd != HWND_NULL) {
                win_info = GetWindowInfo(hWnd);

                if (!IS_CAPTION_VISIBLE (win_info))
                    return 0;

                cap_h = 
                    calc_we_metrics (hWnd, style_info, LFRDR_METRICS_CAPTION_H);
                if (win_info->hIcon)
                    GetIconSize (win_info->hIcon, NULL, &icon_h);
                else
                    icon_h = 0;

                return (icon_h<cap_h) ? icon_h : cap_h;
            }
            else {
                return 16;
            }
        }
            
        case LFRDR_METRICS_ICON_W:
        {
            if (hWnd != HWND_NULL) {
                win_info = GetWindowInfo(hWnd);

                if (!IS_CAPTION_VISIBLE (win_info))
                    return 0;

                cap_h = 
                    calc_we_metrics (hWnd, style_info, LFRDR_METRICS_CAPTION_H);
                if (win_info->hIcon)
                    GetIconSize (win_info->hIcon, &icon_w, &icon_h);
                else {
                    icon_w = 0;
                    icon_h = 0;
                }

                return  (icon_h<cap_h)? icon_w : (icon_w*cap_h/icon_h);
            }
            else {
                return 16;
            }
        }
            
        case LFRDR_METRICS_VSCROLL_W:
        {
            int _idx;
            if (style_info) {
                if (!(style_info->dwStyle & WS_VSCROLL)) {
                    return 0;
                }
                _idx = WE_METRICS_SCROLLBAR & WE_ATTR_INDEX_MASK;
                return wnd_rdr_fashion.we_metrics[_idx];
            }
            else if (hWnd != HWND_NULL) {
                win_info = GetWindowInfo(hWnd);
                if (!IS_VSCROLL_VISIBLE (win_info))
                    return 0;
            }
            return GetWindowElementAttr (hWnd, WE_METRICS_SCROLLBAR);
        }

        case LFRDR_METRICS_HSCROLL_H:
        {
            int _idx;
            if (style_info) {
                if (!(style_info->dwStyle & WS_HSCROLL)) {
                    return 0;
                }
                _idx = WE_METRICS_SCROLLBAR & WE_ATTR_INDEX_MASK;
                return wnd_rdr_fashion.we_metrics[_idx];
            }
            else if (hWnd != HWND_NULL) {
                win_info = GetWindowInfo(hWnd);
                if (!IS_HSCROLL_VISIBLE (win_info))
                    return 0;
            }
            return GetWindowElementAttr (hWnd, WE_METRICS_SCROLLBAR);
        }

        case LFRDR_METRICS_MINWIN_WIDTH:
        {
            int _style, _win_type;

            win_width = 
                (calc_we_metrics (hWnd, style_info, LFRDR_METRICS_BORDER)<< 1);

            if (style_info) {
                _style = style_info->dwStyle;
                _win_type = style_info->winType;
            }
            else if (hWnd == HWND_NULL) {
                    return win_width;
            }
            else {
                win_info = GetWindowInfo (hWnd);

                _style = win_info->dwStyle;
                _win_type = lf_get_win_type(hWnd);
            }

            if (!(_style & WS_CAPTION))
                return win_width;

            cap_h = 
                calc_we_metrics (hWnd, style_info, LFRDR_METRICS_CAPTION_H);

            if (_win_type==LFRDR_WINTYPE_MAINWIN) {
                icon_w = 
                    calc_we_metrics (hWnd, style_info, LFRDR_METRICS_ICON_W);
                icon_h = 
                    calc_we_metrics (hWnd, style_info, LFRDR_METRICS_ICON_H);
            }

            win_width += CAP_BTN_INTERVAL;

            win_width += icon_w ? (icon_w + CAP_BTN_INTERVAL) : 0;

            cap_font = (PLOGFONT)GetWindowElementAttr (hWnd, WE_FONT_CAPTION);

            win_width += (cap_font) ? 
                         ((cap_font->size << 1) + CAP_BTN_INTERVAL) : 0;

            /*two char and internal*/
            btn_w = GetWindowElementAttr(hWnd, WE_METRICS_CAPTION)
                    - CAP_BTN_INTERVAL;
            
            /*buttons and internvals*/
            win_width += btn_w; 

            if (_style & WS_MINIMIZEBOX)
                win_width += btn_w; 
            if (_style & WS_MAXIMIZEBOX)
                win_width += btn_w; 

            return win_width;
        }

        case LFRDR_METRICS_MINWIN_HEIGHT:
        {
            win_height =
                (calc_we_metrics (hWnd, style_info, LFRDR_METRICS_BORDER)<< 1);
            win_height += 
                (calc_we_metrics (hWnd, style_info, LFRDR_METRICS_CAPTION_H)<< 1);

            return win_height;
        }
    }
    return 0;
}

static int calc_we_area (HWND hWnd, int which, RECT* we_area)
{
    FUNCTION_SCOPE_TIMING();
    int border;
    int cap_h;
    int menu_h;
    const WINDOWINFO* win_info = NULL;

    int win_w = 0;
    int win_h = 0;

    win_info = GetWindowInfo (hWnd);
    win_w = win_info->right - win_info->left;
    win_h = win_info->bottom - win_info->top;

    switch (which) {
        case HT_BORDER:
            CHECKNOT_RET_ERR(IS_BORDER_VISIBLE (win_info));
            we_area->left = 0;
            we_area->right = win_w;
            we_area->top = 0;
            we_area->bottom = win_h;
            return 0;

        case HT_CAPTION:
            CHECKNOT_RET_ERR (IS_CAPTION_VISIBLE (win_info));
            border = get_window_border (hWnd, 0, 0);
            cap_h = get_window_caption(hWnd);
            we_area->left = border;
            we_area->right = win_w - border;
            we_area->top = border;
            we_area->bottom = we_area->top + cap_h; 
            return 0;

        case HT_CLOSEBUTTON:            
        case HT_MAXBUTTON:
        case HT_MINBUTTON:
        case HT_ICON:
            CHECKNOT_RET_ERR (IS_CAPTION_VISIBLE (win_info));
            return calc_capbtn_area(hWnd, which, we_area);

        case HT_MENUBAR:
            CHECKNOT_RET_ERR (IS_MENUBAR_VISIBLE (win_info));
            border = get_window_border (hWnd, 0, 0);
            cap_h = get_window_caption (hWnd);
            menu_h = get_window_menubar(hWnd);

            we_area->left = border;
            we_area->top = border + cap_h;
            we_area->right = win_w - border; 
            we_area->bottom = we_area->top + menu_h;
            return 0;

        case HT_CLIENT:
            border = get_window_border (hWnd, 0, 0);
            cap_h = get_window_caption (hWnd);
            menu_h = get_window_menubar (hWnd);

            we_area->top = border + cap_h + menu_h;
            we_area->bottom = win_h - border - 
                get_window_scrollbar(hWnd, FALSE);

            if (IS_LEFT_VSCOLLBAR (win_info)) {
                we_area->left = border +
                    get_window_scrollbar(hWnd, TRUE);
                we_area->right = win_w - border;
            }
            else
            {
                we_area->left = border;
                we_area->right = win_w - border -
                    get_window_scrollbar(hWnd, TRUE);
            }
            return 0;

        case HT_HSCROLL:
            CHECKNOT_RET_ERR (IS_HSCROLL_VISIBLE (win_info));
            return calc_hscroll_area (hWnd, which, we_area);

        case HT_VSCROLL:
            CHECKNOT_RET_ERR (IS_VSCROLL_VISIBLE (win_info));
            return calc_vscroll_area (hWnd, which, we_area);

        default:
            if (which & HT_SB_MASK) {
                if (which < HT_SB_UPARROW) {
                    CHECKNOT_RET_ERR (IS_HSCROLL_VISIBLE (win_info));
                    return calc_hscroll_area(hWnd, which, we_area);
                }
                else {
                    CHECKNOT_RET_ERR (IS_VSCROLL_VISIBLE (win_info));
                    return calc_vscroll_area(hWnd, which, we_area);
                }
            }
            else
                return -1;
    }
}

static void 
calc_thumb_area (HWND hWnd, BOOL vertical, LFSCROLLBARINFO* sb_info)
{
    FUNCTION_SCOPE_TIMING();
    RECT rc;
    int move_range;
    div_t divt;
    
    if (vertical) {
        *sb_info = GetWindowInfo(hWnd)->vscroll;
        calc_vscroll_area(hWnd, HT_VSCROLL, &rc);
        sb_info->arrowLen  = RECTW (rc);
        move_range = RECTH (rc) - (sb_info->arrowLen << 1);
    }
    else {
        *sb_info = GetWindowInfo(hWnd)->hscroll;
        calc_hscroll_area(hWnd, HT_HSCROLL, &rc);
        sb_info->arrowLen  = RECTH (rc);
        move_range = RECTW (rc) - (sb_info->arrowLen << 1);
    }

    if (move_range < 0)
        move_range = 0;

    divt = div (move_range, sb_info->maxPos - sb_info->minPos + 1);
    sb_info->barLen = sb_info->pageStep * divt.quot +
        sb_info->pageStep * divt.rem / 
        (sb_info->maxPos - sb_info->minPos + 1);

    if ((sb_info->barLen || sb_info->pageStep != 0) &&
            sb_info->barLen < LFRDR_SB_MINBARLEN)
        sb_info->barLen = LFRDR_SB_MINBARLEN;

    if (sb_info->minPos == sb_info->maxPos)
    {
        sb_info->barStart = 0;
        sb_info->barLen   = move_range;
    }
    else
    {
        divt = div (move_range, sb_info->maxPos - sb_info->minPos + 1);
        sb_info->barStart = (int) ( 
            (sb_info->curPos - sb_info->minPos) * divt.quot + 
            (sb_info->curPos - sb_info->minPos) * divt.rem /
            (sb_info->maxPos - sb_info->minPos + 1) + 0.5);

        if (sb_info->barStart + sb_info->barLen > move_range)
            sb_info->barStart = move_range - sb_info->barLen;
        if (sb_info->barStart < 0)
            sb_info->barStart = 0;
    }
}

static int find_interval(int* array, int len, int val)
{
    int i;
    for (i=0; i<len-1; i++)
    {
        if (array[i]<=val && val<array[i+1])
            break;
    }

    if (i == len-1)
        return -1;
    else
        return i;
}

/*
 * test_caption:
 *     test mouse in which part of caption 
 * Author: XuguangWang
 * Date: 2007-11-22
 */
static int test_caption(HWND hWnd, int x, int y)
{
    FUNCTION_SCOPE_TIMING();
    RECT rc;
    if (calc_we_area(hWnd, HT_ICON, &rc) == 0)
        CHECK_RET_VAL(PtInRect(&rc, x, y), HT_ICON);

    if (calc_we_area(hWnd, HT_MINBUTTON, &rc) == 0)
        CHECK_RET_VAL(PtInRect(&rc, x, y), HT_MINBUTTON);

    if (calc_we_area(hWnd, HT_MAXBUTTON, &rc) == 0)
        CHECK_RET_VAL(PtInRect(&rc, x, y), HT_MAXBUTTON);

    if (calc_we_area(hWnd, HT_CLOSEBUTTON, &rc) == 0)
        CHECK_RET_VAL(PtInRect(&rc, x, y), HT_CLOSEBUTTON);

    return HT_CAPTION;
}

/*
 * test_scroll:
 *     test mouse in which part of the scrollbar indicated by is_vertival. 
 * Author: XuguangWang
 * Date: 2007-11-22
 */
static int test_scroll(const LFSCROLLBARINFO* sb_info, 
        int left, int right, int x, BOOL is_vertival)
{
    FUNCTION_SCOPE_TIMING();
    static int x_poses[5] = {
        HT_SB_LEFTARROW, HT_SB_LEFTSPACE, HT_SB_HTHUMB,
        HT_SB_RIGHTSPACE, HT_SB_RIGHTARROW
    };

    int array[6];
    int x_pos;
    array[0] = left;
    array[1] = left+sb_info->arrowLen;
    array[2] = array[1] + sb_info->barStart;
    array[3] = array[2] + sb_info->barLen;
    array[5] = right;
    array[4] = array[5] - sb_info->arrowLen;

    x_pos = find_interval(array, 6, x);

    if (x_pos == -1)
        return HT_SB_UNKNOWN;
    else
        return x_poses[x_pos]|((is_vertival) ? HT_SB_VMASK : 0);
}

/*
 * hit_test:
 *      get which window element including parts of caption, scrollbars
 *      mouse in.
 * Author: XuguangWang
 * Date: 2007-11-22
 */
static int hit_test (HWND hWnd, int x, int y)
{
    FUNCTION_SCOPE_TIMING();
    static const int ht_inner_border [4] = {
        HT_CAPTION, HT_MENUBAR, HT_VSCROLL, HT_HSCROLL
    };

    static const int ht_on_border [3][3] = {
        {HT_CORNER_TL,   HT_BORDER_TOP,    HT_CORNER_TR},
        {HT_BORDER_LEFT, HT_UNKNOWN,       HT_BORDER_RIGHT},
        {HT_CORNER_BL,   HT_BORDER_BOTTOM, HT_CORNER_BR},
    };

    RECT rc;
    int border;
    int x_pos;
    int y_pos;
    int array[5];
    int tmp;
    int win_w;
    int win_h;

    const WINDOWINFO* win_info = GetWindowInfo(hWnd);
    win_w = win_info->right - win_info->left;
    win_h = win_info->bottom - win_info->top;

    /*mouse not in this window*/
    CHECK_RET_VAL(!PtInRect((RECT*)win_info, x, y), HT_OUT);

    /*mouse in client area*/
    CHECK_RET_VAL(PtInRect((RECT*)&(win_info->cl), x, y), HT_CLIENT);

    /*change x y to hwnd coordiante*/
    x -= win_info->left;
    y -= win_info->top;

    border = get_window_border (hWnd, 0, 0);
    SetRect(&rc, border, border, win_w-border, win_h-border);

    /*mouse on border*/
    if (!PtInRect(&rc, x, y)) {
        if (!(win_info->dwStyle & WS_CAPTION) || (win_info->dwStyle & WS_DLGFRAME))
            return HT_UNKNOWN;

        //tmp = GetWindowElementAttr(hWnd, WE_METRICS_CAPTION);
        tmp = get_window_caption(hWnd);
        array[0] = 0;
        array[1] = array[0] + tmp;
        array[3] = win_w;
        array[2] = array[3] - tmp;
        x_pos = find_interval(array, 4, x);

        array[0] = 0;
        array[1] = array[0] + tmp;
        array[3] = win_h;
        array[2] = array[3] - tmp;
        y_pos = find_interval(array, 4, y);    

        if (x_pos!=-1 && y_pos!=-1)
            return ht_on_border[y_pos][x_pos];
        else
            return HT_UNKNOWN;
    }
    /*mouse inner border*/
    else {
        array[2] = win_info->ct - win_info->top;
        array[1] = array[2] - get_window_menubar(hWnd);
        array[0] = array[1] - get_window_caption(hWnd);
        array[3] = win_info->cb - win_info->top;
        array[4] = array[3] + get_window_scrollbar(hWnd, FALSE);
        y_pos = find_interval(array, 5, y);    
        if (y_pos != -1)
            switch (ht_inner_border[y_pos]) {
                case HT_CAPTION:
                    return test_caption(hWnd, x, y);

                case HT_MENUBAR:
                    return HT_MENUBAR;

                case HT_HSCROLL:
                    return test_scroll(&(win_info->hscroll),
                           win_info->cl - win_info->left, 
                           win_info->cr - win_info->left, 
                           x, FALSE);

                case HT_VSCROLL:
                    return test_scroll(&(win_info->vscroll), 
                            win_info->ct - win_info->top, 
                            win_info->cb - win_info->top, 
                            y, TRUE);
                default:
                    return HT_UNKNOWN;
            }
        else
            return HT_UNKNOWN;
    }

}

/* draw_border:
 *   This function draw the border of a window. 
 *
 * \param hWnd : the handle of the window.
 * \param hdc : the DC of the window.
 * \param is_active : whether the window is actived.
 * \return : 0 for succeed, other for failure. 
 *
 * Author: wangjian<wangjian@minigui.org>
 * Date: 2007-12-13
 */
static void draw_border (HWND hWnd, HDC hdc, BOOL is_active)
{
    FUNCTION_SCOPE_TIMING();
    int i , border;
    const WINDOWINFO *win_info = NULL;
    RECT rect;
    DWORD c1, c2;
    int l, t, r, b;
    gal_pixel old_pen_color;
    if(calc_we_area (hWnd, HT_BORDER, &rect) == -1)
        return;
    
    l = rect.left;
    t = rect.top;
    r = rect.right-1;
    b = rect.bottom-1;
    
    if(is_active) {
        c1 = GetWindowElementAttr (hWnd, WE_FGC_ACTIVE_WND_BORDER);
    } else {
        c1 = GetWindowElementAttr (hWnd, WE_FGC_INACTIVE_WND_BORDER);
    }
    c2 = c1;
    
    border = (int)GetWindowElementAttr (hWnd, WE_METRICS_WND_BORDER);
    win_info = GetWindowInfo (hWnd);
    old_pen_color = GetPenColor (hdc);

    if (IsMainWindow (hWnd)) // for main window
    {
        if (IsDialog(hWnd))   // for dialog
            border = 0;    
        if(win_info->dwStyle & WS_BORDER
                || win_info->dwStyle & WS_THICKFRAME)
        {
            /** left and top border */
            SetPenColor (hdc, DWORD2Pixel (hdc, c1));
            for(i = 0; i < 4 && i < border + 3; i++)
            {
                c2 = gradient_color (c2, LFRDR_3DBOX_COLOR_LIGHTER, 30);
                SetPenColor (hdc, DWORD2Pixel (hdc, c2));
                MoveTo (hdc, l+i, b-i);
                LineTo (hdc, l+i, t+i);
                LineTo (hdc, r+i - 1, t+i);
            }

            c2 = gradient_color (c2, LFRDR_3DBOX_COLOR_LIGHTER, 30);
            SetPenColor (hdc, DWORD2Pixel (hdc, c2));
            for(;i < border+3; i++)
            {
                MoveTo (hdc, l+i, b-i);
                LineTo (hdc, l+i, t+i);
                LineTo (hdc, r+i - 1, t+i);
            }

            /** right and bottom border */
            c2 = gradient_color (c1, LFRDR_3DBOX_COLOR_DARKER, 80);
            for(i = 0; i < 3 && i < border + 3; i++)
            {
                c2 = gradient_color (c2, LFRDR_3DBOX_COLOR_LIGHTER, 80);
                SetPenColor (hdc, DWORD2Pixel (hdc, c2));
                MoveTo (hdc, l+i + 1, b-i);
                LineTo (hdc, r-i, b-i);
                LineTo (hdc, r-i, t+i);
            }

            c2 = gradient_color (c2, LFRDR_3DBOX_COLOR_LIGHTER, 80);
            SetPenColor (hdc, DWORD2Pixel (hdc, c2));
            for(;i < border+3; i++)
            {
                MoveTo (hdc, l+i + 1, b-i);
                LineTo (hdc, r-i, b-i);
                LineTo (hdc, r-i, t+i);
            }
        }
        else if(win_info->dwStyle & WS_THINFRAME)
        {
            SetPenColor (hdc, DWORD2Pixel (hdc, c1));
            for(i = 0; i < border; i++)
            {
                Rectangle (hdc, rect.left+i, rect.top+i,
                        rect.right-i-1, rect.bottom-i-1);
            }
        }
    }
    else if(IsControl(hWnd))   // for control
    {
        if(win_info->dwStyle & WS_BORDER
                || win_info->dwStyle & WS_THICKFRAME
                || win_info->dwStyle & WS_THINFRAME)    
        {
            SetPenColor (hdc, DWORD2Pixel (hdc, c2));
            Rectangle (hdc, rect.left, rect.top,
                    rect.right-1, rect.bottom-1);
        }
        
    }

    SetPenColor(hdc, old_pen_color);
}

/* draw_caption:
 *   This function draw the caption of a window. 
 *
 * \param hWnd : the handle of the window.
 * \param hdc : the DC of the window.
 * \param is_active : whether the window is actived.
 *
 * Author: wangjian<wangjian@minigui.org>
 * Date: 2007-12-13
 */
#define ICON_ORIGIN 2
static void draw_caption (HWND hWnd, HDC hdc, BOOL is_active)
{
    FUNCTION_SCOPE_TIMING();
    int     font_h;
    gal_pixel text_color, old_text_color;
    PLOGFONT cap_font, old_font;
    const WINDOWINFO *win_info;
    RECT    rect, del_rect, icon_rect, rcTmp;
    int     win_w;
    int     border;
    int  ncbutton_w = 0;

    DWORD   ca = 0, cb = 0;
    RECT    rect_line;
    gal_pixel pixels[2];

    HPATH   path = 0;
    HBRUSH  brush = 0;

    SetRectEmpty (&icon_rect);
    win_info = GetWindowInfo(hWnd);

    if (!(win_info->dwStyle & WS_CAPTION))
        return;
    if(calc_we_area(hWnd, HT_CAPTION, &rect) == -1)
        return;
    if(RECTH(rect) <= 1 || RECTW(rect) <= 0)
        return;

    cap_font = (PLOGFONT)GetWindowElementAttr(hWnd, WE_FONT_CAPTION);

    HGRAPHICS graphic = MGPlusGraphicCreate (RECTW (rect), RECTH (rect));
    if (!graphic)
        return;

    HDC hdc_graphic = MGPlusGetGraphicDC (graphic);

    if(is_active) {
        text_color = GetWindowElementPixelEx (hWnd, hdc_graphic, WE_FGC_ACTIVE_CAPTION);
        ca = GetWindowElementAttr (hWnd, WE_BGCA_ACTIVE_CAPTION);
        cb = GetWindowElementAttr (hWnd, WE_BGCB_ACTIVE_CAPTION);
    } else {
        text_color = GetWindowElementPixelEx (hWnd, hdc_graphic, WE_FGC_INACTIVE_CAPTION);
        ca = GetWindowElementAttr (hWnd, WE_BGCA_INACTIVE_CAPTION);
        cb = GetWindowElementAttr (hWnd, WE_BGCB_INACTIVE_CAPTION);
    }

    BitBlt (hdc, rect.left, rect.top, RECTW (rect), RECTH (rect), hdc_graphic, 0, 0, 0);
    /** draw backgroup right */
    pixels[0] = mp_gradient_color(ca, LFRDR_3DBOX_COLOR_LIGHTER, 180);
    pixels[1] = MPMakeARGB (GetRValue(cb), GetGValue(cb), GetBValue(cb), GetAValue(cb));
    brush = MGPlusBrushCreate(MP_BRUSH_TYPE_LINEARGRADIENT); 

    if (!brush){
        MGPlusGraphicDelete(graphic);
        return;
    }
    path = MGPlusPathCreate(MP_PATH_FILL_MODE_WINDING);
    if (!path) {
        MGPlusGraphicDelete(graphic);
        MGPlusBrushDelete(brush);
        return;
    }

    RECT tmp_rect;

    tmp_rect.left = tmp_rect.top = 0;
    tmp_rect.right = RECTW (rect);
    tmp_rect.bottom = RECTH (rect);

    MGPlusSetLinearGradientBrushMode(brush, MP_LINEAR_GRADIENT_MODE_VERTICAL);
    MGPlusSetLinearGradientBrushRect(brush, &tmp_rect);
    MGPlusSetLinearGradientBrushColors(brush, (ARGB*)pixels, 2); 

    MGPlusPathAddRectangleI(path, 0, 0, RECTW (tmp_rect), RECTH (tmp_rect));
    MGPlusFillPath(graphic, brush, path); 
    MGPlusPathReset (path);

    /** draw backgroup left */
    memset (&rcTmp, 0, sizeof (RECT));
    if (win_info->dwStyle & WS_MINIMIZEBOX) {
        calc_we_area(hWnd, HT_MINBUTTON, &rcTmp);
    }
    else if (win_info->dwStyle & WS_MAXIMIZEBOX) {
        calc_we_area(hWnd, HT_MAXBUTTON, &rcTmp);
    }
    else {
        calc_we_area(hWnd, HT_CLOSEBUTTON, &rcTmp);
    }

    pixels[0] = mp_gradient_color(ca, LFRDR_3DBOX_COLOR_LIGHTER, 80);
    pixels[1] = mp_gradient_color(cb, LFRDR_3DBOX_COLOR_DARKER, 60);

    MGPlusSetLinearGradientBrushMode(brush, MP_LINEAR_GRADIENT_MODE_VERTICAL);
    MGPlusSetLinearGradientBrushRect(brush, &tmp_rect);
    MGPlusSetLinearGradientBrushColors(brush, (ARGB*)pixels, 2); 

    MGPlusPathAddRectangleI(path, 0, 0, RECTW (tmp_rect), RECTH (tmp_rect));
    MGPlusFillPath(graphic, brush, path); 
    MGPlusPathReset (path);

    /** draw backgroup 3 */

    /** draw the first dark line */
    pixels[0] = mp_gradient_color(cb, LFRDR_3DBOX_COLOR_LIGHTER, 40);
    pixels[1] = mp_gradient_color(ca, LFRDR_3DBOX_COLOR_LIGHTER, 65);

    RECT rc_tmp_up;
    rc_tmp_up.left = rect_line.left = tmp_rect.left;
    rc_tmp_up.top = rect_line.top = tmp_rect.top + 2;
    rc_tmp_up.right = rect_line.right = rect_line.left + (rcTmp.left - rect.left) - 10 - 
        (tmp_rect.bottom - tmp_rect.top) - 10;
    rc_tmp_up.bottom = rect_line.bottom = rect_line.top + 1;

    rect_line.left = rect_line.right - 10;

    MGPlusSetLinearGradientBrushMode(brush, MP_LINEAR_GRADIENT_MODE_HORIZONTAL);
    MGPlusSetLinearGradientBrushRect(brush, &rect_line);
    MGPlusSetLinearGradientBrushColors(brush, (ARGB*)pixels, 2); 

    MGPlusPathAddRectangleI(path, rect_line.left, rect_line.top,
            RECTW (rect_line), RECTH (rect_line));
    MGPlusFillPath(graphic, brush, path); 
    MGPlusPathReset(path);

    /* draw the second dark line */
    RECT rc_tmp_down;
    rc_tmp_down.left = rect_line.left = tmp_rect.left;
    rc_tmp_down.top = rect_line.top = tmp_rect.bottom - 4;
    rc_tmp_down.bottom = rect_line.bottom = rect_line.top + 1;
    rc_tmp_down.right = rect_line.right = rect_line.left + (rcTmp.left - rect.left) - 10 - 20;

    pixels [0] = mp_gradient_color (ca, LFRDR_3DBOX_COLOR_DARKER, 100);
    pixels [1] = mp_gradient_color (cb, LFRDR_3DBOX_COLOR_DARKER, 50);

    rect_line.left = rect_line.right - 10;

    MGPlusSetLinearGradientBrushRect (brush, &rect_line);
    MGPlusSetLinearGradientBrushColors (brush, (ARGB*)pixels, 2); 

    MGPlusPathAddRectangleI (path, rect_line.left, rect_line.top,
            RECTW (rect_line), RECTH (rect_line));
    MGPlusFillPath (graphic, brush, path); 
    MGPlusPathReset (path);

    /** draw the third light line */
    pixels [0] = mp_gradient_color (cb, LFRDR_3DBOX_COLOR_LIGHTER, 100);
    pixels [1] = mp_gradient_color (ca, LFRDR_3DBOX_COLOR_LIGHTER, 70);

    RECT rc_tmp_up_small;

    rc_tmp_up_small.left = rect_line.left = tmp_rect.left;
    rc_tmp_up_small.top = rect_line.top = tmp_rect.top + 3;
    rc_tmp_up_small.right = rect_line.right = rect_line.left + (rcTmp.left - rect.left) - 10 - 
        (rect.bottom - rect.top) - 10;
    rc_tmp_up_small.bottom = rect_line.bottom = rect_line.top + 1;

    rect_line.left = rect_line.right - 10;

    MGPlusSetLinearGradientBrushRect (brush, &rect_line);
    MGPlusSetLinearGradientBrushColors (brush, (ARGB*)pixels, 2); 

    MGPlusPathAddRectangleI (path, rect_line.left, rect_line.top,
            RECTW (rect_line), RECTH (rect_line));
    MGPlusFillPath (graphic, brush, path); 
    MGPlusPathReset (path);

    /** draw the fourth light line */
    RECT rc_down_small;
    rc_down_small.left = rect_line.left = tmp_rect.left;
    rc_down_small.top = rect_line.top = tmp_rect.bottom - 3;
    rc_down_small.bottom = rect_line.bottom = rect_line.top + 1;
    rc_down_small.right = rect_line.right = rect_line.left + (rcTmp.left - rect.left) - 10 - 20;
    /** FIXE ME */
    pixels[0] = mp_gradient_color(cb, LFRDR_3DBOX_COLOR_LIGHTER, 40);
    pixels[1] = mp_gradient_color(ca, LFRDR_3DBOX_COLOR_DARKER, 55);

    rect_line.left = rect_line.right - 10;

    MGPlusSetLinearGradientBrushRect (brush, &rect_line);
    MGPlusSetLinearGradientBrushColors (brush, (ARGB*)pixels, 2); 

    MGPlusPathAddRectangleI (path, rect_line.left, rect_line.top, \
            RECTW (rect_line), RECTH (rect_line));
    MGPlusFillPath (graphic, brush, path); 
    MGPlusPathReset (path);

    SetPenColor (hdc_graphic, 
            DWORD2Pixel (hdc_graphic, 
                gradient_color(cb, LFRDR_3DBOX_COLOR_LIGHTER, 40)));
    MoveTo (hdc_graphic, rc_tmp_up.left, rc_tmp_up.top);
    LineTo (hdc_graphic, rc_tmp_up.right - 10, rc_tmp_up.top);
    SetPenColor (hdc_graphic, 
            DWORD2Pixel (hdc_graphic, 
                gradient_color(cb, LFRDR_3DBOX_COLOR_DARKER, 100)));
    MoveTo (hdc_graphic, rc_tmp_down.left, rc_tmp_down.top);
    LineTo (hdc_graphic, rc_tmp_down.right - 10, rc_tmp_down.top);

    SetPenColor (hdc_graphic, 
            DWORD2Pixel (hdc_graphic, 
                gradient_color(cb, LFRDR_3DBOX_COLOR_LIGHTER, 100)));
    MoveTo (hdc_graphic, rc_tmp_up_small.left, rc_tmp_up_small.top);
    LineTo (hdc_graphic, rc_tmp_up_small.right - 10, rc_tmp_up_small.top);
    SetPenColor (hdc_graphic, 
            DWORD2Pixel (hdc_graphic, 
                gradient_color(cb, LFRDR_3DBOX_COLOR_LIGHTER, 40)));
    MoveTo (hdc_graphic, rc_down_small.left, rc_down_small.top);
    LineTo (hdc_graphic, rc_down_small.right - 10, rc_down_small.top);

    /** draw icon */
    if(win_info->hIcon)
    {
        if(calc_we_area(hWnd, HT_ICON, &icon_rect) != -1)
            DrawIcon(hdc_graphic, (icon_rect.left - rect.left), (icon_rect.top - rect.top),
                    RECTW(icon_rect), RECTH(icon_rect), win_info->hIcon);
    }


    /** draw caption title */
    if(win_info->spCaption)
    {
        border = get_window_border (hWnd, win_info->dwStyle, 
                lf_get_win_type(hWnd));
        win_w = win_info->right - win_info->left;

        if (calc_we_area(hWnd, HT_MINBUTTON, &del_rect) != -1){

        }
        else if(calc_we_area(hWnd, HT_CLOSEBUTTON, &del_rect) != -1){

        }
        else if(calc_we_area(hWnd, HT_MAXBUTTON, &del_rect) != -1){

        }
        else 
        {
            del_rect.left = win_w - border;
        }
        del_rect.right = win_w;
        del_rect.left -= CAP_BTN_INTERVAL;

        del_rect.top = border;
        del_rect.bottom = border + 
            GetWindowElementAttr (hWnd, WE_METRICS_CAPTION);

        ExcludeClipRect (hdc_graphic, &del_rect);
        SetBkMode(hdc_graphic, BM_TRANSPARENT);
        old_font = SelectFont (hdc_graphic, cap_font);
        font_h = GetFontHeight (hdc_graphic);
        old_text_color = SetTextColor (hdc_graphic,text_color);

        if ((win_info->dwExStyle & WS_EX_NOCLOSEBOX)) {
            calc_we_area(hWnd, HT_CLOSEBUTTON, &rcTmp);
            ncbutton_w += RECTW(rcTmp);
        }
        if (win_info->dwStyle & WS_MAXIMIZEBOX) {
            calc_we_area(hWnd, HT_MAXBUTTON, &rcTmp);
            ncbutton_w += RECTW(rcTmp);
        }

        if (win_info->dwStyle & WS_MINIMIZEBOX) {
            calc_we_area(hWnd, HT_MINBUTTON, &rcTmp);
            ncbutton_w += RECTW(rcTmp);
        }

        TextOutOmitted (hdc_graphic, rect.left + RECTW(icon_rect) + 
                (ICON_ORIGIN << 1),
                rect.top + ((RECTH(rect)-font_h)>>1),
                win_info->spCaption,
                strlen (win_info->spCaption),  
                (RECTW(rect) - RECTW(icon_rect) - (ICON_ORIGIN << 1) - ncbutton_w));

        SetTextColor(hdc_graphic, old_text_color);
        SelectFont(hdc_graphic, old_font);
        IncludeClipRect (hdc_graphic, &del_rect);
    }

    MGPlusGraphicSave (graphic, hdc, 0, 0, 0, 0, rect.left, rect.top);

    if (path) {
        MGPlusPathDelete (path);
    }
    if (brush) {
        MGPlusBrushDelete (brush);
    }
    MGPlusGraphicDelete (graphic);
}

/*
 * This function make a alpha color, alpha = 0.5
 * used by the funtion draw_caption_button.
 * Author: wangjian<wangjian@minigui.org>
 * Date: 2007-12-13.
 */
static DWORD calc_alpha(DWORD c1, DWORD c2)
{
    FUNCTION_SCOPE_TIMING();
    BYTE r,g,b;
    BYTE r1,g1,b1;
    BYTE r2,g2,b2;

    r1 = GetRValue(c1);
    r2 = GetRValue(c2);
    g1 = GetGValue(c1);
    g2 = GetGValue(c2);
    b1 = GetBValue(c1);
    b2 = GetBValue(c2);

    r = (r1 + r2) >> 1;
    g = (g1 + g2) >> 1;
    b = (b1 + b2) >> 1;
    
    return MakeRGB(r,g,b);

}

/* draw_caption_button:
 *   This function draw the caption button of a window. 
 *
 * \param hWnd : the handle of the window.
 * \param hdc : the DC of the window.
 * \param ht_code : the number for close, max, or min button, 0 for all.
 * \param status : the status of the button drawing.
 *
 * Author: wangjian<wangjian@minigui.org>
 * Date: 2007-12-15
 */
static void 
draw_caption_button (HWND hWnd, HDC hdc, int ht_code, int status)
{
    FUNCTION_SCOPE_TIMING();

    int i, w, h;
    RECT rect;
    DWORD cap_c, alp_c;
        
    gal_pixel pixels[3];
    RECT rc_close;
    gal_pixel fgc_3d;
    int cx, cy, r;
    int save_left, save_top;

    HPATH path;

    path = MGPlusPathCreate(MP_PATH_FILL_MODE_WINDING);
    if (!path) {
        return;
    }
    ARGB pen_color = 0xFF000000;
    HPEN pen = MGPlusPenCreate (1, pen_color);
    if (!pen) {
        MGPlusPathDelete(path);
        return;
    }

    fgc_3d = GetWindowElementPixelEx(hWnd, hdc, WE_FGC_THREED_BODY);

#if 0
    if (GetActiveWindow () == hWnd)
        cap_c = GetWindowElementAttr (hWnd, WE_FGC_ACTIVE_CAPTION);
    else
        cap_c = GetWindowElementAttr (hWnd, WE_FGC_INACTIVE_CAPTION);
#else
    if ((status & LFRDR_BTN_STATUS_INACTIVE) == LFRDR_BTN_STATUS_INACTIVE)
        cap_c = GetWindowElementAttr (hWnd, WE_FGC_INACTIVE_CAPTION);
    else
        cap_c = GetWindowElementAttr (hWnd, WE_FGC_ACTIVE_CAPTION);
#endif

    switch(ht_code)
    {
        case 0:                     // draw all the 3 buttons as normal.
            status = LFRDR_BTN_STATUS_NORMAL;
        case HT_CLOSEBUTTON:
            if (calc_we_area (hWnd, HT_CLOSEBUTTON, &rect) != -1)
            {
                if(RECTH(rect) <= 0 || RECTW(rect) <= 0)
                {
                    MGPlusPenDelete (pen);
                    MGPlusPathDelete (path);
                    return;
                }
                if(status & LFRDR_BTN_STATUS_PRESSED){
                    pixels[1] = mp_gradient_color (cap_c, LFRDR_3DBOX_COLOR_LIGHTER, 50);
                    alp_c = calc_alpha (cap_c, gradient_color (cap_c, LFRDR_3DBOX_COLOR_LIGHTER, 50));
                }
                else if(status & LFRDR_BTN_STATUS_HILITE){
                    pixels [1] = mp_gradient_color (cap_c, LFRDR_3DBOX_COLOR_LIGHTER, 80);
                    alp_c = calc_alpha (cap_c, gradient_color (cap_c, LFRDR_3DBOX_COLOR_LIGHTER, 80));
                }
                else {
                    pixels [1] = mp_gradient_color (cap_c, LFRDR_3DBOX_COLOR_LIGHTER, 10);
                    alp_c = calc_alpha (cap_c, gradient_color (cap_c, LFRDR_3DBOX_COLOR_LIGHTER, 10));
                }

                rc_close.left = 0;
                rc_close.top = 0;
                rc_close.right = RECTW (rect);
                rc_close.bottom = RECTH (rect);
                save_left = rect.left;
                save_top = rect.top;

                pixels [0] = mp_gradient_color (cap_c, LFRDR_3DBOX_COLOR_LIGHTER, 250);


                cx = (rc_close.left + rc_close.right) >> 1;
                cy = (rc_close.top + rc_close.bottom) >> 1;
                r = (RECTH (rc_close) >> 1) - 1;

                MPPOINT pt = {(float)cx, (float)cy};
                RECT rc={cx-r, cy-r, cx+r, cy+r};

                HGRAPHICS graphic = MGPlusGraphicCreate (RECTW (rect), RECTH (rect));
                if (!graphic){
                    MGPlusPenDelete(pen);
                    MGPlusPathDelete(path);
                    return;
                }
                BitBlt (hdc, rect.left, rect.top, RECTW (rect), RECTH (rect),
                        MGPlusGetGraphicDC (graphic), 0, 0, 0);

                HBRUSH brush = MGPlusBrushCreate(MP_BRUSH_TYPE_PATHGRADIENT); 
                if (!brush){
                    MGPlusGraphicDelete(graphic);
                    MGPlusPenDelete(pen);
                    MGPlusPathDelete(path);
                    return;
                }
                MGPlusPathReset (path);
                MGPlusSetPathGradientBrushSurroundRect (brush, &rc);
                MGPlusSetPathGradientBrushCenterPoint(brush, &pt);
                MGPlusSetPathGradientBrushCenterColor (brush, pixels[0]);
                pixels[2] = pixels[1];
                MGPlusSetPathGradientBrushSurroundColors(brush, (ARGB *)&pixels[1], 2);

                MGPlusPathAddEllipseI(path, cx, cy, r, r, TRUE);

                MGPlusFillPath(graphic, brush, path); 

                pen_color = 0xFF000000 | alp_c;
                pen_color = MPMakeARGB (GetRValue (pen_color), GetGValue (pen_color),
                        GetBValue (pen_color), GetAValue (pen_color));
                MGPlusPenSetColor (pen, pen_color);

                MGPlusDrawPath (graphic, pen, path); 

                SetPenColor(hdc, fgc_3d);
                
                rect.right --; rect.bottom --;
                
                w = RECTW(rect);
                h = RECTH(rect);

                rect.left   += (w>>2)+1;
                rect.top    += (h>>2)+1;
                rect.right  -= (w>>2)+1;
                rect.bottom -= (h>>2)+1;

                MGPlusGraphicSave (graphic, hdc, 0, 0, 0, 0, save_left, save_top);

                for(i = 0; i < (w>>4)+2; i++) // the pen width : w/4-1
                {
                    MoveTo(hdc, rect.left, rect.top + i);
                    LineTo(hdc, rect.right - i, rect.bottom);
                    MoveTo(hdc, rect.left + i, rect.bottom);
                    LineTo(hdc, rect.right, rect.top + i);
                    
                    MoveTo(hdc, rect.left + i, rect.top);
                    LineTo(hdc, rect.right, rect.bottom - i);
                    MoveTo(hdc, rect.left, rect.bottom - i);
                    LineTo(hdc, rect.right - i, rect.top);
                }

                
                MGPlusBrushDelete (brush);
                MGPlusGraphicDelete (graphic);
            } 
            if (ht_code == HT_CLOSEBUTTON) break;

        case HT_MAXBUTTON:
            if (calc_we_area (hWnd, HT_MAXBUTTON, &rect) != -1)
            {
                if(RECTH(rect) <= 0 || RECTW(rect) <= 0)
                {
                    MGPlusPenDelete (pen);
                    MGPlusPathDelete (path);
                    return;
                }
                if(status & LFRDR_BTN_STATUS_PRESSED){
                    pixels[1] = mp_gradient_color (cap_c, LFRDR_3DBOX_COLOR_LIGHTER, 50);
                    alp_c = calc_alpha (cap_c, gradient_color (cap_c, LFRDR_3DBOX_COLOR_LIGHTER, 50));
                }
                else if(status & LFRDR_BTN_STATUS_HILITE){
                    pixels [1] = mp_gradient_color (cap_c, LFRDR_3DBOX_COLOR_LIGHTER, 80);
                    alp_c = calc_alpha (cap_c, gradient_color (cap_c, LFRDR_3DBOX_COLOR_LIGHTER, 80));
                }
                else {
                    pixels [1] = mp_gradient_color (cap_c, LFRDR_3DBOX_COLOR_LIGHTER, 10);
                    alp_c = calc_alpha (cap_c, gradient_color (cap_c, LFRDR_3DBOX_COLOR_LIGHTER, 10));
                }


                pixels [0] = mp_gradient_color (cap_c, LFRDR_3DBOX_COLOR_LIGHTER, 250);

                rc_close.left = 0;
                rc_close.top = 0;
                rc_close.right = RECTW (rect);
                rc_close.bottom = RECTH (rect);
                save_left = rect.left;
                save_top = rect.top;

                cx = (rc_close.left + rc_close.right) >> 1;
                cy = (rc_close.top + rc_close.bottom) >> 1;
                r = (RECTH (rc_close) >> 1) - 1;

                MPPOINT pt = {(float)cx, (float)cy};
                RECT rc={cx-r, cy-r, cx+r, cy+r};

                HGRAPHICS graphic = MGPlusGraphicCreate (RECTW (rect), RECTH (rect));
                if (!graphic){
                    MGPlusPenDelete(pen);
                    MGPlusPathDelete(path);
                    return;
                }
                BitBlt (hdc, rect.left, rect.top, RECTW (rect), RECTH (rect),
                        MGPlusGetGraphicDC (graphic), 0, 0, 0);

                HBRUSH brush = MGPlusBrushCreate(MP_BRUSH_TYPE_PATHGRADIENT); 
                if (!brush){
                    MGPlusGraphicDelete(graphic);
                    MGPlusPenDelete(pen);
                    MGPlusPathDelete(path);
                    return;
                }
                MGPlusPathReset (path);
                MGPlusSetPathGradientBrushSurroundRect (brush, &rc);
                MGPlusSetPathGradientBrushCenterPoint(brush, &pt);
                MGPlusSetPathGradientBrushCenterColor (brush, pixels[0]);
                pixels[2] = pixels[1];
                MGPlusSetPathGradientBrushSurroundColors(brush, (ARGB *)&pixels[1], 2);

                MGPlusPathAddEllipseI(path, cx, cy, r, r, TRUE);

                MGPlusFillPath(graphic, brush, path); 

                pen_color = 0xFF000000 | alp_c;
                pen_color = MPMakeARGB (GetRValue (pen_color), GetGValue (pen_color),
                        GetBValue (pen_color), GetAValue (pen_color));
                MGPlusPenSetColor (pen, pen_color);
                MGPlusDrawPath (graphic, pen, path); 
             
                SetPenColor(hdc, fgc_3d);
                
                rect.right --; rect.bottom --;
                
                w = RECTW(rect);
                h = RECTH(rect);

                rect.left   += (w>>2)+1;
                rect.top    += (h>>2)+1;
                rect.right  -= (w>>2)+1;
                rect.bottom -= (h>>2)+1;
                
                MGPlusGraphicSave (graphic, hdc, 0, 0, 0, 0, save_left, save_top);

                for(i = 0; i < (w>>4)+1; i++) // pen width w/16+1.
                {
                    Rectangle (hdc, rect.left + i, rect.top + i,
                            rect.right - i, rect.bottom - i);
                    MoveTo (hdc, rect.left + i, rect.top + i + (w>>4) + 1);
                    LineTo (hdc, rect.right - i, rect.top + i + (w>>4) + 1);
                }

                
                MGPlusBrushDelete (brush);
                MGPlusGraphicDelete (graphic);
            } 
            if(ht_code == HT_MAXBUTTON) break;

        case HT_MINBUTTON:
            if (calc_we_area (hWnd, HT_MINBUTTON, &rect) != -1)
            {
                if(RECTH(rect) <= 0 || RECTW(rect) <= 0)
                {
                    MGPlusPenDelete (pen);
                    MGPlusPathDelete (path);
                    return;
                }
                if(status & LFRDR_BTN_STATUS_PRESSED){
                    pixels[1] = mp_gradient_color (cap_c, LFRDR_3DBOX_COLOR_LIGHTER, 50);
                    alp_c = calc_alpha (cap_c, gradient_color (cap_c, LFRDR_3DBOX_COLOR_LIGHTER, 50));
                }
                else if(status & LFRDR_BTN_STATUS_HILITE){
                    pixels [1] = mp_gradient_color (cap_c, LFRDR_3DBOX_COLOR_LIGHTER, 80);
                    alp_c = calc_alpha (cap_c, gradient_color (cap_c, LFRDR_3DBOX_COLOR_LIGHTER, 80));
                }
                else {
                    pixels [1] = mp_gradient_color (cap_c, LFRDR_3DBOX_COLOR_LIGHTER, 10);
                    alp_c = calc_alpha (cap_c, gradient_color (cap_c, LFRDR_3DBOX_COLOR_LIGHTER, 10));
                }

                rc_close.left = 0;
                rc_close.top = 0;
                rc_close.right = RECTW (rect); 
                rc_close.bottom = RECTH (rect);
                save_left = rect.left;
                save_top = rect.top;

                pixels [0] = mp_gradient_color (cap_c, LFRDR_3DBOX_COLOR_LIGHTER, 250);


                cx = (rc_close.left + rc_close.right) >> 1;
                cy = (rc_close.top + rc_close.bottom) >> 1;
                r = (RECTH (rc_close) >> 1) - 1;

                MPPOINT pt = {(float)cx, (float)cy};
                RECT rc={cx-r, cy-r, cx+r, cy+r};

                HGRAPHICS graphic = MGPlusGraphicCreate (RECTW (rect), RECTH (rect));
                if (!graphic){
                    MGPlusPenDelete(pen);
                    MGPlusPathDelete(path);
                    return;
                }
                BitBlt (hdc, rect.left, rect.top, RECTW (rect), RECTH (rect),
                        MGPlusGetGraphicDC (graphic), 0, 0, 0);

                HBRUSH brush = MGPlusBrushCreate(MP_BRUSH_TYPE_PATHGRADIENT); 
                if (!brush){
                    MGPlusGraphicDelete(graphic);
                    MGPlusPenDelete(pen);
                    MGPlusPathDelete(path);
                    return;
                }
                MGPlusPathReset (path);
                MGPlusSetPathGradientBrushSurroundRect (brush, &rc);
                MGPlusSetPathGradientBrushCenterPoint(brush, &pt);
                MGPlusSetPathGradientBrushCenterColor (brush, pixels[0]);
                pixels[2] = pixels[1];
                MGPlusSetPathGradientBrushSurroundColors(brush, (ARGB *)&pixels[1], 2);

                MGPlusPathAddEllipseI(path, cx, cy, r, r, TRUE);

                MGPlusFillPath(graphic, brush, path); 

                pen_color = 0xFF000000 | alp_c;
                pen_color = MPMakeARGB (GetRValue (pen_color), GetGValue (pen_color),
                        GetBValue (pen_color), GetAValue (pen_color));
                MGPlusPenSetColor (pen, pen_color);
                MGPlusDrawPath (graphic, pen, path); 
             
                SetPenColor(hdc, fgc_3d); 
                
                rect.right --; rect.bottom --;
                
                w = RECTW(rect);
                h = RECTH(rect);

                rect.left   += (w>>2)+1;
                rect.top    += (h>>2)+1;
                rect.right  -= (w>>2)+1;
                rect.bottom -= (h>>2)+1;
                
                MGPlusGraphicSave (graphic, hdc, 0, 0, 0, 0, save_left, save_top);
                for(i = 0; i < ((w>>4)+1); i++) // pen width (w/16+1)X2.
                {
                    MoveTo (hdc, rect.left + i, rect.bottom - i - (w>>3));
                    LineTo (hdc, rect.right - i, rect.bottom - i - (w>>3));
                    MoveTo (hdc, rect.left + i, rect.bottom + i - (w>>3));
                    LineTo (hdc, rect.right - i, rect.bottom + i - (w>>3));
                }

                
                MGPlusBrushDelete (brush);
                MGPlusGraphicDelete (graphic);
            } 
            if(ht_code == HT_MINBUTTON) break;
            if(ht_code == 0) break;

        default:
            break;
    }

    MGPlusPenDelete (pen);
    MGPlusPathDelete (path);
}

static int calc_scrollbarctrl_area(HWND hWnd, int sb_pos, PRECT prc)
{
    FUNCTION_SCOPE_TIMING();
    PSCROLLBARDATA pScData = (PSCROLLBARDATA) GetWindowAdditionalData2 (hWnd);
    GetClientRect (hWnd, prc);
    
    switch (sb_pos)
    {
        case HT_HSCROLL:
        case HT_VSCROLL:
                break;
        case HT_SB_LEFTARROW:
            {
                prc->right = prc->left + pScData->arrowLen;
                break;
            }
        case HT_SB_UPARROW:
            {
                prc->bottom = prc->top + pScData->arrowLen;
                break;
            }
        case HT_SB_RIGHTARROW:
            {
                prc->left = prc->right - pScData->arrowLen;
                break;
            }
        case HT_SB_DOWNARROW:
            {
                prc->top = prc->bottom - pScData->arrowLen;
                break;
            }
        case HT_SB_HTHUMB:
            {
                prc->left = pScData->barStart + pScData->arrowLen;
                prc->right = prc->left + pScData->barLen - 1;
                break;
            }
        case HT_SB_VTHUMB:
            {
                prc->top = pScData->barStart + pScData->arrowLen;
                prc->bottom = prc->top + pScData->barLen - 1;
                break;
            }
        default:
            return -1;
    }
    if(0 >= prc->right - prc->left || 0 >= prc->bottom - prc->top)
        return -1;
  
    return 0;
}

static int get_scroll_status (HWND hWnd, BOOL isVert)
{
    FUNCTION_SCOPE_TIMING();
    int sb_status; 
    const WINDOWINFO  *info;
    
    if (0 == strncasecmp(CTRL_SCROLLBAR, GetClassName(hWnd), strlen(CTRL_SCROLLBAR)))
    {
        sb_status = ((PSCROLLBARDATA)GetWindowAdditionalData2(hWnd))->status;
    }
    else
    {
        info = (WINDOWINFO*)GetWindowInfo (hWnd);
        if(isVert)
            sb_status = info->vscroll.status; 
        else
            sb_status = info->hscroll.status; 
    }
   return sb_status; 
}

/* draw_scrollbar:
 *   This function draw the scrollbar of a window. 
 *
 * \param hWnd : the handle of the window.
 * \param hdc : the DC of the window.
 * \param sb_pos : the pos need to draw.
 * \param status : the status of the drawing part. 
 *
 * Author: wangjian<wangjian@minigui.org>
 * Date: 2007-11-22
 */
static void draw_scrollbar (HWND hWnd, HDC hdc, int sb_pos)
{
    FUNCTION_SCOPE_TIMING();
    BOOL    isCtrl = FALSE;    /** if TRUE it is scrollbar control else not */
    int     sb_status = 0;
    int     bn_status = 0;
    RECT    rect, rect1, rect2;
    DWORD   fgc_3d, fgc_dis, bgc;
    gal_pixel   pixels [2], pixels_other [2];
    gal_pixel   old_brush_color, old_pen_color;
    const WINDOWINFO  *info = (WINDOWINFO*)GetWindowInfo (hWnd);
    HPATH   path;
    HBRUSH  brush;

    old_pen_color = GetPenColor(hdc);
    brush = MGPlusBrushCreate (MP_BRUSH_TYPE_LINEARGRADIENT);
    if (!brush){
        return;
    }
    path = MGPlusPathCreate (MP_PATH_FILL_MODE_WINDING);
    if (!path) {
        MGPlusBrushDelete (brush);
        return;
    }

    fgc_dis = GetWindowElementAttr(hWnd, WE_BGCB_ACTIVE_CAPTION);
    fgc_3d = gradient_color (fgc_dis, LFRDR_3DBOX_COLOR_LIGHTER, 80);
    bgc = GetWindowElementAttr (hWnd, WE_MAINC_THREED_BODY);

    if (0 == strncasecmp(CTRL_SCROLLBAR, GetClassName(hWnd), 
                strlen(CTRL_SCROLLBAR)))
    {
        isCtrl = TRUE;
    }
    
    int (*calc_fn)(HWND hWnd, int sb_pos, PRECT prc);

    if( isCtrl )
        calc_fn = calc_scrollbarctrl_area;
    else 
        calc_fn = calc_we_area;

    if (0 != calc_fn(hWnd, sb_pos, &rect)) {
        goto ret;
    }

    switch (sb_pos)
    {
        case HT_HSCROLL:       /* paint the hscrollbar */
        { 
            // DK: Fill the blank of gap junction.
            if(info->dwStyle & WS_VSCROLL)
            {
                RECT rc_vs_down;
                int flag = 0;
                if (0 == calc_we_area (hWnd, HT_SB_DOWNARROW, &rc_vs_down))
                {
                    old_brush_color = 
                        SetBrushColor(hdc, RGB2Pixel(hdc, 0xEE,0xED,0xE5));
                    flag = 1;
                }
                FillBox(hdc, rc_vs_down.left, rc_vs_down.bottom, 
                            RECTW(rc_vs_down), RECTH(rect));
                if (flag) {
                    SetBrushColor(hdc, old_brush_color);
                }
            }
            RECT rc_larrow; 
            RECT rc_rarrow; 
            RECT rc_thumb; 
            RECT rc_back;

            /* draw the rectangle */
            SetPenColor(hdc, DWORD2Pixel(hdc, 
                              gradient_color(fgc_dis, 
                             LFRDR_3DBOX_COLOR_DARKER, 10)));
            Rectangle (hdc, rect.left, rect.top, rect.right-1, 
                    rect.bottom-1);

            HGRAPHICS graphic = MGPlusGraphicCreate (RECTW (rect), RECTH (rect));
            
            if (!graphic)
            {
                goto ret;
            }

            HDC hdc_graphic = MGPlusGetGraphicDC (graphic);

            /* draw the background */
            rc_back.top =  1;
            rc_back.left =  1;
            rc_back.right = RECTW (rect) - 1;
            rc_back.bottom = RECTH (rect) - 1;
            
            pixels[0] = mp_gradient_color(bgc, LFRDR_3DBOX_COLOR_LIGHTER, 20);
            pixels[1] = mp_gradient_color(bgc, LFRDR_3DBOX_COLOR_LIGHTER, 220);

            /* draw left arrow */
            if (0 == calc_fn(hWnd, HT_SB_LEFTARROW, &rc_larrow))
            {
                rc_back.left += (RECTW (rc_larrow) - 1);
                sb_status = get_scroll_status (hWnd, FALSE);
                if (sb_status & SBS_DISABLED_LTUP) // The arrow button disabled.
                {
                    bn_status |= LFRDR_BTN_STATUS_DISABLED;
                }
                else
                {
                    if(sb_status & SBS_PRESSED_LTUP)
                        bn_status |= LFRDR_BTN_STATUS_PRESSED;
                    else if(sb_status & SBS_HILITE_LTUP)
                        bn_status |= LFRDR_BTN_STATUS_HILITE;
                }

                if(sb_status & SBS_DISABLED_LTUP || sb_status & SBS_DISABLED) 
                {
                    draw_arrow(hWnd, hdc, 
                            &rc_larrow, fgc_3d, 
                            bn_status|LFRDR_ARROW_HAVESHELL|LFRDR_ARROW_LEFT);
                }
                else {
                    draw_arrow(hWnd, hdc, 
                            &rc_larrow, fgc_dis, 
                            bn_status|LFRDR_ARROW_HAVESHELL|LFRDR_ARROW_LEFT);
                }
            }

            /* draw right arrow */
            if (0 == calc_fn(hWnd, HT_SB_RIGHTARROW, &rc_rarrow))
            {
                // DK: Add for fix the bug that both arrow button change to hilite/pressed at one time.
                bn_status = 0;
                rc_back.right -= (RECTW (rc_larrow) - 1);
                sb_status = get_scroll_status (hWnd, FALSE);
                if (sb_status & SBS_DISABLED_BTDN) 
                {
                    bn_status |= LFRDR_BTN_STATUS_DISABLED;
                }
                else
                {
                    if(sb_status & SBS_PRESSED_BTDN)
                        bn_status |= LFRDR_BTN_STATUS_PRESSED;
                    else if(sb_status & SBS_HILITE_BTDN)
                        bn_status |= LFRDR_BTN_STATUS_HILITE;
                }

                if((sb_status & SBS_DISABLED_BTDN) || (sb_status & SBS_DISABLED))
                {
                    draw_arrow (hWnd, hdc, &rc_rarrow, fgc_3d, 
                            bn_status | LFRDR_ARROW_HAVESHELL | LFRDR_ARROW_RIGHT);
                }
                else
                {
                    draw_arrow (hWnd, hdc, &rc_rarrow, fgc_dis, 
                            bn_status | LFRDR_ARROW_HAVESHELL | LFRDR_ARROW_RIGHT);
                }
            }
            /* draw thumb */
            if (0 == calc_fn(hWnd, HT_SB_HTHUMB, &rc_thumb))
            {
                // Fill the scroll slot.
                BitBlt (hdc, rect.left, rect.top, RECTW (rect), RECTH (rect),
                        hdc_graphic, 0, 0, 0);

                MGPlusSetLinearGradientBrushMode(brush, 
                        MP_LINEAR_GRADIENT_MODE_VERTICAL);
                MGPlusSetLinearGradientBrushRect(brush, &rc_back);
                MGPlusSetLinearGradientBrushColors(brush, (ARGB*)pixels, 2);

                // Draw slot blank of both sides of thumb.
                int relative_left = rc_thumb.left - RECTW(rc_larrow);
                if (rect.left < relative_left - 1)
                {
                    int left_blank_w = relative_left - rect.left;
                    MGPlusPathAddRectangleI(path, rc_back.left, rc_back.top, 
                            left_blank_w, RECTH(rc_back));
                    MGPlusFillPath (graphic, brush, path); 
                    MGPlusGraphicSave (graphic, hdc, RECTW(rc_rarrow), 0,
                            left_blank_w, RECTH(rect), rect.left + RECTW(rc_rarrow), rect.top);
                }
                int relative_right = rect.right - RECTW(rc_rarrow);
                if (rc_thumb.right < relative_right - 1)
                {
                    int right_blank_w = relative_right - rc_thumb.right;

                    MGPlusPathReset (path);
                    MGPlusPathAddRectangleI(path, rc_back.right - right_blank_w, rc_back.top, 
                            right_blank_w, RECTH(rc_back));
                    MGPlusFillPath (graphic, brush, path); 
                    MGPlusGraphicSave (graphic, hdc, rc_back.right - right_blank_w - 1, 0,
                            right_blank_w, RECTH(rect), rc_thumb.right - (RECTW(rc_rarrow) ? 0 : 1), rect.top);
                }

                {
                    pixels[0] = mp_gradient_color (fgc_3d, LFRDR_3DBOX_COLOR_LIGHTER, 250);
                    pixels[1] = mp_gradient_color (fgc_dis, LFRDR_3DBOX_COLOR_LIGHTER, 70);

                    pixels_other[0] = mp_gradient_color(fgc_dis, 
                            LFRDR_3DBOX_COLOR_LIGHTER, 70);
                    pixels_other[1] = mp_gradient_color(fgc_dis, 
                            LFRDR_3DBOX_COLOR_LIGHTER, 100);

                    HGRAPHICS thumb_graphic = MGPlusGraphicCreate (RECTW (rc_thumb), RECTH (rc_thumb));
                    if (thumb_graphic)
                    {
                        BitBlt (hdc, rc_thumb.left, rc_thumb.top, RECTW (rc_thumb), RECTH (rc_thumb),
                                MGPlusGetGraphicDC (thumb_graphic), 0, 0, 0);

                        rect1.top = 1;
                        rect1.left = 1;
                        rect1.right = RECTW (rc_thumb);
                        rect1.bottom = rect1.top + RECTH(rc_thumb)/3;

                        rect2.top = rect1.bottom;
                        rect2.left = rect1.left;
                        rect2.right = rect1.right;
                        rect2.bottom = RECTH(rc_thumb) - 1;

                        MGPlusSetLinearGradientBrushRect (brush, &rect1);
                        MGPlusSetLinearGradientBrushColors (brush, (ARGB*)pixels, 2); 

                        MGPlusPathReset (path);
                        MGPlusPathAddRectangleI (path, rect1.left, rect1.top,
                                RECTW(rect1), RECTH(rect1));
                        MGPlusFillPath (thumb_graphic, brush, path); 

                        MGPlusSetLinearGradientBrushRect (brush, &rect2);
                        MGPlusSetLinearGradientBrushColors (brush, (ARGB*)pixels_other, 2); 

                        MGPlusPathReset (path);
                        MGPlusPathAddRectangleI (path, rect2.left, rect2.top,
                                RECTW(rect2), RECTH(rect2));
                        MGPlusFillPath (thumb_graphic, brush, path); 

                        MGPlusGraphicSave (thumb_graphic, hdc, 0, 0, RECTW(rc_thumb), RECTH(rc_thumb),
                                                                rc_thumb.left, rc_thumb.top);

                        MGPlusGraphicDelete (thumb_graphic);
                    }
                }
                SetPenColor (hdc, DWORD2Pixel (hdc, 
                            gradient_color (fgc_dis, LFRDR_3DBOX_COLOR_DARKER, 10)));
                MoveTo (hdc, rc_thumb.left, rc_thumb.top);
                LineTo (hdc, rc_thumb.left, rc_thumb.bottom - 1);
                LineTo (hdc, rc_thumb.right, rc_thumb.bottom - 1);
                LineTo (hdc, rc_thumb.right, rc_thumb.top);
                LineTo (hdc, rc_thumb.left, rc_thumb.top);

            }
            MGPlusGraphicDelete (graphic);
        }
        break;
        case HT_VSCROLL:       /* paint the vscrollbar */
        {
            RECT rc_up_arrow; 
            RECT rc_down_arrow; 
            RECT rc_thumb; 
            RECT rc_back;

            /* draw the rectangle */
            SetPenColor (hdc, 
                    DWORD2Pixel (hdc, 
                        gradient_color (fgc_dis, 
                            LFRDR_3DBOX_COLOR_DARKER, 10)));
            Rectangle(hdc, rect.left, 
                    rect.top, rect.right - 1, rect.bottom - 1);

            pixels[0] = mp_gradient_color(bgc, LFRDR_3DBOX_COLOR_LIGHTER, 20);
            pixels[1] = mp_gradient_color (bgc, LFRDR_3DBOX_COLOR_LIGHTER, 200);

            rc_back.top =  1;
            rc_back.left =  1;
            rc_back.right = RECTW (rect) - 1;
            rc_back.bottom = RECTH (rect) - 1;

            HGRAPHICS graphic = MGPlusGraphicCreate (RECTW (rect), RECTH (rect));

            if (!graphic)
            {
                goto ret;
            }

            HDC hdc_graphic = MGPlusGetGraphicDC (graphic);

            if (0 == calc_fn(hWnd, HT_SB_UPARROW, &rc_up_arrow))
            {
                rc_back.top += RECTH (rc_up_arrow) - 1;
                sb_status = get_scroll_status(hWnd, TRUE);
                if (sb_status & SBS_DISABLED_LTUP)
                    bn_status |= LFRDR_BTN_STATUS_DISABLED;
                else
                {
                    if(sb_status & SBS_PRESSED_LTUP)
                        bn_status |= LFRDR_BTN_STATUS_PRESSED;
                    else if(sb_status & SBS_HILITE_LTUP)
                        bn_status |= LFRDR_BTN_STATUS_HILITE;
                }

                if(sb_status & SBS_DISABLED_LTUP || sb_status & SBS_DISABLED)
                    draw_arrow(hWnd, hdc, &rc_up_arrow, fgc_3d, 
                            bn_status | LFRDR_ARROW_HAVESHELL | LFRDR_ARROW_UP);
                else
                    draw_arrow(hWnd, hdc, &rc_up_arrow, fgc_dis, 
                            bn_status | LFRDR_ARROW_HAVESHELL | LFRDR_ARROW_UP);
            }

            if (0 == calc_fn(hWnd, HT_SB_DOWNARROW, &rc_down_arrow))
            {
                // DK: Add for fix the bug that both arrow button change to 
                // hilite/pressed state at one time.
                bn_status = 0;
                rc_back.bottom -= RECTH (rc_down_arrow) - 1;
                sb_status = get_scroll_status (hWnd, TRUE);
                if (sb_status & SBS_DISABLED_BTDN)
                    bn_status |= LFRDR_BTN_STATUS_DISABLED;
                else
                {
                    if(sb_status & SBS_PRESSED_BTDN)
                        bn_status |= LFRDR_BTN_STATUS_PRESSED;
                    else if(sb_status & SBS_HILITE_BTDN)
                        bn_status |= LFRDR_BTN_STATUS_HILITE;
                }

                if(sb_status & SBS_DISABLED_BTDN || sb_status & SBS_DISABLED)
                    draw_arrow (hWnd, hdc, &rc_down_arrow, fgc_3d, 
                            bn_status | LFRDR_ARROW_HAVESHELL | LFRDR_ARROW_DOWN);
                else
                    draw_arrow (hWnd, hdc, &rc_down_arrow, fgc_dis, 
                            bn_status | LFRDR_ARROW_HAVESHELL | LFRDR_ARROW_DOWN);
            }

            if (0 == calc_fn(hWnd, HT_SB_VTHUMB, &rc_thumb))
            {
                BitBlt (hdc, rect.left, rect.top, RECTW (rect), RECTH (rect),
                        hdc_graphic, 0, 0, 0);
                MGPlusSetLinearGradientBrushMode (brush, 
                        MP_LINEAR_GRADIENT_MODE_HORIZONTAL);
                MGPlusSetLinearGradientBrushRect (brush, &rc_back);
                MGPlusSetLinearGradientBrushColors (brush, (ARGB*)pixels, 2); 
                // Draw slot blank of both sides of thumb.
                int relative_top = rc_thumb.top - RECTH(rc_up_arrow);
                if (rect.top < relative_top - 1)
                {
                    int above_blank_h = relative_top - rect.top;
                    MGPlusPathAddRectangleI(path, rc_back.left, rc_back.top, 
                            RECTW(rc_back), above_blank_h);
                    MGPlusFillPath (graphic, brush, path); 
                    MGPlusGraphicSave (graphic, hdc, 0, RECTH(rc_up_arrow),
                            RECTW(rect), above_blank_h + 1, rect.left, rect.top + RECTH(rc_up_arrow));
                }
                int relative_bottom = rect.bottom - RECTH(rc_down_arrow);
                if (rc_thumb.bottom < relative_bottom - 1)
                {
                    int below_blank_h = relative_bottom - rc_thumb.bottom;
                    MGPlusPathReset (path);
                    MGPlusPathAddRectangleI(path, rc_back.left, rc_back.bottom - below_blank_h, 
                            RECTW(rc_back), below_blank_h);
                    MGPlusFillPath (graphic, brush, path); 
                    MGPlusGraphicSave (graphic, hdc, 0, rc_back.bottom - below_blank_h,
                            RECTW(rect), below_blank_h, rect.left, rc_thumb.bottom 
                            - (RECTH(rc_down_arrow) ? 0 : 1));
                }

                pixels[0] = mp_gradient_color(fgc_3d, LFRDR_3DBOX_COLOR_LIGHTER, 250);
                pixels[1] = mp_gradient_color(fgc_dis, LFRDR_3DBOX_COLOR_LIGHTER, 70);

                pixels_other[0] = mp_gradient_color(fgc_dis, 
                        LFRDR_3DBOX_COLOR_LIGHTER, 70);
                pixels_other[1] = mp_gradient_color(fgc_dis, 
                        LFRDR_3DBOX_COLOR_LIGHTER, 100);

                HGRAPHICS thumb_graphic = MGPlusGraphicCreate (RECTW (rc_thumb), RECTH (rc_thumb));
                if (thumb_graphic)
                {
                    BitBlt (hdc, rc_thumb.left, rc_thumb.top, RECTW (rc_thumb), RECTH (rc_thumb),
                            MGPlusGetGraphicDC (thumb_graphic), 0, 0, 0);
                    rect1.top = 1;
                    rect1.left = 1;
                    rect1.right = rect1.left + RECTW(rc_thumb)/3;
                    rect1.bottom = RECTH (rc_thumb);

                    rect2.top = rect1.top;
                    rect2.left = rect1.right;
                    rect2.right = RECTW(rc_thumb) - 1;
                    rect2.bottom = rect1.bottom;

                    MGPlusSetLinearGradientBrushRect (brush, &rect1);
                    MGPlusSetLinearGradientBrushColors (brush, (ARGB*)pixels, 2); 

                    MGPlusPathReset (path);
                    MGPlusPathAddRectangleI (path, rect1.left, rect1.top, 
                            RECTW(rect1), RECTH(rect1));
                    MGPlusFillPath (thumb_graphic, brush, path); 

                    MGPlusSetLinearGradientBrushRect(brush, &rect2);
                    MGPlusSetLinearGradientBrushColors(brush, (ARGB*)pixels_other, 2); 

                    MGPlusPathReset (path);
                    MGPlusPathAddRectangleI(path, rect2.left, rect2.top,
                            RECTW(rect2), RECTH(rect2));

                    MGPlusFillPath (thumb_graphic, brush, path); 
                    MGPlusGraphicSave (thumb_graphic, hdc, 0, 0, RECTW(rc_thumb), RECTH(rc_thumb),
                                                            rc_thumb.left, rc_thumb.top);

                    MGPlusGraphicDelete (thumb_graphic);
                }
                SetPenColor (hdc, DWORD2Pixel (hdc, 
                            gradient_color (fgc_dis,
                                LFRDR_3DBOX_COLOR_DARKER, 10)));
                MoveTo (hdc, rc_thumb.left, rc_thumb.top);
                LineTo (hdc, rc_thumb.right - 1, rc_thumb.top);
                LineTo (hdc, rc_thumb.right - 1, rc_thumb.bottom);
                LineTo (hdc, rc_thumb.left, rc_thumb.bottom);
                LineTo (hdc, rc_thumb.left, rc_thumb.top);
                //                MoveTo (hdc, rc_thumb.left + 1, rc_thumb.bottom);
                //                LineTo (hdc, rc_thumb.right - 2, rc_thumb.bottom);
            }
            MGPlusGraphicDelete (graphic);
        }
        break;
        default:
            break;
    }

ret:
    MGPlusPathDelete (path);
    MGPlusBrushDelete (brush);
    SetPenColor(hdc, old_pen_color);
}

static void draw_trackbar_thumb (HWND hWnd, HDC hdc, 
        const RECT* pRect, DWORD dwStyle)
{
    FUNCTION_SCOPE_TIMING();
    RECT    rc_draw;
    BOOL    vertical;

    gal_pixel pixels[3];
    DWORD bgc;

    int corner, l, t, r, b;

    /** leave little margin */
    if (dwStyle & TBS_VERTICAL) {
        rc_draw.left   = pRect->left;
        rc_draw.top    = pRect->top + 2;
        rc_draw.right  = pRect->right - 1;
        rc_draw.bottom = pRect->bottom - 2;
        vertical = TRUE;
    }
    else{
        rc_draw.left   = pRect->left + 2;
        rc_draw.top    = pRect->top;
        rc_draw.right  = pRect->right - 2;
        rc_draw.bottom = pRect->bottom - 1;
        vertical = FALSE;
    }

    bgc = GetWindowElementAttr (hWnd, WE_MAINC_THREED_BODY);
    pixels [0] = MPMakeARGB (GetRValue(bgc), GetGValue(bgc), GetBValue(bgc), GetAValue(bgc));
    
    bgc = GetWindowElementAttr(hWnd, WE_BGCB_ACTIVE_CAPTION);
    pixels [1] = mp_gradient_color(bgc, LFRDR_3DBOX_COLOR_LIGHTER, 20);
    pixels [2] = pixels[0];

    MPLinearGradientMode mode;
    if (!vertical) {
        mode = MP_LINEAR_GRADIENT_MODE_VERTICAL;
    }
    else {
        mode = MP_LINEAR_GRADIENT_MODE_HORIZONTAL;
    }

    linear_gradient_draw(hdc, mode, &rc_draw, pixels, 3);
     /*draw border*/
    corner = 1;
    l = rc_draw.left;
    t = rc_draw.top;
    r = rc_draw.right;
    b = rc_draw.bottom;

    SetPenColor (hdc, DWORD2Pixel (hdc, gradient_color(bgc, LFRDR_3DBOX_COLOR_LIGHTER, 20)));
    MoveTo(hdc, l + corner, t);
    LineTo(hdc, r - 1 - corner, t);
    LineTo(hdc, r - 1, t+corner);
    LineTo(hdc, r - 1, b - 1-corner);
    LineTo(hdc, r - 1-corner, b - 1);
    LineTo(hdc, l+corner, b - 1);
    LineTo(hdc, l, b - 1-corner);
    LineTo(hdc, l, t+corner);
    LineTo(hdc, l+corner, t);
}

static void 
calc_trackbar_rect (HWND hWnd, LFRDR_TRACKBARINFO *info, DWORD dwStyle,
        const RECT* rcClient, RECT* rcRuler, RECT* rcBar, RECT* rcBorder)
{
    FUNCTION_SCOPE_TIMING();
    int x, y, w, h;
    int pos, min, max;
    int sliderx, slidery, sliderw, sliderh;

    x = rcClient->left;
    y = rcClient->top;
    w = RECTWP (rcClient);
    h = RECTHP (rcClient);

    pos = info->nPos;
    max = info->nMax;
    min = info->nMin;

    /* Calculate border rect. */
    if (dwStyle & TBS_BORDER) {
        x += TB_BORDER;
        y += TB_BORDER;
        w -= TB_BORDER << 1;
        h -= TB_BORDER << 1;
    }

    if (rcBorder) {
        SetRect (rcBorder, x, y, x+w, y+h);
    }

    /* Calculate ruler rect. */
    if (rcRuler) {
        if (dwStyle & TBS_VERTICAL) {
            rcRuler->left = x + ((w - WIDTH_VERT_RULER)>>1);
            rcRuler->top = y + (HEIGHT_VERT_SLIDER >> 1);
            rcRuler->right = x + ((w + WIDTH_VERT_RULER)>>1);
            rcRuler->bottom = y + h - (HEIGHT_VERT_SLIDER >> 1);
        }
        else {
            rcRuler->left = x + (WIDTH_HORZ_SLIDER >> 1);
            rcRuler->top = y + ((h - HEIGHT_HORZ_RULER)>>1);
            rcRuler->right = x + w - (WIDTH_HORZ_SLIDER >> 1);
            rcRuler->bottom = y + ((h + HEIGHT_HORZ_RULER)>>1);
        }
    }

    if (rcBar) {
        /* Calculate slider rect. */
        if (dwStyle & TBS_VERTICAL) {
            sliderw = WIDTH_VERT_SLIDER;
            sliderh = HEIGHT_VERT_SLIDER;
        }
        else {
            sliderw = WIDTH_HORZ_SLIDER;
            sliderh = HEIGHT_HORZ_SLIDER;
        }

        if (dwStyle & TBS_VERTICAL) {
            sliderx = x + ((w - sliderw) >> 1); 
            slidery = y + (HEIGHT_VERT_SLIDER>>1)+ (int)((max - pos) * 
                    (h - HEIGHT_VERT_SLIDER) / (float)(max - min)) - (sliderh>>1);
        }
        else {
            slidery = y + ((h - sliderh) >> 1); 
            sliderx = x + (WIDTH_HORZ_SLIDER >> 1) + (int)((pos - min) * 
                    (w - WIDTH_HORZ_SLIDER) / (float)(max - min)) - (sliderw>>1);
        }

        SetRect (rcBar, sliderx, slidery, sliderx + sliderw, slidery + sliderh);
    }
}


static void 
draw_trackbar (HWND hWnd, HDC hdc, LFRDR_TRACKBARINFO *info)
{
    FUNCTION_SCOPE_TIMING();
    RECT    rc_client, rc_border, rc_ruler, rc_bar, rect;
    int     x = 0, y = 0, w = 0, h = 0;
    int     max, min;
    int     TickFreq;
    int     TickStart, TickEnd;
    int     sliderw, sliderh;
    fixed   TickGap, Tick;
    DWORD   bgc, light_dword, dwStyle;

    gal_pixel pixels[2];
    HPATH path;
    HBRUSH brush;

    /** same as backgroud color */
    bgc = GetWindowElementAttr (hWnd, WE_MAINC_THREED_BODY);

    GetClientRect (hWnd, &rc_client);
    dwStyle = GetWindowStyle (hWnd);
    HGRAPHICS graphic = MGPlusGraphicCreate (RECTW (rc_client), RECTH (rc_client));

    if (!graphic)
        return;
    HDC hdc_graphic = MGPlusGetGraphicDC (graphic);
    calc_trackbar_rect (hWnd, info, dwStyle, &rc_client, 
            &rc_ruler, &rc_bar, &rc_border);
    rc_bar.left -= rc_client.left;
    rc_bar.top -= rc_client.top;
    rc_bar.right -= rc_client.left;
    rc_bar.bottom -= rc_client.top;

    rc_border.left -= rc_client.left;
    rc_border.top -= rc_client.top;
    rc_border.right -= rc_client.left;
    rc_border.bottom -= rc_client.top;

    x = rc_border.left - rc_client.left;
    y = rc_border.top - rc_client.top;
    w = RECTW (rc_border);
    h = RECTH (rc_border);

    RECT rc_draw = {0, 0, RECTW (rc_client) - 1, RECTH (rc_client) - 1};
    BitBlt (hdc, rc_client.left, rc_client.top, RECTW (rc_client), RECTH (rc_client),
            hdc_graphic, 0, 0, 0);

    brush = MGPlusBrushCreate (MP_BRUSH_TYPE_LINEARGRADIENT); 
    if (!brush){
        MGPlusGraphicDelete (graphic);
        return;
    }

    path = MGPlusPathCreate (MP_PATH_FILL_MODE_WINDING);
    if (!path) {
        MGPlusGraphicDelete (graphic);
        MGPlusBrushDelete (brush);
        return;
    }

    rc_ruler.left -= rc_client.left;
    rc_ruler.top -= rc_client.top;
    rc_ruler.right -= rc_client.left;
    rc_ruler.bottom -= rc_client.top;

    /* get data of trackbar. */
    TickFreq = info->nTickFreq;

    /* draw the border according to trackbar style with renderer. */
    //        rc_draw.left   = 0;
    //        rc_draw.top    = 0;
    //        rc_draw.right  = RECTW (rc_client) - 1;
    //        rc_draw.bottom = RECTH (rc_client)  - 1;
    draw_3dbox (hdc_graphic, &rc_draw, bgc, LFRDR_BTN_STATUS_PRESSED);

    rect = rc_draw;
    ++rect.left;
    ++rect.top;
    --rect.right;
    --rect.bottom;

    pixels [0] = mp_gradient_color (bgc, LFRDR_3DBOX_COLOR_DARKER, 60);
    pixels [1]  = mp_gradient_color (bgc, LFRDR_3DBOX_COLOR_LIGHTER, 90);

    MPLinearGradientMode mode;
    if (!(dwStyle & TBS_VERTICAL)) {
        mode = MP_LINEAR_GRADIENT_MODE_VERTICAL;
    }
    else {
        mode = MP_LINEAR_GRADIENT_MODE_HORIZONTAL;
    }

    MGPlusSetLinearGradientBrushMode (brush, mode);
    MGPlusSetLinearGradientBrushRect (brush, &rc_draw);
    MGPlusSetLinearGradientBrushColors (brush, (ARGB*)pixels, 2); 

    MGPlusPathAddRectangleI (path, rc_draw.left, rc_draw.top, RECTW(rc_draw), RECTH(rc_draw));
    MGPlusFillPath (graphic, brush, path); 

    /* draw the rulder in middle of trackbar with renderer . */
    rc_draw.left   = rc_ruler.left;
    rc_draw.top    = rc_ruler.top;
    rc_draw.right  = rc_ruler.right - 1;
    rc_draw.bottom = rc_ruler.bottom - 1;
    draw_3dbox (hdc_graphic, &rc_draw, bgc, LFRDR_BTN_STATUS_PRESSED);

    rect = rc_draw;
    ++rect.left;
    ++rect.top;
    --rect.right;
    --rect.bottom;

    pixels[0] = mp_gradient_color(bgc, LFRDR_3DBOX_COLOR_DARKER, 40);
    pixels[1] = mp_gradient_color(bgc, LFRDR_3DBOX_COLOR_LIGHTER, 100);

    if (!(dwStyle & TBS_VERTICAL)) {
        mode = MP_LINEAR_GRADIENT_MODE_VERTICAL;
    }
    else {
        mode = MP_LINEAR_GRADIENT_MODE_HORIZONTAL;
    }
    MGPlusPathReset (path);
    MGPlusSetLinearGradientBrushMode (brush, mode);
    MGPlusSetLinearGradientBrushRect (brush, &rc_draw);
    MGPlusSetLinearGradientBrushColors (brush, (ARGB*)pixels, 2); 

    MGPlusPathAddRectangleI(path, rc_draw.left, rc_draw.top, 
            RECTW(rc_draw), RECTH(rc_draw));
    MGPlusFillPath (graphic, brush, path); 

    max = info->nMax;
    min = info->nMin;
    sliderw = RECTW(rc_bar);
    sliderh = RECTH(rc_bar);

    /* draw the tick of trackbar. */
    if (!(dwStyle & TBS_NOTICK)) {
        SetPenColor(hdc_graphic, 
                GetWindowElementPixel(hWnd, WE_FGC_THREED_BODY));
        if (dwStyle & TBS_VERTICAL) {
            TickStart = y + (HEIGHT_VERT_SLIDER >> 1); 
            TickGap = itofix(h - HEIGHT_VERT_SLIDER);
            TickGap = fixmul(TickGap, fixdiv( itofix(TickFreq), itofix (max - min)));
            TickEnd = y + h - (HEIGHT_VERT_SLIDER >> 1);

            for (Tick = itofix (TickStart); (int)(fixtof(Tick)) <= TickEnd; Tick = fixadd (Tick, TickGap) ) {
                MoveTo (hdc_graphic, x + (w>>1) + (sliderw>>1) + GAP_TICK_SLIDER, (int)(fixtof (Tick)));
                LineTo (hdc_graphic, x + (w>>1) + (sliderw>>1) + GAP_TICK_SLIDER + LEN_TICK, (int)(fixtof (Tick)));
            }
            if ((int) (fixtof (fixadd (fixsub (Tick, TickGap), ftofix (0.9)))) < TickEnd) {
                MoveTo (hdc_graphic, x + (w>>1) + (sliderw>>1) + GAP_TICK_SLIDER, TickEnd);
                LineTo (hdc_graphic, x + (w>>1) + (sliderw>>1) + GAP_TICK_SLIDER + LEN_TICK, TickEnd);
            }
        } else {
            TickStart = x + (WIDTH_HORZ_SLIDER >> 1); 
            TickGap = fixmul (itofix (w - WIDTH_HORZ_SLIDER), fixdiv (itofix (TickFreq), itofix (max - min)));
            TickEnd = x + w - (WIDTH_HORZ_SLIDER >> 1);

            for (Tick = itofix (TickStart); (int)(fixtof(Tick)) <= TickEnd; Tick = fixadd (Tick, TickGap) ) {
                MoveTo (hdc_graphic, (int)(fixtof (Tick)), y + (h>>1) + (sliderh>>1) + GAP_TICK_SLIDER);
                LineTo (hdc_graphic, (int)(fixtof (Tick)), y + (h>>1) + (sliderh>>1) + GAP_TICK_SLIDER + LEN_TICK);
            }

            if ((int) (fixtof (fixadd (fixsub (Tick, TickGap), ftofix (0.9)))) < TickEnd)
            {
                MoveTo (hdc_graphic, TickEnd, y + (h>>1) + (sliderh>>1) + GAP_TICK_SLIDER);
                LineTo (hdc_graphic, TickEnd, y + (h>>1) + (sliderh>>1) + GAP_TICK_SLIDER + LEN_TICK);
            }
        }
    }

    MGPlusPathDelete (path);
    MGPlusBrushDelete (brush);

    draw_trackbar_thumb (hWnd, hdc_graphic, &rc_bar, dwStyle);
    /* draw the focus frame with renderer. */
    if (dwStyle & TBS_FOCUS) {
        light_dword = calc_3dbox_color (bgc, LFRDR_3DBOX_COLOR_LIGHTEST);
        rc_draw.left   = x + 1;
        rc_draw.top    = y + 1;
        rc_draw.right  = x + w - 3; 
        rc_draw.bottom = y + h - 3;
        draw_focus_frame (hdc_graphic, &rc_draw, light_dword);
    }

    MGPlusGraphicSave (graphic, hdc, 0, 0, 0, 0, rc_client.left, rc_client.top);

    MGPlusGraphicDelete (graphic);
}

static void 
disabled_text_out (HWND hWnd, HDC hdc, 
        const char* spText, PRECT rc, DWORD dt_fmt)
{
    FUNCTION_SCOPE_TIMING();
    DWORD mainc, color;
    
    SetBkMode (hdc, BM_TRANSPARENT);
    SetBkColor (hdc, GetWindowBkColor (hWnd));

    mainc = GetWindowElementAttr (hWnd, WE_MAINC_THREED_BODY);
    rc->left+=1;
    rc->top+=1;
    rc->right+=1;
    rc->bottom+=1;
    color = calc_3dbox_color (mainc, LFRDR_3DBOX_COLOR_LIGHTER);
    SetTextColor (hdc,
            RGBA2Pixel (hdc, GetRValue (color), GetGValue (color), 
                GetBValue (color), GetAValue (color)));
    DrawText (hdc, spText, -1, rc, dt_fmt);

    rc->left-=1;
    rc->top-=1;
    rc->right-=1;
    rc->bottom-=1;
    color = calc_3dbox_color (mainc, LFRDR_3DBOX_COLOR_DARKER);
    SetTextColor (hdc,
            RGBA2Pixel (hdc, GetRValue (color), GetGValue (color), 
                GetBValue (color), GetAValue (color)));
    DrawText (hdc, spText, -1, rc, dt_fmt);
}

/*
 * draw_tab:
 *  This function draw a tab for the propsheet.
 * Author : wangjian<wangjian@minigui.org>
 * Date : 2007-12-18.
 */
static void 
draw_tab (HWND hWnd, HDC hdc, RECT *rect, char *title, DWORD color, 
                int flag, HICON icon)
{
    FUNCTION_SCOPE_TIMING();
    int x, ty, by, text_extent;
    SIZE size;
    int eff_chars, eff_len;
    DWORD light_c, darker_c, darkest_c;
    DWORD c1;

    light_c = calc_3dbox_color(color, LFRDR_3DBOX_COLOR_LIGHTEST);
    darker_c = calc_3dbox_color(color, LFRDR_3DBOX_COLOR_DARKER);
    darkest_c = calc_3dbox_color(color, LFRDR_3DBOX_COLOR_DARKEST);

    c1 = GetWindowElementAttr(hWnd, WE_BGCB_ACTIVE_CAPTION);

    x = rect->left + 2;
    ty = rect->top;

    if (!(flag & LFRDR_TAB_ACTIVE)) {
        if (flag & LFRDR_TAB_BOTTOM) {
            ty -= 2;
            by = rect->bottom - 2;
        } else {
            ty += 2;
            by = rect->bottom;
        }
     } else {
        by = rect->bottom ;
        FillBox (hdc, rect->left+1, ty, rect->right - rect->left, by);
     }

    /* draw the title's edge */
    /* pc3d style & flat & grap */
    if (flag & LFRDR_TAB_BOTTOM) {
        /* left and bottom */
        SetPenColor(hdc, DWORD2Pixel (hdc, light_c)); 
        MoveTo (hdc, rect->left, ty);
        LineTo (hdc, rect->left, by - 3 );
        if (flag & LFRDR_TAB_ACTIVE) {
            SetPenColor(hdc, DWORD2Pixel (hdc, c1)); 
            MoveTo (hdc, rect->left + 2, by - 1);
            LineTo (hdc, rect->right - 4, by - 1);
            MoveTo (hdc, rect->left + 1, by - 2);
            LineTo (hdc, rect->right - 3, by - 2);
            MoveTo (hdc, rect->left , by - 3);
            LineTo (hdc, rect->right - 2, by - 3);
        } else {
            SetPenColor(hdc, DWORD2Pixel (hdc, darker_c)); 
            MoveTo (hdc, rect->left + 2, by - 1);
            LineTo (hdc, rect->right - 4, by - 1);
            MoveTo (hdc, rect->left + 1, by - 2);
            LineTo (hdc, rect->left + 1, by - 2);
        }
        /*right*/
        SetPenColor(hdc, DWORD2Pixel (hdc, darkest_c)); 
        MoveTo (hdc, rect->right - 2, by - 3);
        LineTo (hdc, rect->right - 2, ty);
        MoveTo (hdc, rect->right - 3, by - 2 );
        LineTo (hdc, rect->right - 3, by - 2);
        SetPenColor(hdc, DWORD2Pixel (hdc, darker_c)); 
        MoveTo (hdc, rect->right - 3, by -3);
        LineTo (hdc, rect->right - 3, ty);
    } else {
        /*left and top */
        SetPenColor(hdc, DWORD2Pixel (hdc, light_c)); 
        MoveTo (hdc, rect->left, by - 1);
        LineTo (hdc, rect->left, ty + 2);
        if (flag & LFRDR_TAB_ACTIVE) {
            SetPenColor(hdc, DWORD2Pixel (hdc, c1)); 
            MoveTo (hdc, rect->left , ty+2);
            LineTo (hdc, rect->right - 2, ty+2);
            MoveTo (hdc, rect->left + 1, ty+1);
            LineTo (hdc, rect->right - 3, ty+1);
            MoveTo (hdc, rect->left + 2, ty);
            LineTo (hdc, rect->right - 4, ty);
        } else {
            MoveTo (hdc, rect->left + 2, ty);
            LineTo (hdc, rect->right - 4, ty);
            MoveTo (hdc, rect->left + 1, ty + 1);
            LineTo (hdc, rect->left + 1, ty + 1);
        }
        /*right*/
        SetPenColor(hdc, DWORD2Pixel (hdc, darkest_c)); 
        MoveTo (hdc, rect->right - 2, ty + 2 );
        LineTo (hdc, rect->right - 2, by - 2);
        MoveTo (hdc, rect->right - 3, ty + 1 );
        LineTo (hdc, rect->right - 3, ty + 1);
        SetPenColor(hdc, DWORD2Pixel (hdc, darker_c)); 
        MoveTo (hdc, rect->right - 3, ty + 2 );
        LineTo (hdc, rect->right - 3, by - 2);
    }

    /* draw the ICON */
    ty += 2 + 2;
    text_extent = RECTWP (rect) - 2 * 2;
    if (icon) {
        int icon_x, icon_y;
        icon_x = RECTHP(rect) - 8;
        icon_y = icon_x;
        
        DrawIcon (hdc, x, ty, icon_x, icon_y, icon);
        x += icon_x;
        x += 2;
        text_extent -= icon_x + 2;
    }
        
    /* draw the TEXT */
    SetBkColor (hdc, DWORD2Pixel (hdc, color)); 
    text_extent -= 4;
    eff_len = GetTextExtentPoint (hdc, title, strlen(title), 
                  text_extent, &eff_chars, NULL, NULL, &size);

    SetBkMode (hdc, BM_TRANSPARENT);
    TextOutLen (hdc, x + 2, ty, title, eff_len);
}

static void 
draw_progress (HWND hWnd, HDC hdc, 
        int nMax, int nMin, int nPos, BOOL fVertical)
{
    FUNCTION_SCOPE_TIMING();
    RECT    rcClient;
    int     x, y, w, h;
    ldiv_t   ndiv_progress;
    unsigned int     nAllPart;
    unsigned int     nNowPart;
    int     step, pbar_border = 2;
    gal_pixel pixel[2], bkpixel[2];
    DWORD c1, c2;
    int prog;
    char szText[8];
    SIZE text_ext;
    gal_pixel graphic_pixel;

    HPATH path;
    HBRUSH brush;

    if (nMax == nMin)
        return;
    
    if ((nMax - nMin) > 5)
        step = 5;
    else
        step = 1;

    GetClientRect (hWnd, &rcClient);

    HGRAPHICS graphic = MGPlusGraphicCreate (RECTW (rcClient), RECTH (rcClient));
    if (!graphic)
        return;
    brush = MGPlusBrushCreate (MP_BRUSH_TYPE_LINEARGRADIENT); 
    if (!brush){
        MGPlusGraphicDelete (graphic);
        return;
    }
    path = MGPlusPathCreate (MP_PATH_FILL_MODE_WINDING);
    if (!path) {
        MGPlusGraphicDelete (graphic);
        MGPlusBrushDelete (brush);
        return;
    }
    HDC hdc_graphic = MGPlusGetGraphicDC (graphic);
    BitBlt (hdc, rcClient.left, rcClient.top, RECTW (rcClient), RECTH (rcClient),
            hdc_graphic, 0, 0, 0);
    //draw progressbar bkcolor
    c1 = GetWindowElementAttr (hWnd, WE_BGC_WINDOW);
    c2 = GetWindowElementAttr (hWnd, WE_BGCB_ACTIVE_CAPTION);

    bkpixel[0] = MPMakeARGB (GetRValue(c1), GetGValue(c1), GetBValue(c1), GetAValue(c1));
    bkpixel[1] = mp_gradient_color(c1, LFRDR_3DBOX_COLOR_LIGHTER, 120);

    RECT rc_tmp;
    rc_tmp.left = rc_tmp.top = 0;
    rc_tmp.right = RECTW (rcClient);
    rc_tmp.bottom = RECTH (rcClient);

    MGPlusSetLinearGradientBrushMode (brush, 
                MP_LINEAR_GRADIENT_MODE_VERTICAL);
    MGPlusSetLinearGradientBrushRect (brush, &rc_tmp);
    MGPlusSetLinearGradientBrushColors (brush, (ARGB*)bkpixel, 2); 

    MGPlusPathAddRectangleI (path, rc_tmp.left, rc_tmp.top, 
            RECTW(rc_tmp), RECTH(rc_tmp));
    MGPlusFillPath (graphic, brush, path); 
    
    //draw progressbar frame
    draw_fashion_frame (hdc_graphic, rc_tmp, 
            gradient_color(c1, LFRDR_3DBOX_COLOR_DARKER, 120));

    x = rc_tmp.left + pbar_border;
    y = rc_tmp.top + pbar_border;
    w = RECTW (rc_tmp) - (pbar_border << 1);
    h = RECTH (rc_tmp) - (pbar_border << 1);

    ndiv_progress = ldiv (nMax - nMin, step);
    nAllPart = ndiv_progress.quot;
    
    ndiv_progress = ldiv (nPos - nMin, step);
    nNowPart = ndiv_progress.quot;

    pixel[0] = mp_gradient_color(c2, LFRDR_3DBOX_COLOR_LIGHTER, 50);
    pixel[1] = mp_gradient_color(gradient_color(c2, LFRDR_3DBOX_COLOR_LIGHTER, 80),
                              LFRDR_3DBOX_COLOR_LIGHTER,
                              250);

    RECT rc_gradient;

    if (fVertical) {
        prog = h * nNowPart/nAllPart;

        if (nPos == nMax) prog = h;

        if (prog != 0) {
            rc_gradient.left = rc_tmp.left + 1;
            rc_gradient.right = rc_tmp.right - 1;
            rc_gradient.top = rc_tmp.bottom - prog - 3;
            rc_gradient.bottom = rc_tmp.bottom - 1;

            MGPlusSetLinearGradientBrushMode (brush, 
                            MP_LINEAR_GRADIENT_MODE_HORIZONTAL);
        }
    }
    else {
        prog = w * nNowPart/nAllPart;
        if (nPos == nMax) prog = w;

        if (prog != 0) {
            rc_gradient.top = rc_tmp.top + 1;
            rc_gradient.bottom = rc_tmp.bottom - 1;
            rc_gradient.left = rc_tmp.left + 1;
            rc_gradient.right = rc_tmp.left + prog + 3;

            MGPlusSetLinearGradientBrushMode (brush, 
                            MP_LINEAR_GRADIENT_MODE_VERTICAL);
        }
    }

    if (prog != 0) {
        MGPlusPathReset (path);
        MGPlusSetLinearGradientBrushRect (brush, &rc_gradient);
        MGPlusSetLinearGradientBrushColors (brush, (ARGB*)pixel, 2); 

        MGPlusPathAddRectangleI (path, rc_gradient.left, rc_gradient.top,
                    RECTW(rc_gradient), RECTH(rc_gradient));
        MGPlusFillPath (graphic, brush, path); 
    }

    SetBkMode (hdc_graphic, BM_TRANSPARENT);
    sprintf (szText, "%d%%", (nNowPart*100/nAllPart));
    GetTextExtent (hdc_graphic, szText, -1, &text_ext);
    x += ((w - text_ext.cx) >> 1) + 1;
    y += ((h - text_ext.cy) >> 1);

    graphic_pixel = GetWindowElementPixel(hWnd, WE_BGCB_ACTIVE_CAPTION);

    SetTextColor (hdc_graphic,
            DWORD2Pixel (hdc_graphic,
                mp_gradient_color (graphic_pixel, LFRDR_3DBOX_COLOR_LIGHTER, 10)));
    TextOut (hdc_graphic, x, y, szText);
    MGPlusGraphicSave (graphic, hdc, 0, 0, 0, 0, rcClient.left, rcClient.top);

    MGPlusPathDelete (path);
    MGPlusBrushDelete (brush);
    MGPlusGraphicDelete (graphic);
}

static void draw_header (HWND hWnd, HDC hdc, const RECT *pRect, DWORD color)
{
    FUNCTION_SCOPE_TIMING();
    gal_pixel pixels[2];
    RECT gradient_rect;
    gal_pixel pixel_org;

    if (pRect->right < pRect->left || pRect->bottom < pRect->top) {
        return;
    }

    pixels [1] = mp_gradient_color (color, LFRDR_3DBOX_COLOR_DARKER, 60);
    pixels [0] = mp_gradient_color (color, LFRDR_3DBOX_COLOR_LIGHTER, 120);

    gradient_rect.left = pRect->left + 1;
    gradient_rect.right = pRect->right;
    gradient_rect.top = pRect->top + 1;
    gradient_rect.bottom = pRect->bottom - 1;

    linear_gradient_draw (hdc, MP_LINEAR_GRADIENT_MODE_VERTICAL,
            &gradient_rect, pixels, 2);

    /*border*/
    pixel_org = SetPenColor (hdc, PIXEL_darkgray);
    MoveTo (hdc, pRect->right-1,pRect->top);
    LineTo (hdc, pRect->right-1,pRect->bottom-1);
    LineTo (hdc, pRect->left,pRect->bottom-1);

    SetPenColor (hdc, pixel_org);
}

WINDOW_ELEMENT_RENDERER wnd_rdr_fashion = {
    "fashion",
    init,
    deinit,

    calc_3dbox_color,
    draw_3dbox,
    draw_radio,
    draw_checkbox,
    draw_checkmark,
    draw_arrow,
    draw_fold,
    draw_focus_frame,

    draw_normal_item,
    draw_hilite_item,
    draw_disabled_item,
    draw_significant_item,

    draw_push_button,
    draw_radio_button,
    draw_check_button,

    draw_border,
    draw_caption,
    draw_caption_button,
    draw_scrollbar,

    calc_trackbar_rect,
    draw_trackbar,

    calc_we_area,
    calc_we_metrics,
    hit_test,
    NULL,
    NULL,

    calc_thumb_area,
    disabled_text_out,
 
    draw_tab,
    draw_progress,
    draw_header,

    NULL,
    NULL,
    erase_bkgnd,

    draw_normal_menu_item,
    draw_hilite_menu_item,
    draw_disabled_menu_item,
};

BOOL MGPlusRegisterFashionLFRDR (void)
{
    return AddWindowElementRenderer ("fashion", &wnd_rdr_fashion);
}

BOOL MGPlusUnregisterFashionLFRDR (void)
{
    return RemoveWindowElementRenderer("fashion");
}

#endif /* _MGPLUS_LFRDR_FASHION */
