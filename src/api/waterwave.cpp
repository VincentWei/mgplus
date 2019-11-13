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
 *   <http://www.minigui.com/blog/minigui-licensing-policy/>.
 */
#include <stdio.h>
#include <minigui/common.h>
#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>
#include <minigui/control.h>

#include "agg_basics.h"
#include "agg_rendering_buffer.h"
#include "agg_renderer_base.h"
#include "agg_rasterizer_scanline_aa.h"
#include "agg_scanline_u.h"
#include "agg_renderer_scanline.h"
#include "agg_rounded_rect.h"
#include "agg_pixfmt_rgba.h"
#include "agg_span_allocator.h"
#include "agg_span_gradient.h"
#include "agg_gsv_text.h"
#include "agg_span_interpolator_linear.h"
#include "agg_platform_support.h"

#include "agg_span_image_filter_rgb.h"
#include "agg_span_interpolator_adaptor.h"
#include "agg_pixfmt_rgb.h"
#include "agg_pixfmt_rgba.h"
#include "agg_image_accessors.h"


#include "mgplus.h"
#include "graphics.h"

typedef agg::rgba8 color;
typedef color::value_type value_type;
typedef agg::order_argb order;
typedef agg::int32u pixel_type;
typedef agg::rendering_buffer rbuf_type;
typedef agg::pixfmt_argb32         pixfmt;

typedef agg::blender_rgba<color, order> prim_blender_type;
typedef agg::pixfmt_alpha_blend_rgba <prim_blender_type, rbuf_type, pixel_type> prim_pixfmt_type;
typedef agg::renderer_base<prim_pixfmt_type> prim_ren_base_type;



static int pBack [480][640] = {0};
static int pV [480][640] = {0};
static int pFront [480][640] = {0};

#define COSDATA_NUM     256

static int g_nSurfWidth = 640;
static int g_nSurfHeight = 480;

static float g_fCosData [COSDATA_NUM + 1];
static float g_ppfSurfHeight [640][480];
static float g_ppbIsFree [640][480];
static float g_ppfSurfSpeed [640][480];
static float g_fDx [640][480];
static float g_fDy [640][480];

static float g_fPullQ = 0.3775;//1.0 / 2;
//static float g_fDampQ = 0.99;//1.0;
static float g_fDampQ = 0.99;

void set_wave_level (float level) 
{

    float pi = 3.1415f * level;
    for(int i = 0; i < COSDATA_NUM; i++)
    {
        g_fCosData[i] = (1 + cos(pi * i / float(COSDATA_NUM))) * 0.5 * (COSDATA_NUM - i) / COSDATA_NUM;
    }
}


void Placate (void)
{       

    int x;
    for(x = 0; x < g_nSurfWidth; x++)
    {
        memset (&g_ppfSurfHeight [x][0], 0, sizeof(float) * g_nSurfHeight);
        memset (&g_ppfSurfSpeed [x][0] , 0, sizeof(float) * g_nSurfHeight);
        memset (&g_ppbIsFree [x][0], 1, sizeof(BOOL) * g_nSurfHeight);
    }
}

void Evolve (int n);
void Disturb (int x, int y, int radius, float strength)
{
    x++, y++;
    int x0 = x - radius, y0 = y - radius, x1 = x + radius, y1 = y + radius;
    if(x0 < 1)
    {
        x0 = 1;
    }
    if(y0 < 1)
    {
        y0 = 1;
    }
    if(x1 > g_nSurfWidth - 2)
    {
        x1 = g_nSurfWidth - 2;
    }
    if(y1 > g_nSurfHeight - 2)
    {
        y1 = g_nSurfHeight - 2;
    }

    int i, j, cosIndex;
    for (i = x0; i < x1; i++)
    {
        for (j = y0; j < y1; j++)
        {
            if (g_ppbIsFree[i][j])
            {
                cosIndex =
                    int((sqrt (float((i - x) * (i - x) + (j - y) * (j - y))) / radius)
                            * COSDATA_NUM);
                if (cosIndex <= COSDATA_NUM)
                {
                    g_ppfSurfHeight[i][j] -=
                        int(g_fCosData [cosIndex] * strength);
                }
            }
        }
    }
}


