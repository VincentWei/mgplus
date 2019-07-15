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
#include <pthread.h>

#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>
#include <minigui/control.h>

#include "tclanimate.h"

int alpha_in[] = 
{132, 187, 224, 245, 251, 255};
//{99, 132, 161, 187, 208, 224, 236, 245, 251, 255};
//{33, 66, 99, 132, 157, 182, 207, 224, 240, 255};

int alpha_out[] =
{ 245, 187, 0};
//{ 245, 224, 187, 99, 0};

//small to normal size
void * InitSmall2NormalAnimate(int frame_num, HWND hwnd, HDC hdc, 
        int w, int h)
{
    S2NANIMATE *info=(S2NANIMATE *)calloc(1, sizeof(S2NANIMATE));
    if (!info)
        return NULL;

    info->hwnd = hwnd;
    info->frame_num = frame_num;
    info->maxw = w;
    info->maxh = h;
    info->fash_stepw = w*1.6/frame_num;
    info->fash_steph = h*1.6/frame_num;
    info->slow_stepw = w*0.4/frame_num;
    info->slow_steph = h*0.4/frame_num;
    //info->alpha = 255/frame_num;

    //SetBitmapScalerType(HDC_SCREEN, BITMAP_SCALER_BILINEAR);

    GetBitmapFromDC(hdc, 0, 0,
            GetGDCapability(hdc, GDCAP_MAXX),
            GetGDCapability(hdc, GDCAP_MAXY),
            &info->bmp);
    info->bmp.bmType |= BMP_TYPE_ALPHACHANNEL;
    info->hdc = CreateCompatibleDC(HDC_SCREEN);
    return (void *)info;
}

void FreeSmall2NormalAnimate(void *context)
{
    S2NANIMATE *info=(S2NANIMATE *)context;
    UnloadBitmap(&info->bmp);
    DeleteCompatibleDC(info->hdc);
    free(info);
}

void CbSmall2NormalAnimate(int index, void *context)
{
    int w, h;
    int sx, sy;
    S2NANIMATE *info = (S2NANIMATE *)context;

    if (index < info->frame_num/2)
    {
        w = info->fash_stepw*index;
        h = info->fash_steph*index;
    }
    else
    {
        if (index == info->frame_num) {
            w = info->maxw;
            h = info->maxh;
        }
        else {
            w = info->fash_stepw*5+info->slow_stepw*(index-5);
            h = info->fash_steph*5+info->slow_steph*(index-5);
        }
    }
    sx = (RECTW(g_rcScr)-w)/2;
    sy = (RECTH(g_rcScr)-h)/2;

    //RECT rc;
    //HDC shdc = GetClientDC(info->hwnd);
    //GetWindowRect(info->hwnd, &rc);
    //SetMemDCAlpha(shdc, MEMDC_FLAG_SRCALPHA, alpha_in[index]);
    //BitBlt(shdc, 0, 0, 0, 0, HDC_SCREEN, sx, sy, 0);
    //ReleaseDC(shdc);

    SetBrushColor(info->hdc, PIXEL_black);
    FillBox(info->hdc, sx, sy, w, h);

    info->bmp.bmAlpha = alpha_in[index-1];
    FillBoxWithBitmap(info->hdc, sx, sy, w, h, &info->bmp);
    BitBlt(info->hdc, sx, sy, w, h, HDC_SCREEN, sx, sy, 0);
}

void * InitZoomOutAnimate(int frame_num, HWND hwnd, HDC hdc, int w, int h)
{
    ZOOMOUTANIMATE *info=(ZOOMOUTANIMATE *)calloc(1, sizeof(ZOOMOUTANIMATE ));
    if (!info)
        return NULL;

    info->hwnd = hwnd;
    info->frame_num = frame_num;
    info->fast_mul  = 1.0/frame_num;
    info->alpha = 255/frame_num;
    //info->slow_mul = 0.8/frame_num;
    info->w = w;
    info->h = h;

    //SetBitmapScalerType(HDC_SCREEN, BITMAP_SCALER_BILINEAR);

    GetBitmapFromDC(hdc, 0, 0, 
            //GetGDCapability(hdc, GDCAP_HPIXEL),
            //GetGDCapability(hdc, GDCAP_VPIXEL),
            //GetGDCapability(hdc, GDCAP_MAXX),
            //GetGDCapability(hdc, GDCAP_MAXY),
            w, h,
            &info->bmp);

    info->bmp.bmType |= BMP_TYPE_ALPHACHANNEL;
    info->hdc = CreateCompatibleDC(HDC_SCREEN);
    return (void *)info;
}

