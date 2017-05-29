#ifndef _ANIMATE_H
#define _ANIMATE_H

typedef struct _small_to_normal_animate
{
    BOOL    f_enter;
    HWND    hwnd;
    HDC     hdc;
    int     frame_num;
    int     maxw;
    int     maxh;
    int     fash_stepw;
    int     fash_steph;
    int     slow_stepw;
    int     slow_steph;
    //float   alpha;
    BITMAP  bmp;
}S2NANIMATE;

typedef struct _zoom_out_animate
{
    BOOL    f_enter;
    HWND    hwnd;
    HDC     hdc;
    HDC     dhdc;
    int     frame_num;
    int     w;
    int     h;
    float   alpha;
    float   fast_mul;
    //float   slow_mul;
    BITMAP  bmp;
}ZOOMOUTANIMATE;

typedef void (*CB_ANIMATE)(int index, void *context);

void RunAnimate(int interval, int frame_num, CB_ANIMATE cbanimate, void *context);

void * InitSmall2NormalAnimate(int frame_num, HWND hwnd, HDC hdc, int w, int h);
void FreeSmall2NormalAnimate(void *context);
void CbSmall2NormalAnimate(int index, void *context);

void * InitZoomOutAnimate(int frame_num, HWND hwnd, HDC hdc, int w, int h);
void FreeZoomOutAnimate(void *context);
void CbZoomOutAnimate(int index, void *context);

void * InitLarge2NormalAnimate(int frame_num, HWND hwnd, HDC hdc, int w, int h);
void FreeLarge2NormalAnimate(void *context);
void CbLarge2NormalAnimate(int index, void *context);

void *InitBlindsInfo(HWND hwnd, int frame_num, int part_num, PBITMAP bmp_bk, PBITMAP bmp_fg);
void DestroyBlindsInfo(void *ctx);
void CbDrawBlinds(int idx, void *ctx);
#endif