void Evolve (int n)
{
    int x, y;
    int i;
    float x0, y0;

    for(i = 0; i < n; i++)
    {
        for(x = 1; x < g_nSurfWidth - 1; x++)
        {
            for(y = 1; y < g_nSurfHeight - 1; y++)
            {
                g_ppfSurfSpeed [x][y] += (g_ppfSurfHeight[x][y - 1] +
                        g_ppfSurfHeight[x - 1][y] +
                        g_ppfSurfHeight[x][y + 1] +
                        g_ppfSurfHeight[x + 1][y] -
                        g_ppfSurfHeight[x][y] -
                        g_ppfSurfHeight[x][y] -
                        g_ppfSurfHeight[x][y] -
                        g_ppfSurfHeight[x][y]) * g_fPullQ;
            }
        }

        for (x = 1; x < g_nSurfWidth - 1; x++)
        {
            for (y = 1; y < g_nSurfHeight - 1; y++)
            {
                if (g_ppbIsFree[x][y])
                {
                    g_ppfSurfHeight [x][y] =
                        (g_ppfSurfHeight[x][y] + g_ppfSurfSpeed [x][y])
                        * g_fDampQ;
                }
            }
        }

    }

    x0 = 0;
    for(x = 0; x < g_nSurfWidth - 2; x++)
    {
        y0 = 0;
        for(y = 0; y < g_nSurfHeight - 2; y++)
        {
            g_fDx[x][y] = x0
                + g_ppfSurfHeight[x][y + 1] - g_ppfSurfHeight[x + 2][y + 1];
            g_fDy[x][y] = y0
                + g_ppfSurfHeight[x + 1][y] - g_ppfSurfHeight[x + 1][y + 2];
            y0 += 1;
        }
        x0 += 1;
    }
#if 0 
    printf ("Dx:\n");
    for (x = 0; x < g_nSurfWidth - 1; x ++)
    {
        printf ("%d : ", x);
        for(y = 0; y < g_nSurfHeight - 1; y++)
        {
            printf ("%f ", g_fDx[x][y]);
        }
        printf ("\n");
    }

    printf ("Dy:\n");
    for (x = 0; x < g_nSurfWidth - 1; x ++)
    {
        printf ("%d : ", x);
        for(y = 0; y < g_nSurfHeight - 1; y++)
        {
            printf ("%f ", g_fDx[x][y]);
        }
        printf ("\n");
    }
#endif
}

#define G_PI 3.14159265358979323846f

static double g_amplitude = 50;
static double g_wavelength = 50;
static double g_radial = 0;

