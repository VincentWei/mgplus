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
#include <string.h>

#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>
#include <minigui/control.h>

#include <mgplus/mgplus.h>

#include "newlistview.h"
#include "tclanimate.h"

#define C2_KEY_LEFT     SCANCODE_CURSORBLOCKLEFT     /* 方向左键 */
#define C2_KEY_RIGHT    SCANCODE_CURSORBLOCKRIGHT    /* 方向右键 */
#define C2_KEY_UP       SCANCODE_CURSORBLOCKUP       /* 方向上键 */
#define C2_KEY_DOWN     SCANCODE_CURSORBLOCKDOWN     /* 方向下键 */

#define C2_KEY_OK       SCANCODE_ENTER          /* 确定键 */
#define C2_KEY_EXIT     SCANCODE_ESCAPE         /* 退出键 */
#define C2_KEY_SOURCE   SCANCODE_F1             /* 信源键 */
#define C2_KEY_USB      SCANCODE_F2             /* USB键 */

#define C2_KEY_PLAY     SCANCODE_P              /* 播放键 */
#define C2_KEY_PAUSE    SCANCODE_L              /* 暂停键 */
#define C2_KEY_STOP     SCANCODE_S              /* 停止键 */
#define C2_KEY_FORWORD  SCANCODE_F              /* 快进键 */
#define C2_KEY_REWIND   SCANCODE_R              /* 快退键 */
#define C2_KEY_NEXT     SCANCODE_N              /* 下一个键 */
#define C2_KEY_PREV     SCANCODE_R              /* 上一个键 */

#define C2_KEY_1        SCANCODE_1              /* 数字键 */
#define C2_KEY_2        SCANCODE_2
#define C2_KEY_3        SCANCODE_3
#define C2_KEY_4        SCANCODE_4
#define C2_KEY_5        SCANCODE_5
#define C2_KEY_6        SCANCODE_6
#define C2_KEY_7        SCANCODE_7
#define C2_KEY_8        SCANCODE_8
#define C2_KEY_9        SCANCODE_9
#define C2_KEY_0        SCANCODE_0

#define MAIN_WND_WIDTH  592
#define MAIN_WND_HEIGHT 523

#define MENU_START_X    (25+(RECTW(g_rcScr)-MAIN_WND_WIDTH)/2)
#define MENU_START_Y    (116+(RECTH(g_rcScr)-MAIN_WND_HEIGHT)/2)
#define MENU_WIDTH      270 
#define MENU_HEIGHT     50 

#define DISK_START_X    (297+(RECTW(g_rcScr)-MAIN_WND_WIDTH)/2)
#define DISK_START_Y    (116+(RECTH(g_rcScr)-MAIN_WND_HEIGHT)/2)
#define DISK_WIDTH      270 
#define DISK_HEIGHT     50 

#define MSG_SUBWNDCLOSE MSG_USER+13

static char *res_file[] =
{
    "res/usb_title.png",
    "res/midline.png",
};

static char focus_pic[] = "res/focus2.png";
static char select_pic[] = "res/focus1.png";

#if 0
static char * txt_menu[] =
{
    "电        影",
    "图        片",
    "音        乐",
    "所有文件",
    "设        置"
};
#else
static char * txt_menu[] =
{
    "电    影",
    "图    片",
    "音    乐",
    "所有文件",
    "设    置"
};

#endif
static char * txt_disk[] =
{
    "C 分区",
    "D 分区",
    "E 分区"
};
static char * txt_setting[] =
{
    "网络设置",
    "电视设置",
    "效果选择"
};

static PLOGFONT plogfont=INV_LOGFONT;
static BITMAP bmp_res[2];

static BITMAP sub_bmp_bk={0};
static BITMAP exit_sub_bmp={0};

#define ID_MENU     1001
#define ID_DISK     1002

static HWND hwnd_menu, hwnd_disk;
static HWND hsub;