void FreeZoomOutAnimate(void *context)
{
    ZOOMOUTANIMATE *info=(ZOOMOUTANIMATE *)context;
    UnloadBitmap(&info->bmp);
    DeleteCompatibleDC(info->hdc);
    free(info);
}

void CbZoomOutAnimate(int index, void *context)
{
    int w, h;
    int sx, sy;
    ZOOMOUTANIMATE *info = (ZOOMOUTANIMATE *)context;

    w = info->w * (1+info->fast_mul * index);
    h = info->h * (1+info->fast_mul * index);

    sx = (RECTW(g_rcScr)-w)/2;
    sy = (RECTH(g_rcScr)-h)/2;

    SetBrushColor(info->hdc, PIXEL_black);
    FillBox(info->hdc, g_rcScr.left, g_rcScr.top, 
            RECTW(g_rcScr), RECTH(g_rcScr));
    
    //info->bmp.bmAlpha = info->alpha*(info->frame_num-index);
    info->bmp.bmAlpha = alpha_out[index-1];
    FillBoxWithBitmap(info->hdc, sx, sy, w, h, &info->bmp);

    BitBlt(info->hdc, 0, 0, 0, 0, HDC_SCREEN, 0, 0, 0);
}

void draw_alpha(HWND hwnd, int alpha)
{
    RECT rc;
    HDC shdc = GetSecondaryDC(hwnd);
    GetWindowRect(hwnd, &rc);
    //WindowToScreen(hwnd, &rc.left, &rc.top);
	SetMemDCAlpha(shdc, MEMDC_FLAG_SRCALPHA, alpha);
	BitBlt(shdc, 0, 0, 0, 0, HDC_SCREEN, rc.left, rc.top, 0);
    ReleaseSecondaryDC(hwnd, shdc);
}


void * InitLarge2NormalAnimate(int frame_num, HWND hwnd, HDC hdc, int w, int h)
{
    ZOOMOUTANIMATE *info=(ZOOMOUTANIMATE *)calloc(1, sizeof(ZOOMOUTANIMATE ));
    if (!info)
        return NULL;

    info->hwnd = hwnd;
    info->frame_num = frame_num;
    info->fast_mul  = 1.0/frame_num;
    info->alpha = 255/frame_num;
    //info->slow_mul = 0.8/frame_num;
    info->w = w;
    info->h = h;

    //SetBitmapScalerType(HDC_SCREEN, BITMAP_SCALER_BILINEAR);

    GetBitmapFromDC(hdc, 0, 0, 
            GetGDCapability(hdc, GDCAP_MAXX),
            GetGDCapability(hdc, GDCAP_MAXY),
            &info->bmp);

    info->bmp.bmType |= BMP_TYPE_ALPHACHANNEL;
    info->hdc = CreateCompatibleDC(hdc);
    info->dhdc = CreateCompatibleDC(HDC_SCREEN);
    return (void *)info;
}

void FreeLarge2NormalAnimate(void *context)
{
    ZOOMOUTANIMATE *info=(ZOOMOUTANIMATE *)context;
    UnloadBitmap(&info->bmp);
    DeleteCompatibleDC(info->hdc);
    DeleteCompatibleDC(info->dhdc);
    free(info);
}

void CbLarge2NormalAnimate(int index, void *context)
{
    int w, h;
    int sx, sy;
    ZOOMOUTANIMATE *info = (ZOOMOUTANIMATE *)context;

    if (index == info->frame_num)
    {
        w = info->w;
        h = info->h;
        sx = (RECTW(g_rcScr)-w)/2-1;
        sy = (RECTH(g_rcScr)-h)/2-1;
    }
    else
    {
        w = info->w * (2-info->fast_mul * index);
        h = info->h * (2-info->fast_mul * index);
        sx = (RECTW(g_rcScr)-w)/2;
        sy = (RECTH(g_rcScr)-h)/2;
    }

    SetBrushColor(info->dhdc, PIXEL_black);
    FillBox(info->dhdc, g_rcScr.left, g_rcScr.top,
            RECTW(g_rcScr), RECTH(g_rcScr));
    info->bmp.bmAlpha = alpha_in[index-1];
    FillBoxWithBitmap(info->dhdc, sx, sy, w, h, &info->bmp);
    BitBlt(info->dhdc, 0, 0, 0, 0, HDC_SCREEN, 0, 0, 0);
}