inline void calculate_wave(int* x, int* y,
        double cx, double cy,
        double m_period, double m_amplitude, double m_phase, double m_radial)
{   
#if 0
#if 1 
    const float W = 1.0f;
    int i, j, X, Y;
    float a = 0, b = 0, c = 0, d = 0, A = 0, B = 0, C = 0, D = 0;
    int tx, ty;

    double xd = double(*x) / agg::image_subpixel_scale;
    double yd = double(*y) / agg::image_subpixel_scale;

    X = (int) (xd);
    Y = (int) (yd);
    //printf ("X %d, Y %d\n", X, Y);
#if 0
    x0 = X0;
    y0 = Y0;
    A = g_fDx[X + 1][Y] - g_fDx[X][Y];
    a = A * W;
    B = g_fDx[X][Y + 1] - g_fDx[X][Y];
    b = B * W;
    C = g_fDy[X + 1][Y] - g_fDy[X][Y];
    c = C * W;
    D = g_fDy[X][Y + 1] - g_fDy[X][Y];
    d = D * W;
    B = 0;
    D = 0;
    A = 0;
    C = 0;
#endif
    A = g_fDx[X + 1][Y] - g_fDx[X][Y];
    B = g_fDx[X][Y + 1] - g_fDx[X][Y];
    C = g_fDy[X + 1][Y] - g_fDy[X][Y];
    D = g_fDy[X][Y + 1] - g_fDy[X][Y];
    tx = int (g_fDx[X][Y] + A + B);
    ty = int (g_fDy[X][Y] + C + D);

    if ((tx > 0 && tx < 640) && (ty > 0 && ty < 480))
    {
        //*x = int((tx) * agg::image_subpixel_scale);
        //*y = int((ty)* agg::image_subpixel_scale);
        *x = int((tx) * agg::image_subpixel_scale);
        *y = int((ty)* agg::image_subpixel_scale);
    }
#endif

#if 0 
    int n_x, n_y;
    int xoff, yoff, nx, ny;
    double tmp_x, tmp_y;
    tmp_x = double (*x) / agg::image_subpixel_scale;
    tmp_y = double (*y) / agg::image_subpixel_scale;

    //printf ("(tmp_x, tmp_y): %f, %f\n", tmp_x, tmp_y);
    n_x = (int)tmp_x;
    n_y = (int)tmp_y;
    //printf ("(x, y): %d, %d\n", x, y);

    xoff = pFront [n_y][n_x+1] - pFront [n_y][n_x-1];
    yoff = pFront [n_y-1][n_x] - pFront [n_y+1][n_x];
    //printf ("xoff %d, yoff %d\n", xoff, yoff);
    nx = n_x + xoff / 10;
    ny = n_y + yoff / 10;
    //printf ("nx %d, ny %d\n", nx, ny);
    if (!(nx >= 640 || nx < 0 || ny >= 480 || ny < 0))
    {
        *x = int((nx + 10) * agg::image_subpixel_scale);
        *y = int((ny + 2)* agg::image_subpixel_scale);
    }
#endif
#if  0
    double xd = double(*x) / agg::image_subpixel_scale - cx;
    double yd = double(*y) / agg::image_subpixel_scale - cy;
    double d = sqrt(xd*xd + yd*yd);
    //printf ("xd %f, yd %f, d %f, m_radial %f\n", xd, yd, d, m_radial);
    if(d > 1 && d > (m_radial / 2 ) && (xd < 320 || xd > -320) && (yd < 120 || yd > -120))
    {
        double a = cos(d / (16.0 * period) - phase) * (1.0 / (amplitude * d)) + 1.0;
        //double a = (1.0 / (amplitude * d)) + 1.0;
        *x = int((xd * a + cx) * agg::image_subpixel_scale);
        *y = int((yd * a + cy) * agg::image_subpixel_scale);
    }
#endif
#endif
    double amplitude;
    double wavelength;
    double phase;
    double amnt;
    double needx, needy;
    double cen_x, cen_y;
    cen_x = 320;
    cen_y = 240;
    double dx = double(*x) / agg::image_subpixel_scale - cen_x;
    double dy = double(*y) / agg::image_subpixel_scale - cen_y;
    double d = sqrt (dx*dx + dy*dy);
    wavelength = g_wavelength * 2;
    phase = 0;

    if (g_amplitude < 0.0)
        amplitude = 0.0;//g_amplitude;
    else
        amplitude = g_amplitude;

    if (d < g_radial)
        return;

    {
        amnt = amplitude * sin (((d / wavelength) * (2.0 * G_PI) +
                    phase));

        needx = (amnt + dx) + cen_x;
        needy = (amnt + dy) + cen_y;
    }

    if ((needx > 0 && needx < 640) && (needy > 0 && needy < 480))
    {
        *x = int((needx) * agg::image_subpixel_scale);
        *y = int((needy) * agg::image_subpixel_scale);
    }

#if 0
    //printf ("xd %f, yd %f, d %f, m_radial %f\n", xd, yd, d, m_radial);
    if(d > 1 && d > (m_radial / 2 ) && (xd < 320 || xd > -320) && (yd < 120 || yd > -120))
    {
        double a = cos(d / (16.0 * period) - phase) * (1.0 / (amplitude * d)) + 1.0;
        //double a = (1.0 / (amplitude * d)) + 1.0;
        *x = int((xd * a + cx) * agg::image_subpixel_scale);
        *y = int((yd * a + cy) * agg::image_subpixel_scale);
    }
#endif
}