static LRESULT SubWinProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch(message)
    {
        case MSG_CREATE:
            break;
        case MSG_PAINT:
        {
            HDC hdc = BeginPaint(hWnd);
            FillBoxWithBitmap(hdc, 0, 0, RECTW(g_rcScr), 
                    RECTH(g_rcScr), &sub_bmp_bk);
            EndPaint(hWnd, hdc);
            break;
        }
        case MSG_CLOSE:
            DestroyMainWindow(hWnd);
            //PostQuitMessage(hWnd);
            return 0;
        case MSG_KEYUP:
        {
            int scancode = (int)wParam;
            switch(scancode)
            {
                case C2_KEY_EXIT:
                {
                    HDC hdc = GetClientDC(hWnd);
                    GetBitmapFromDC(hdc, 0, 0, RECTW(g_rcScr),
                            RECTH(g_rcScr), &exit_sub_bmp);
                    ReleaseDC(hdc);
                    //SendMessage(hWnd, MSG_CLOSE, 0, 0);
                    PostMessage(GetHosting(hWnd), MSG_SUBWNDCLOSE, 0, 0);
                    return 0;
                }
                default:
                    break;
            }
            break;
        }
        default:
            break;
    }
    return DefaultMainWinProc(hWnd, message, wParam, lParam);
}

HWND CreateSubWnd(HWND hParentWnd)
{
    DLGTEMPLATE template = 
    {
        WS_CHILD,//WS_VISIBLE,
        WS_EX_AUTOSECONDARYDC,//WS_NONE,
        g_rcScr.left, g_rcScr.top, 
        RECTW(g_rcScr), RECTH(g_rcScr),
        "",
        0, 0,
        0, NULL,
        0
    };
    return CreateMainWindowIndirect(&template, hParentWnd, SubWinProc);
}

void GUIAPI UpdateAll (HWND hWnd, BOOL fErase)
{
    MSG Msg;
    UpdateWindow(hWnd, fErase);
    while (PeekMessageEx (&Msg, hWnd, MSG_PAINT, MSG_PAINT+1,
                            FALSE, PM_REMOVE))
    {
        TranslateMessage (&Msg);
        DispatchMessage (&Msg);
    }
}

void StartWndChangeAnimate(HWND hWnd, HWND hsub)
{
    HDC hdc;
    void * ctx=NULL;

    //zoom out main wnd
    hdc = GetClientDC(hWnd);
    ctx=InitZoomOutAnimate(3, hWnd, hdc, RECTW(g_rcScr), RECTH(g_rcScr));
    //ctx=InitZoomOutAnimate(5, hWnd, hdc, RECTW(g_rcScr), RECTH(g_rcScr));
    ReleaseDC(hdc);
    RunAnimate(30, 3, CbZoomOutAnimate, ctx);
    //RunAnimate(30, 5, CbZoomOutAnimate, ctx);
    FreeZoomOutAnimate(ctx);
    ctx = NULL;

    //zoom in sub wnd
    hdc = GetSecondaryDC(hsub);
    SetSecondaryDC(hsub, hdc, ON_UPDSECDC_DONOTHING);
    ShowWindow(hsub, SW_SHOWNORMAL);
    UpdateAll(hsub, TRUE);

    //ctx = InitSmall2NormalAnimate(10, hsub, hdc, 
    ctx = InitSmall2NormalAnimate(6, hsub, hdc, 
            RECTW(g_rcScr), RECTH(g_rcScr));
    RunAnimate(30, 6, CbSmall2NormalAnimate, ctx);
    //RunAnimate(30, 10, CbSmall2NormalAnimate, ctx);
    FreeSmall2NormalAnimate(ctx);
    SetSecondaryDC(hsub, hdc, ON_UPDSECDC_DEFAULT);
    ReleaseSecondaryDC(hsub, hdc);
}

void StartLarge2NormalAnimate(HWND hWnd, HWND hsub)
{
    void * ctx=NULL;
    HDC hdc = GetSecondaryDC(hsub);
    SetSecondaryDC(hsub, hdc, ON_UPDSECDC_DONOTHING);
    ReleaseSecondaryDC(hsub, hdc);

    hdc = GetSecondaryDC(hWnd);
    SetSecondaryDC(hWnd, hdc, ON_UPDSECDC_DONOTHING);
    ReleaseSecondaryDC(hWnd, hdc);

    SendMessage(hsub, MSG_CLOSE, 0, 0);
    FillBoxWithBitmap(HDC_SCREEN, 0, 0, RECTW(g_rcScr),
            RECTH(g_rcScr), &exit_sub_bmp);

    UpdateAll(hWnd, TRUE);

    hdc = GetSecondaryDC(hWnd);
    ctx = InitLarge2NormalAnimate(6, hWnd, hdc, RECTW(g_rcScr),
            RECTH(g_rcScr));
    RunAnimate(30, 6, CbLarge2NormalAnimate, ctx);
    //hdc = GetSecondaryDC(hWnd);
    //ctx = InitLarge2NormalAnimate(10, hWnd, hdc, RECTW(g_rcScr),
    //        RECTH(g_rcScr));
    FreeLarge2NormalAnimate(ctx);

    SetSecondaryDC(hWnd, hdc, ON_UPDSECDC_DEFAULT);
    ReleaseSecondaryDC(hWnd, hdc);
}

