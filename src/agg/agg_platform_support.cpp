//----------------------------------------------------------------------------
// Anti-Grain Geometry (AGG) - Version 2.5
// A high quality rendering engine for C++
// Copyright (C) 2002-2006 Maxim Shemanarev
// Copyright (C) 2004 Mauricio Piacentini (SDL Support)
// Contact: mcseem@antigrain.com
//          mcseemagg@yahoo.com
//          http://antigrain.com
// 
// AGG is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
// 
// AGG is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with AGG; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, 
// MA 02110-1301, USA.
//----------------------------------------------------------------------------

#include <string.h>

extern "C" {
#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>
#include <minigui/control.h>
}

#include "agg_platform_support.h"

namespace agg
{

    //------------------------------------------------------------------------
    class platform_specific
    {
    public:
        platform_specific(pix_format_e format, bool flip_y);
        //platform_specific(pix_format_e format, bool flip_y, HWND hwnd);
        ~platform_specific();

        pix_format_e  m_format;
        pix_format_e  m_sys_format;
        bool          m_flip_y;
        unsigned      m_bpp;
        unsigned      m_sys_bpp;
        unsigned      m_rmask;
        unsigned      m_gmask;
        unsigned      m_bmask;
        unsigned      m_amask;
        bool          m_update_flag;
        bool          m_resize_flag;
        bool          m_initialized;
        HDC           m_surf_window;
        PBITMAP       m_surf_img[platform_support::max_images];
        int           m_cur_x;
        int           m_cur_y;
	    int           m_sw_start;
    };

    //------------------------------------------------------------------------
    platform_specific::platform_specific(pix_format_e format, bool flip_y) :
        m_format(format),
        m_sys_format(pix_format_undefined),
        m_flip_y(flip_y),
        m_bpp(0),
        m_sys_bpp(0),
        m_update_flag(true), 
        m_resize_flag(true),
        m_initialized(false),
        m_surf_window(0),
        m_cur_x(0),
        m_cur_y(0)
    {
        memset(m_surf_img, 0, sizeof(m_surf_img));
        //printf("m_format=%d\n", m_format);
        switch(m_format)
        {
			case pix_format_gray8:
            m_bpp = 8;
            break;

        case pix_format_rgb565:
            m_rmask = 0xF800;
            m_gmask = 0x7E0;
            m_bmask = 0x1F;
            m_amask = 0;
            m_bpp = 16;
            break;

        case pix_format_rgb555:
            m_rmask = 0x7C00;
            m_gmask = 0x3E0;
            m_bmask = 0x1F;
            m_amask = 0;
            m_bpp = 16;
            break;
			
#if SDL_BYTEORDER == SDL_LIL_ENDIAN
        case pix_format_rgb24:
			m_rmask = 0xFF;
            m_gmask = 0xFF00;
            m_bmask = 0xFF0000;
            m_amask = 0;
            m_bpp = 24;
            break;

        case pix_format_bgr24:
            m_rmask = 0xFF0000;
            m_gmask = 0xFF00;
            m_bmask = 0xFF;
            m_amask = 0;
            m_bpp = 24;
            break;

        case pix_format_bgra32:
            m_rmask = 0xFF0000;
            m_gmask = 0xFF00;
            m_bmask = 0xFF;
            m_amask = 0xFF000000;
            m_bpp = 32;
            break;

        case pix_format_abgr32:
            m_rmask = 0xFF000000;
            m_gmask = 0xFF0000;
            m_bmask = 0xFF00;
            m_amask = 0xFF;
            m_bpp = 32;
            break;

        case pix_format_argb32:
            m_rmask = 0xFF00;
            m_gmask = 0xFF0000;
            m_bmask = 0xFF000000;
            m_amask = 0xFF;
            //m_amask = 0xFF000000;
            m_bpp = 32;
            break;

        case pix_format_rgba32:
            m_rmask = 0xFF;
            m_gmask = 0xFF00;
            m_bmask = 0xFF0000;
            m_amask = 0xFF000000;
            //m_amask = 0xFF;
            m_bpp = 32;
            break;
#else //SDL_BIG_ENDIAN (PPC)
        case pix_format_rgb24:
			m_rmask = 0xFF0000;
            m_gmask = 0xFF00;
            m_bmask = 0xFF;
            m_amask = 0;
            m_bpp = 24;
            break;

        case pix_format_bgr24:
            m_rmask = 0xFF;
            m_gmask = 0xFF00;
            m_bmask = 0xFF0000;
            m_amask = 0;
            m_bpp = 24;
            break;

        case pix_format_bgra32:
            m_rmask = 0xFF00;
            m_gmask = 0xFF0000;
            m_bmask = 0xFF000000;
            m_amask = 0xFF;
            m_bpp = 32;
            break;

        case pix_format_abgr32:
            m_rmask = 0xFF;
            m_gmask = 0xFF00;
            m_bmask = 0xFF0000;
            m_amask = 0xFF000000;
            m_bpp = 32;
            break;

        case pix_format_argb32:
            m_rmask = 0xFF0000;
            m_gmask = 0xFF00;
            m_bmask = 0xFF;
            m_amask = 0xFF000000;
            m_bpp = 32;
            break;

        case pix_format_rgba32:
            m_rmask = 0xFF000000;
            m_gmask = 0xFF0000;
            m_bmask = 0xFF00;
            m_amask = 0xFF;
            m_bpp = 32;
            break;
#endif
        default:
            _MG_PRINTF ("mGPlus>AGG: Not handled pixel format: %x.\n", m_format);
            break;
        }
    }