class periodic_distortion
{
    public:
        periodic_distortion() :
            m_cx(0.0),
            m_cy(0.0),
            m_period(0.5),
            m_amplitude(0.5), 
            m_phase(0.0),
            m_radial (0.0)
    {}

        void center(double x, double y) { m_cx = x; m_cy = y; }
        void period(double v)           { m_period = v; }
        void amplitude(double v)        { m_amplitude = 1.0 / v; }
        void phase(double v)            { m_phase = v; }
        void radial(double v)            { m_radial = v; }

        virtual void calculate(int* x, int* y) const = 0;

    protected:
        double m_cx;
        double m_cy; 
        double m_period;
        double m_amplitude;
        double m_phase;
        double m_radial;
};




class distortion_wave : public periodic_distortion
{   
    virtual void calculate(int* x, int* y) const
    {
        calculate_wave (x, y, m_cx, m_cy, m_period, m_amplitude, m_phase, m_radial);
    }
};

/**
 * \fn MPStatus MGPlusWaterWaveFlameInit (void)
 * \brief Init the water wave data.
 *
 * This function inits the water wave data. 
 * 
 * \return Get Status, MP_GENERIC_ERROR indicates an error.
 */ 

MPStatus MGPlusWaterWaveFlameInit (void)
{
    Placate ();
    set_wave_level (1);
    Disturb (320, 240, 35, 4000.0);
    return MP_OK;
}

static double g_dphase = 0;
/**
 * \fn MPStatus MGPlusWaterWaveFlame (HGRAPHICS graphics, int n_src_index, 
       int n_dst_index, int nFlame, int nTotalFlame)
 * \brief Get the water wave flame.
 *
 * This function get the water wave flame. 
 * 
 * \param graphics           the graphic handler. 
 * \param n_src_index        the src bitmap index. 
 * \param n_dst_index        the dst bitmap index. 
 * \param nFlame             the count of the total flame.
 * \param nTotalFlame        the total flame. 
 *
 * \return Get Status, MP_GENERIC_ERROR indicates an error.
 */ 