extern void Center4SplitAnimate(HDC hdc, const RECT *rt, 
        PBITMAP bmpIn, PBITMAP bmpOut, int frame_num, BOOL dir);

void StartCenterSplitAnimate(HWND hWnd, HWND hsub, BOOL dir)
{
    BITMAP bmp_bk={0};

    HDC render_dc = CreateCompatibleDCEx(HDC_SCREEN, 
             RECTW(g_rcScr), RECTH(g_rcScr));

    HDC hdc = GetSecondaryDC(hsub);
    SetSecondaryDC(hsub, hdc, ON_UPDSECDC_DONOTHING);
    ReleaseSecondaryDC(hsub, hdc);

    hdc = GetSecondaryDC(hWnd);
    SetSecondaryDC(hWnd, hdc, ON_UPDSECDC_DONOTHING);
    ReleaseSecondaryDC(hWnd, hdc);

    if (dir)
    {
        SendMessage(hsub, MSG_CLOSE, 0, 0);

        UpdateAll(hWnd, TRUE);
        hdc = GetSecondaryDC(hWnd);
        GetBitmapFromDC(hdc, 0, 0, 
                GetGDCapability(hdc, GDCAP_HPIXEL),
                GetGDCapability(hdc, GDCAP_VPIXEL),
                &bmp_bk);
        ReleaseSecondaryDC(hWnd, hdc);

        Center4SplitAnimate (render_dc, &g_rcScr, &bmp_bk, &exit_sub_bmp, 20, dir);

        hdc = GetSecondaryDC(hWnd);
        SetSecondaryDC(hWnd, hdc, ON_UPDSECDC_DEFAULT);
        ReleaseSecondaryDC(hWnd, hdc);

        //UnloadBitmap(&bmp_bk);
        UnloadBitmap(&exit_sub_bmp);
    }
    else
    {
        BITMAP bmp_fg={0};

        hdc = GetClientDC(hWnd);
        GetBitmapFromDC(hdc, 0, 0, 
                GetGDCapability(hdc, GDCAP_HPIXEL),
                GetGDCapability(hdc, GDCAP_VPIXEL),
                &bmp_bk);
        ReleaseDC(hdc);

        hdc = GetSecondaryDC(hsub);
        SetSecondaryDC(hsub, hdc, ON_UPDSECDC_DONOTHING);
        ShowWindow(hsub, SW_SHOWNORMAL);
        UpdateAll(hsub, TRUE);

        GetBitmapFromDC(hdc, 0, 0, 
                GetGDCapability(hdc, GDCAP_HPIXEL),
                GetGDCapability(hdc, GDCAP_VPIXEL),
                &bmp_fg);
        Center4SplitAnimate (render_dc, &g_rcScr, &bmp_bk, &bmp_fg, 20, dir);
        SetSecondaryDC(hsub, hdc, ON_UPDSECDC_DEFAULT);
        ReleaseSecondaryDC(hsub, hdc);
        UnloadBitmap(&bmp_fg);
    }
    UnloadBitmap(&bmp_bk);
    DeleteCompatibleDC(render_dc);
}