    //------------------------------------------------------------------------
    platform_specific::~platform_specific()
    {
        int i;
        for(i = platform_support::max_images - 1; i >= 0; --i)
        {
            if(m_surf_img[i]) 
            {
#ifndef _MG_MINIMALGDI
                UnloadBitmap(m_surf_img[i]);
#endif
                free(m_surf_img[i]);
                m_surf_img[i]=NULL;
            }
        }
        if(m_surf_window) 
            DeleteMemDC(m_surf_window);
    }

    //------------------------------------------------------------------------
    platform_support::platform_support(pix_format_e format, bool flip_y) :
        m_specific(new platform_specific(format, flip_y)),
        m_format(format),
        m_bpp(m_specific->m_bpp),
        m_window_flags(0),
        m_wait_mode(true),
        m_flip_y(flip_y)
    {
        //GAL_Init(GAL_INIT_VIDEO);
        //strcpy(m_caption, "Anti-Grain Geometry Application");
    }


    //------------------------------------------------------------------------
    platform_support::~platform_support()
    {
        delete m_specific;
    }


    //------------------------------------------------------------------------
    void platform_support::caption(const char* cap)
    {
        //SetWindowCaption(m_specific->m_hwnd, cap);
    }
    
#define SURFACE(dc)    ((unsigned char*)dc+2*sizeof(short)+2*sizeof(int))
#define PIXEL(surface) ((unsigned char*)surface+4*sizeof(int)+2*sizeof(char*))

    //------------------------------------------------------------------------
    bool platform_support::init(unsigned width, unsigned height, unsigned flags)
    {
        m_window_flags = flags;
        unsigned wflags = MEMDC_FLAG_SWSURFACE;

        if(m_window_flags & window_hw_buffer)
        {
            wflags = MEMDC_FLAG_HWSURFACE;
        }
/*
        if(m_window_flags & window_resize)
        {
            wflags |= GAL_RESIZABLE;
        }
*/
        //SDL_WM_SetCaption(m_caption, 0);

        if(m_specific->m_surf_window) 
            DeleteMemDC(m_specific->m_surf_window);
        
#if 0
        printf("init memdc: w=%d, h=%d, m_bpp=%d, wflags=%d\n", width, height, m_bpp, wflags);
        printf("init memdc: m_rmask=%x, m_gmask=%x, m_bmask=%x, m_amask=%x\n", 
                    m_specific->m_rmask,
                    m_specific->m_gmask,
                    m_specific->m_bmask,
                    m_specific->m_amask);
#endif
        m_specific->m_surf_window = CreateMemDC( 
                                 width, 
                                 height,
                                 m_bpp,
                                 wflags,
                                 m_specific->m_rmask, 
                                 m_specific->m_gmask, 
                                 m_specific->m_bmask, 
                                 m_specific->m_amask);

        if(m_specific->m_surf_window == 0) 
        {
            fprintf(stderr, 
                    "Unable to create image buffer %dx%d %d bpp: %s\n", 
                    width, 
                    height, 
                    m_bpp, 
                    //SDL_GetError());
                    "failed");
            return false;
        }
        int pitch = width * (m_bpp/8);
        //printf("init, pitch=%d\n", pitch);    
#if 0
        m_rbuf_window.attach((unsigned char*)PIXEL(SURFACE(m_specific->m_surf_window)), 
                             width, 
                             height, 
                             m_flip_y ? -pitch:pitch);
#else

        m_rbuf_window.attach((unsigned char*)LockDC(m_specific->m_surf_window, NULL, NULL, NULL, NULL), 
                             width, 
                             height, 
                             m_flip_y ? -pitch:pitch);
#endif
        if(!m_specific->m_initialized)
        {
            m_initial_width = width;
            m_initial_height = height;
            on_init();
            m_specific->m_initialized = true;
        }
        on_resize(m_rbuf_window.width(), m_rbuf_window.height());
        m_specific->m_update_flag = true;
        return true;
    }