typedef struct _blinds_info
{
    HDC hdc;
    int frame_num;
    int part_num;
    int part_w;
    int part_h;
    div_t frame_h;
    div_t final;
    PBITMAP bmp_bk;
    PBITMAP bmp_fg;
}BLINDSINFO;

void *InitBlindsInfo(HWND hwnd, int frame_num,
        int part_num, PBITMAP bmp_bk, PBITMAP bmp_fg)
{
    RECT rc;
    div_t part;
    BLINDSINFO *info = calloc(1, sizeof(BLINDSINFO));

    info->hdc = CreateCompatibleDC(HDC_SCREEN);

    GetClientRect(hwnd, &rc);

    info->frame_num = frame_num;
    info->part_num = part_num;

    /*count the max height of blind*/
    part = div(RECTH(rc)+part_num, part_num);
    info->part_h = part.quot;
    info->part_w = RECTW(rc);

    /*count the height of blind which 1~part_num-1*/
    info->frame_h = div(info->part_h, frame_num);

    /*count the last height of blind*/
    info->final = div(info->part_h - part_num + part.rem, frame_num);

    info->bmp_bk = bmp_bk;
    info->bmp_fg = bmp_fg;

    return (void *)info;
}

void DestroyBlindsInfo(void *ctx)
{
    BLINDSINFO *info = (BLINDSINFO *)ctx;
    ReleaseDC(info->hdc);
    free(info);
}

void CbDrawBlinds(int idx, void *ctx)
{
    int i, h;
    BLINDSINFO *info = (BLINDSINFO *)ctx;

    FillBoxWithBitmap(info->hdc, 0, 0, 0, 0, &info->bmp_bk);
    idx -= 1;
    for(i=0;i<info->part_num-1;i++)
    {
        if (idx < info->frame_h.rem)
            h = i*info->part_h+idx*(info->frame_h.quot+1);
        else
            h = i*info->part_h+idx*(info->frame_h.quot)+info->frame_h.rem;

        FillBoxWithBitmapPart(info->hdc,
            0, h,
            info->part_w, 
            (idx < info->frame_h.rem)?info->frame_h.quot+1:info->frame_h.quot,
            0, 0,
            info->bmp_fg, 0, h);
    }

    /*draw the final part*/
    if (idx < info->final.rem)
        h = i*info->part_h+idx*(info->final.quot+1);
    else
        h = i*info->part_h+idx*(info->final.quot)+info->final.rem;

    FillBoxWithBitmapPart(info->hdc,
            0, h,
            info->part_w, 
            (idx < info->final.rem)?info->final.quot+1:info->final.quot,
            //part_w, h,
            0, 0,
            info->bmp_fg, 0, h);

    BitBlt(info->hdc, 0, 0, 0, 0, HDC_SCREEN, 0, 0, 0);
}

void RunAnimate(int interval, int frame_num, CB_ANIMATE cbanimate, void *context)
{
	pthread_cond_t cond;
	pthread_mutex_t mutex;

    int index=1;
	int interval_sec;
	int interval_nsec;
	struct timeval tv;
    int start_time;

	gettimeofday(&tv,NULL);
	start_time =tv.tv_sec*1000 + (tv.tv_usec+999)/1000;

	pthread_cond_init(&cond,NULL);
	pthread_mutex_init(&mutex, NULL);

	interval_sec = interval/1000;
	interval_nsec = (interval%1000)*1000000;

    while(1)
    {
		struct timeval tv;
		struct timespec timeout;

		gettimeofday(&tv, NULL);
		timeout.tv_sec = tv.tv_sec + interval_sec;
		timeout.tv_nsec = tv.tv_usec*1000 + interval_nsec;
		if(timeout.tv_nsec > 1000000000){
			timeout.tv_sec ++;
			timeout.tv_nsec -= 1000000000;
		}

		pthread_mutex_lock(&mutex);
        cbanimate(index, context);
        index++;
        if (index > frame_num)
            break;

		pthread_cond_timedwait(&cond, &mutex, &timeout);
		pthread_mutex_unlock(&mutex);
    }

	pthread_cond_destroy(&cond);
	pthread_mutex_destroy(&mutex);
}