void StartBlindsInAnimate(HWND hWnd, HWND hsub)
{
    BITMAP bmp_fg={0};
    BITMAP bmp_bk={0};
    void * ctx=NULL;
    HDC hdc;

    hdc = GetSecondaryDC(hWnd);
    SetSecondaryDC(hWnd, hdc, ON_UPDSECDC_DONOTHING);
    GetBitmapFromDC(hdc, 0, 0, 
            GetGDCapability(hdc, GDCAP_HPIXEL),
            GetGDCapability(hdc, GDCAP_VPIXEL),
            &bmp_bk);
    ReleaseSecondaryDC(hWnd, hdc);

    hdc = GetSecondaryDC(hsub);
    SetSecondaryDC(hsub, hdc, ON_UPDSECDC_DONOTHING);
    ReleaseSecondaryDC(hsub, hdc);

    ShowWindow(hsub, SW_SHOWNORMAL);
    UpdateAll(hsub, TRUE);

    hdc = GetSecondaryDC(hsub);
    GetBitmapFromDC(hdc, 0, 0, 
            GetGDCapability(hdc, GDCAP_HPIXEL),
            GetGDCapability(hdc, GDCAP_VPIXEL),
            &bmp_fg);
    ReleaseSecondaryDC(hsub, hdc);

    ctx = InitBlindsInfo(hsub, 10, 8, &bmp_bk, &bmp_fg);
    RunAnimate(30, 10, CbDrawBlinds, ctx);
    DestroyBlindsInfo(ctx);

    hdc = GetSecondaryDC(hWnd);
    SetSecondaryDC(hWnd, hdc, ON_UPDSECDC_DEFAULT);
    ReleaseSecondaryDC(hWnd, hdc);

    hdc = GetSecondaryDC(hsub);
    SetSecondaryDC(hsub, hdc, ON_UPDSECDC_DEFAULT);
    ReleaseSecondaryDC(hsub, hdc);

    UnloadBitmap(&bmp_fg);
    UnloadBitmap(&bmp_bk);
}

void StartBlindsOutAnimate(HWND hWnd, HWND hsub)
{
    BITMAP bmp_fg={0};
    void * ctx=NULL;
    HDC hdc = GetSecondaryDC(hsub);
    SetSecondaryDC(hsub, hdc, ON_UPDSECDC_DONOTHING);
    ReleaseSecondaryDC(hsub, hdc);

    hdc = GetSecondaryDC(hWnd);
    SetSecondaryDC(hWnd, hdc, ON_UPDSECDC_DONOTHING);
    ReleaseSecondaryDC(hWnd, hdc);

    SendMessage(hsub, MSG_CLOSE, 0, 0);

    UpdateAll(hWnd, TRUE);
    hdc = GetSecondaryDC(hWnd);
    GetBitmapFromDC(hdc, 0, 0, 
            GetGDCapability(hdc, GDCAP_HPIXEL),
            GetGDCapability(hdc, GDCAP_VPIXEL),
            &bmp_fg);
    ReleaseSecondaryDC(hWnd, hdc);

    ctx = InitBlindsInfo(hWnd, 10, 8, &exit_sub_bmp, &bmp_fg);
    RunAnimate(30, 10, CbDrawBlinds, ctx);
    DestroyBlindsInfo(ctx);

    hdc = GetSecondaryDC(hWnd);
    SetSecondaryDC(hWnd, hdc, ON_UPDSECDC_DEFAULT);
    ReleaseSecondaryDC(hWnd, hdc);

    UnloadBitmap(&bmp_fg);
    UnloadBitmap(&exit_sub_bmp);
}