    HDC platform_support::rbuf_dc()   
    { 
        return m_specific->m_surf_window; 
    } 
    
    unsigned long platform_support::rbuf_img_p(int idx)
    {
        if(idx < max_images) 
            return (unsigned long)m_specific->m_surf_img[idx];
        return 0;
    }

    //------------------------------------------------------------------------
    void platform_support::update_window()
    {
        //printf("platform_support::update_window\n");
        //BitBlt(m_specific->m_surf_window, 0, 0, 0, 0, 
        //        HDC_SCREEN, 0, 0, 0);
    }


    //------------------------------------------------------------------------
    int platform_support::run()
    {
        return 0;
    }


    //------------------------------------------------------------------------
    const char* platform_support::img_ext() const { return ".bmp"; }

    //------------------------------------------------------------------------
    const char* platform_support::full_file_name(const char* file_name)
    {
        return file_name;
    }

    bool platform_support::load_img_from_bitmap(unsigned idx, PBITMAP pbmp)
    {
        if(idx < max_images) 
        {
            int ret = FALSE;

#ifndef _MG_MINIMALGDI
            if(m_specific->m_surf_img[idx]) 
                UnloadBitmap(m_specific->m_surf_img[idx]);
            else
                m_specific->m_surf_img[idx] = (PBITMAP)calloc(1, sizeof(BITMAP)); 
#endif

            HDC tmpdc = CreateCompatibleDC(m_specific->m_surf_window);
            HDC tmpdc2 = CreateCompatibleDC(HDC_SCREEN);

            if(tmpdc == HDC_INVALID)
                return false;

            FillBoxWithBitmap(tmpdc2, 0, 0, pbmp->bmWidth, pbmp->bmHeight, 
                    pbmp);

            BitBlt(tmpdc2, 0, 0, 0, 0, tmpdc, 0, 0, 0);
            do {
                if (!GetBitmapFromDC(tmpdc, 
                            0, 0, 240, 320, 
                            m_specific->m_surf_img[idx]))
                {
                    ret = FALSE;
                    break;
                }
                m_rbuf_img[idx].attach(m_specific->m_surf_img[idx]->bmBits, 
                        m_specific->m_surf_img[idx]->bmWidth, 
                        m_specific->m_surf_img[idx]->bmHeight, 
                        m_flip_y?(-m_specific->m_surf_img[idx]->bmPitch):
                        m_specific->m_surf_img[idx]->bmPitch);

                ret = TRUE;

            } while(FALSE);

            DeleteCompatibleDC(tmpdc);
            DeleteCompatibleDC(tmpdc2);

            return ret;
        }
        return false;
    }
    bool platform_support::load_img_from_dc(unsigned idx, HDC hdc)
    {
        if(idx < max_images) 
        {
#ifndef _MG_MINIMALGDI
            if(m_specific->m_surf_img[idx]) 
                UnloadBitmap(m_specific->m_surf_img[idx]);
            else
                m_specific->m_surf_img[idx] = (PBITMAP)calloc(1, sizeof(BITMAP)); 
#endif

#if 0
            if (LoadBitmapFromMem(m_specific->m_surf_window, 
                                m_specific->m_surf_img[idx], 
                                (const void *)LockDC(hdc, NULL, NULL, NULL, NULL), 
                                0,
                                "bmp") != 0)
#else
            HDC tmpdc = CreateCompatibleDC(m_specific->m_surf_window);
            if(tmpdc == HDC_INVALID)
                return false;

            BitBlt(hdc, 0, 0, 0, 0, tmpdc, 0, 0, 0);
            if ( !GetBitmapFromDC(tmpdc, 
                            0, 0, 240, 320, 
                            m_specific->m_surf_img[idx]) )
#endif
            {
                DeleteCompatibleDC(tmpdc);
                return false;
            }

            DeleteCompatibleDC(tmpdc);
#if 0
            FillBoxWithBitmap(HDC_SCREEN, 350, 0, 0, 0, 
                    m_specific->m_surf_img[idx]);
            //BitBlt(hdc, 0, 0, 0, 0, HDC_SCREEN, 500, 0, true);
#endif

#if 0
            printf("load img: w=%d, h=%d, pitch=%d, pixels=%x\n", 
                    m_specific->m_surf_img[idx]->bmWidth, 
                    m_specific->m_surf_img[idx]->bmHeight, 
                    m_specific->m_surf_img[idx]->bmPitch, 
                    m_specific->m_surf_img[idx]->bmBits);
#endif
            m_rbuf_img[idx].attach(m_specific->m_surf_img[idx]->bmBits, 
                    m_specific->m_surf_img[idx]->bmWidth, 
                    m_specific->m_surf_img[idx]->bmHeight, 
                    m_flip_y?(-m_specific->m_surf_img[idx]->bmPitch):
                            m_specific->m_surf_img[idx]->bmPitch);
            return true;
        }
        return false;
    }