MPStatus MGPlusWaterWaveFlame (HGRAPHICS graphics, int n_src_index, 
        int n_dst_index, int nFlame, int nTotalFlame)
{
    MPGraphics* p_graphics=NULL;
    unsigned int n_alpha = nFlame * 255 / nTotalFlame;

    g_dphase += 15.0 * agg::pi / 180.0;

#if 1 
    if (g_dphase > agg::pi * 200.0)
        g_dphase -= agg::pi * 200.0;
#endif

#if 0
    g_amplitude = g_amplitude - 2;// - (double)(nFlame * 50 / nTotalFlame);
    //g_wavelength = g_wavelength + 1;// - (double)(nFlame * 50 / nTotalFlame);
    //g_wavelength = g_wavelength + (double)(nFlame * 30 * 2 / nTotalFlame);
    g_radial = g_radial + 20;
    g_wavelength = g_wavelength + 1;
#endif
    g_amplitude = 50 - 2 * nFlame;// - (double)(nFlame * 50 / nTotalFlame);
    g_wavelength = 50 + 2 * nFlame;
    g_radial = 20 * nFlame;

    p_graphics = (MPGraphics *) graphics;

    if (!graphics || n_src_index < 0 || n_src_index >= MGPLUS_MAX_GRAPHIC_BITMAPNUM
        || n_src_index < 0 || n_src_index >= MGPLUS_MAX_GRAPHIC_BITMAPNUM)
        return MP_GENERIC_ERROR;

    if (p_graphics->surf_img [n_src_index] == NULL)
        return MP_GENERIC_ERROR;

    if (p_graphics->surf_img [n_dst_index] == NULL)
        return MP_GENERIC_ERROR;

    prim_pixfmt_type pixf (p_graphics->rendering_buff);
    prim_ren_base_type rb (pixf);
    rb.clear (agg::rgba8 (0, 0, 0));

    typedef agg::span_allocator<agg::rgba8> span_alloc_type;
    span_alloc_type sa;


    typedef agg::span_interpolator_adaptor<agg::span_interpolator_linear<>,
            periodic_distortion> interpolator_type;

    agg::rasterizer_scanline_aa<> ras;
    agg::scanline_u8 sl;
    {
        typedef agg::comp_op_adaptor_rgba<color, order> blender_type;
        typedef agg::pixfmt_custom_blend_rgba<blender_type, rbuf_type> pixfmt_type;
        typedef agg::renderer_base<pixfmt_type> renderer_type;
        typedef agg::rect_base<int>    rect_i; 

        rect_i rect;

        rect.x1 = 0;
        rect.y1 = 0;
        rect.x2 = p_graphics->width;
        rect.y2 = p_graphics->height;

        pixfmt_type ren_pixf (p_graphics->rendering_buff);
        renderer_type renderer (ren_pixf);

        agg::renderer_base<prim_pixfmt_type> rb(pixf);

        rb.blend_from (prim_pixfmt_type (p_graphics->rendering_img [n_src_index]),
                &rect, 0, 0,
                n_alpha);

#if 1 
        rb.blend_from (prim_pixfmt_type (p_graphics->rendering_img [n_dst_index]),
                &rect, 0, 0,
                unsigned (256 - n_alpha));
#endif
    }


    if (nFlame != nTotalFlame)
    {

        pixfmt img_pixf(p_graphics->rendering_buff);

        typedef agg::renderer_base<pixfmt> renderer_base;

        agg::trans_affine src_mtx;
        double img_width = p_graphics->width;
        double img_height = p_graphics->height;

        double d_radial = 110 + nFlame * 115;
        agg::trans_affine img_mtx;

        typedef agg::span_interpolator_adaptor<agg::span_interpolator_linear<>,
                periodic_distortion> interpolator_type;

        periodic_distortion*  dist = 0;
        distortion_wave       dist_wave;

        dist = &dist_wave;

        dist->period (1.0);
        dist->amplitude (10.0);
        dist->phase (0.0);
        dist->radial (d_radial / 2);
        double cx = p_graphics->width / 2;
        double cy = p_graphics->height / 2;
        pBack [1][1] = 11;
        //img_mtx.transform (&cx, &cy);
        dist->center (cx, cy);

        int m;
        //for (m = 0; m < 10; m++)
        Evolve (10);

        interpolator_type interpolator(img_mtx, *dist);

        typedef agg::image_accessor_clip<pixfmt> img_source_type;
        img_source_type img_src (img_pixf, agg::rgba(0,0,0, n_alpha));

        typedef agg::span_image_filter_rgb_nn<img_source_type,
                interpolator_type> span_gen_type;
        span_gen_type sg(img_src, interpolator);

        agg::path_storage p_st;

        p_st.remove_all ();
        p_st.move_to (0, 0);
        p_st.line_to (640, 0);
        p_st.line_to (640, 480);
        p_st.line_to (0, 480);

        agg::conv_transform<agg::path_storage> tr(p_st, src_mtx);
        ras.add_path(tr);
        agg::render_scanlines_aa(ras, sl, rb, sa, sg);
    }

    return MP_OK;
}