static int
TcldemoWinProc(HWND hWnd, int message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
        case MSG_CREATE:
        {
            int i;
            for(i=0;i<sizeof(res_file)/sizeof(char *);i++)
                LoadBitmap (HDC_SCREEN, &bmp_res[i], res_file[i]);

            LoadBitmapFromFile(HDC_SCREEN, &sub_bmp_bk, "res/usb_movie.jpg");
            
            plogfont = CreateLogFont("sef", 
                        "fzxhgb_yh", 
                        "UTF-8",
                        FONT_WEIGHT_BOOK,
                        FONT_SLANT_ROMAN, 
                        0, 
                        0,
                        FONT_UNDERLINE_NONE,
                        FONT_STRUCKOUT_NONE,
                        18, 0);
            if ( plogfont == INV_LOGFONT)
            {
                printf("create fzxhgb_yh font failed\n");
                plogfont = CreateLogFont ("upf", "fmsong", "UTF-8",
                        FONT_WEIGHT_REGULAR, FONT_SLANT_ROMAN,
                        FONT_FLIP_NIL, FONT_OTHER_NIL,
                        FONT_UNDERLINE_NONE, FONT_STRUCKOUT_NONE,
                        18, 0);
            }
            SetWindowFont(hWnd, plogfont);

            hwnd_menu = CreateWindow(NEW_LISTVIEW, "",  
                    WS_CHILD|NLS_2STATE,
                    ID_MENU,
                    MENU_START_X, MENU_START_Y, 
                    MENU_WIDTH, MENU_HEIGHT*5,
                    hWnd, 0);
            hwnd_disk = CreateWindow(NEW_LISTVIEW, "",  
                    WS_CHILD,
                    ID_DISK,
                    DISK_START_X, DISK_START_Y, 
                    DISK_WIDTH, DISK_HEIGHT*5,
                    hWnd, 0);
            SetWindowFont(hwnd_menu, plogfont);
            InitNewlistview(hwnd_menu, MENU_WIDTH, MENU_HEIGHT,
                    30, 5, sizeof(txt_menu)/sizeof(char *), 
                    txt_menu,
                    focus_pic,
                    select_pic);
            SetFocus(hwnd_menu);
            ShowWindow(hwnd_menu, SW_SHOWNORMAL);

            SetWindowFont(hwnd_disk, plogfont);
            InitNewlistview(hwnd_disk, DISK_WIDTH, DISK_HEIGHT,
                    30, 5, sizeof(txt_disk)/sizeof(char *), 
                    txt_disk,
                    focus_pic,
                    select_pic);
            ShowWindow(hwnd_disk, SW_SHOWNORMAL);
            break;
        }
        case MSG_COMMAND:
        {
            int id = LOWORD(wParam);
            int code = HIWORD(wParam);
            switch(code)
            {
                case NLN_ENTER:
                    if (id == ID_MENU)
                        SetFocus(hwnd_disk);
                    else
                    {
                        int idx = SendMessage(hwnd_disk, NLMSG_GETSELECTED, 0, 0);
                        hsub=CreateSubWnd(hWnd);

                        switch(idx)
                        {
                            case 0:
                                StartWndChangeAnimate(hWnd, hsub);
                                break;
                            case 1:
                                StartBlindsInAnimate(hWnd, hsub);
                                break;
                            case 2:
                                StartCenterSplitAnimate(hWnd, hsub, FALSE);
                                break;
                        }
                    }
                    break;
                case NLN_LOSTFOCUS:
                    if(id == ID_DISK)
                        SetFocus(hwnd_menu);
                    else
                        PostMessage(hWnd, MSG_CLOSE, 0, 0);
                    break;
                case NLN_FOCUSIDXCHANG:
                    if (id == ID_MENU)
                    {
                        int idx = SendMessage(hwnd_menu, NLMSG_GETFOCUSIDX, 0, 0);
                        if (idx == 4)
                            ResetListviewTxt(hwnd_disk, 
                                sizeof(txt_setting)/sizeof(char *),
                                txt_setting);
                        else
                            ResetListviewTxt(hwnd_disk, 
                                sizeof(txt_disk)/sizeof(char *),
                                txt_disk);
                    }
                    break;
                default:
                    break;
            }
            break;
        }
        case MSG_PAINT:
        {
            RECT rc;
            HDC hdc;

            hdc = BeginPaint(hWnd);
            //SelectFont(hdc, plogfont);
            GetClientRect(hWnd, &rc);
            SetBrushColor(hdc, GetWindowBkColor(hWnd));
            FillBox(hdc, 0, 0, RECTW(rc), RECTH(rc));

            SetTextColor(hdc, PIXEL_lightwhite);
            SetBkMode(hdc, BM_TRANSPARENT);

            FillBoxWithBitmap(hdc, 
                    (RECTW(g_rcScr)-bmp_res[0].bmWidth)/2,
                    (RECTH(g_rcScr)-MAIN_WND_HEIGHT)/2,
                    0, 0, &bmp_res[0]);
            FillBoxWithBitmap(hdc, 
                    (RECTW(g_rcScr)-bmp_res[1].bmWidth)/2, 
                    (RECTH(g_rcScr)-MAIN_WND_HEIGHT)/2+bmp_res[0].bmHeight, 
                    0, 0, &bmp_res[1]);

            EndPaint(hWnd, hdc);
            break;
        }
        case MSG_SUBWNDCLOSE:
        {
            int idx = SendMessage(hwnd_disk, NLMSG_GETSELECTED, 0, 0);
            switch(idx)
            {
                default:
                case 0:
                    StartLarge2NormalAnimate(hWnd, hsub);
                    break;
                case 1:
                    StartBlindsOutAnimate(hWnd, hsub);
                    break;
                case 2:
                    StartCenterSplitAnimate(hWnd, hsub, TRUE);
                    break;
            }
            break;
        }
        case MSG_CLOSE:
        {
            int i;
            for(i=0;i<sizeof(res_file)/sizeof(char *);i++)
                UnloadBitmap (&bmp_res[i]);

            UnloadBitmap(&sub_bmp_bk);

            DestroyLogFont(plogfont);
            DestroyMainWindow(hWnd);
            PostQuitMessage(hWnd);
            return 0;
        }
        default:
            break;
    }

    return DefaultMainWinProc(hWnd, message, wParam, lParam);
}