    //------------------------------------------------------------------------
    bool platform_support::load_img(unsigned idx, const char* file)
    {
        if(idx < max_images) {

#ifndef _MG_MINIMALGDI
            if(m_specific->m_surf_img[idx]) 
                UnloadBitmap(m_specific->m_surf_img[idx]);
            else
                m_specific->m_surf_img[idx] = (PBITMAP)calloc(1, sizeof(BITMAP)); 

            if (LoadBitmapFromFile(m_specific->m_surf_window, 
                        m_specific->m_surf_img[idx], file) != 0)
                return false;
#endif
#if 0
            FillBoxWithBitmap(m_specific->m_surf_window, 0, 0, 0, 0, 
                    m_specific->m_surf_img[idx]);
#endif

#if 0
            printf("load img: w=%d, h=%d, pitch=%d, pixels=%x\n", 
                    m_specific->m_surf_img[idx]->bmWidth, 
                    m_specific->m_surf_img[idx]->bmHeight, 
                    m_specific->m_surf_img[idx]->bmPitch, 
                    m_specific->m_surf_img[idx]->bmBits);
#endif
            m_rbuf_img[idx].attach(m_specific->m_surf_img[idx]->bmBits, 
                    m_specific->m_surf_img[idx]->bmWidth, 
                    m_specific->m_surf_img[idx]->bmHeight, 
                    m_flip_y?(-m_specific->m_surf_img[idx]->bmPitch):
                            m_specific->m_surf_img[idx]->bmPitch);
            return true;
        }
        return false;
    }

    //------------------------------------------------------------------------
    bool platform_support::save_img(unsigned idx, const char* file)
    {
        if(idx < max_images)
        {
#if 0
            char fn[1024];
            strcpy(fn, file);
            int len = strlen(fn);
            if(len < 4 || strcmp(fn + len - 4, ".bmp") != 0)
            {
                strcat(fn, ".bmp");
            }
            return SDL_SaveBMP(m_specific->m_surf_img[idx], fn) == 0;
#endif
        }
        return false;
    }


    //------------------------------------------------------------------------
    bool platform_support::create_img(unsigned idx, unsigned width, unsigned height)
    {
        if(idx < max_images) {
            return true;
        }

        return false;
    }
    
    //------------------------------------------------------------------------
    void platform_support::start_timer()
    {
#ifndef _MG_MINIMALGDI
        m_specific->m_sw_start = GetTickCount();
#endif
    }

    //------------------------------------------------------------------------
    double platform_support::elapsed_time() const
    {
#ifndef _MG_MINIMALGDI
        int stop = GetTickCount();
#else
        int stop = 0;
#endif
        return double(stop - m_specific->m_sw_start);
    }

    //------------------------------------------------------------------------
    void platform_support::message(const char* msg)
    {
        fprintf(stderr, "%s\n", msg);
    }

    //------------------------------------------------------------------------
    void platform_support::force_redraw()
    {
        //m_specific->m_update_flag = true;
        //printf("platform_support::force_redraw\n");
        BitBlt(m_specific->m_surf_window, 0, 0, 0, 0, 
                HDC_SCREEN, 0, 0, 0);
        //m_specific->m_update_flag = false;
    }


    //------------------------------------------------------------------------
    void platform_support::on_init() {}
    void platform_support::on_resize(int sx, int sy) {}
    void platform_support::on_idle() {}
    void platform_support::on_mouse_move(int x, int y, unsigned flags) {}
    void platform_support::on_mouse_button_down(int x, int y, unsigned flags) {}
    void platform_support::on_mouse_button_up(int x, int y, unsigned flags) {}
    void platform_support::on_key(int x, int y, unsigned key, unsigned flags) {}
    void platform_support::on_ctrl_change() {}
    void platform_support::on_draw() {}
    void platform_support::on_post_draw(void* raw_handler) {}
}
