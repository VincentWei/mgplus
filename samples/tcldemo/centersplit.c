///////////////////////////////////////////////////////////////////////////////
//
//                          IMPORTANT NOTICE
//
// The following open source license statement does not apply to any
// entity in the Exception List published by FMSoft.
//
// For more information, please visit:
//
// https://www.fmsoft.cn/exception-list
//
//////////////////////////////////////////////////////////////////////////////
#include <sys/time.h>

#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>
#include <minigui/control.h>

#include <mgplus/mgplus.h>

#include "./../animate/common_animates/common_animates.h"
#include "stdio.h"
#include "string.h"

#define PI 3.14159265358979323846

static void def_out_bkgnd(HDC hdc, const RECT* rtbk, void *param)
{
    PBITMAP bmpIn = (PBITMAP)param;
    FillBoxWithBitmap(hdc, 0, 0,  
            RECTW(g_rcScr), RECTH(g_rcScr), bmpIn);
}

static void def_out_draw_animate(HDC hdc, ANIMATE* ani)
{
    if(GetAnimateW(ani) != 0 && GetAnimateH(ani)!=0) {
        PBITMAP pbmp = (PBITMAP)ani->img;
        if (GetAnimateX(ani) >= -2 && GetAnimateX(ani) < 0 ) {
            SetAnimateX(ani, 0);
        }
        if (GetAnimateY(ani) >= -2 && GetAnimateY(ani) < 0) {
            SetAnimateY(ani, 0);
        }
        FillBoxWithBitmap(hdc,
                GetAnimateX(ani), GetAnimateY(ani),
                pbmp->bmWidth, pbmp->bmHeight,
                pbmp);
    }
}

static void on_end_draw_one_frame(ANIMATE_SENCE *as)
{
	if(as != NULL)
	{
		BitBlt(as->hdc, 0, 0, RECTW(as->rtArea),RECTH(as->rtArea), HDC_SCREEN, 0, 0, 0); 
	}
}
    
static void GetSubBitmap(PBITMAP pbmp, PBITMAP psub, int x, int y, int w, int h)
{
    memcpy(psub, pbmp, sizeof(BITMAP));
    psub->bmWidth  = w;
    psub->bmHeight = h;
    psub->bmBits   = pbmp->bmBits + (pbmp->bmPitch * y) 
        + x * pbmp->bmBytesPerPixel;
}

void Center4SplitAnimate(HDC hdc, const RECT *rt, 
        PBITMAP bmpIn, PBITMAP bmpOut, int frame_num, BOOL dir)
{
	int w,h;
	w = RECTWP(rt);
	h = RECTHP(rt);
    BITMAP bmp1, bmp2, bmp3, bmp4;
    int center_x = w/2, center_y = h/2;

    GetSubBitmap(bmpOut, &bmp1, 0, 0, w/2, h/2);
    GetSubBitmap(bmpOut, &bmp2, center_x, 0, w/2, h/2);
    GetSubBitmap(bmpOut, &bmp3, 0, center_y, w/2, h/2);
    GetSubBitmap(bmpOut, &bmp4, center_x, center_y, w/2, h/2);

    {
        ANIMATE_OPS ops = {
            def_out_draw_animate,
            def_out_bkgnd,
            NULL,
            NULL,
            on_end_draw_one_frame
        };
        SetInterval (50);
        if (dir){
            PUSH_PULL_OBJ objs[4] ={
                {&bmp1, 0, 0, -center_x, -center_y},
                {&bmp2, center_x, 0, w, -center_y},
                {&bmp3, 0, center_y, -center_x, h},
                {&bmp4, center_x, center_y, w, h},
            };
            RunPushPullAnimate(hdc, rt, objs, TABLESIZE(objs),  &ops, 20, bmpIn);
        }
        else
        {
            PUSH_PULL_OBJ objs[4] ={
                {&bmp1, -center_x, -center_y, 0, 0},
                {&bmp2, w, -center_y, center_x, 0},
                {&bmp3, -center_x, h, 0, center_y},
                {&bmp4, w, h, center_x, center_y},
            };

            RunPushPullAnimate(hdc, rt, objs, TABLESIZE(objs),  &ops, 20, bmpIn);
        }
    }
}