#ifdef _MISC_MOUSECALIBRATE
static void
mouse_calibrate(void)
{
    POINT   src_pts[5] =
        { {5, 10}, {600, 20}, {620, 450}, {20, 470}, {310, 234} };
    POINT   dst_pts[5] = { {0, 0}, {639, 0}, {639, 479}, {0, 479}, {320, 240} };

    SetMouseCalibrationParameters(src_pts, dst_pts);
}
#endif /* !_MISC_MOUSECALIBRATE */

int MiniGUIMain(int argc, const char *argv[])
{
    HDC     hdc;
    MSG     Msg;
    HWND    hMainWnd;
    MAINWINCREATE CreateInfo;

#ifdef _MGRM_PROCESSES
    JoinLayer(NAME_DEF_LAYER, "tcldemo", 0, 0);
#endif

#ifdef _MISC_MOUSECALIBRATE
    mouse_calibrate();
#endif
    RegisterNewlistview();

    CreateInfo.dwStyle = WS_VISIBLE;
    CreateInfo.dwExStyle = WS_EX_AUTOSECONDARYDC;
    CreateInfo.spCaption = "";
    //CreateInfo.hMenu = hMenu;
    CreateInfo.hMenu = 0;
    CreateInfo.hCursor = 0;
    CreateInfo.hCursor = GetSystemCursor(0);
    CreateInfo.hIcon = 0;
    CreateInfo.MainWindowProc = TcldemoWinProc;
    CreateInfo.lx = g_rcScr.left;
    CreateInfo.ty = g_rcScr.top;
    CreateInfo.rx = g_rcScr.right;
    CreateInfo.by = g_rcScr.bottom;
    CreateInfo.iBkColor = COLOR_black;
    CreateInfo.dwAddData = 0;
    CreateInfo.hHosting = HWND_DESKTOP;

    hMainWnd = CreateMainWindow(&CreateInfo);

    if (hMainWnd == HWND_INVALID)
        return -1;
    
    //SetBitmapScalerType(HDC_SCREEN, BITMAP_SCALER_BILINEAR);

    hdc = GetSecondaryDC(hMainWnd);
    SetSecondaryDC(hMainWnd, hdc, ON_UPDSECDC_DONOTHING);

    ShowWindow(hMainWnd, SW_SHOWNORMAL);
    UpdateAll (hMainWnd, TRUE);

    //AlphaAnimate(hMainWnd, 30, TRUE);
    void *ctx = InitSmall2NormalAnimate(6, hMainWnd, hdc, MAIN_WND_WIDTH, MAIN_WND_HEIGHT);
    RunAnimate(30, 6, CbSmall2NormalAnimate, ctx);
    //void *ctx = InitSmall2NormalAnimate(10, hMainWnd, hdc, MAIN_WND_WIDTH, MAIN_WND_HEIGHT);
    //RunAnimate(30, 10, CbSmall2NormalAnimate, ctx);
    FreeSmall2NormalAnimate(ctx);

    SetSecondaryDC(hMainWnd, hdc, ON_UPDSECDC_DEFAULT);
    ReleaseSecondaryDC(hMainWnd, hdc);
    while (GetMessage(&Msg, hMainWnd))
    {
        TranslateMessage(&Msg);
        DispatchMessage(&Msg);
    }

    UnregisterNewlistview();
    MainWindowThreadCleanup(hMainWnd);
    return 0;
}

#ifndef _LITE_VERSION
#include <minigui/dti.c>
#endif
