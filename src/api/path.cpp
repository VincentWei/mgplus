/*
 ** $Id: path.cpp 10885 2008-12-05 12:01:39Z dengkexi$
 **
 ** graphics.cpp: Implementation of create and add path.
 ** This file includes macro definitions, typedefs and function interfaces of 
 ** mgplus component. mGPlus includes path, gradient filling and color composite.
 **
 ** Copyright (C) 2003 ~ 2008 Feynman Software.
 ** Copyright (C) 2000 ~ 2002 Wei Yongming.
 **
 ** Create date: 2008/12/02
 */

#include "path.h"

#define SAFE_CHECK_PARAMETER(pointer)  \
    if(!pointer) return MP_INVALID_PARAMETER;

BOOL is_path_closed (MPPath* path)
{
    double x, y;
    unsigned int flag;

    if (!path->m_agg_ps.total_vertices ())
        return TRUE;

    flag = path->m_agg_ps.last_vertex (&x, &y);

    if ((flag & agg::path_flags_close) || (flag == agg::path_flags_none))
        return TRUE;

    return FALSE;
}


MPStatus 
MGPlusPathSetTransform (HPATH path, MPMatrix *matrix)
{
    SAFE_CHECK_PARAMETER (path);
    SAFE_CHECK_PARAMETER (matrix);

    if (matrix->sx == 0 && matrix->shy == 0 &&
            matrix->shx == 0 && matrix->sy == 0)
        return MP_INVALID_PARAMETER;

    double m_mtx [6] = 
    {
        matrix->sx, matrix->shy, matrix->shx, 
        matrix->sy, matrix->tx, matrix->ty
    };

    MPPath* m_path = (MPPath*) path;
    m_path->matrix.load_from (m_mtx);
    return MP_OK;
}

MPStatus 
MGPlusPathGetTransform (HPATH path, MPMatrix* matrix)
{
    SAFE_CHECK_PARAMETER (path);
    SAFE_CHECK_PARAMETER (matrix);

    if (matrix->sx == 0 && matrix->shy == 0 &&
            matrix->shx == 0 && matrix->sy == 0)
        return MP_INVALID_PARAMETER;

    MPPath* m_path = (MPPath*) path;
    m_path->matrix.store_to ((double*)matrix);
    return MP_OK;
}

MPStatus 
MGPlusPathResetTransform (HPATH path)
{
    SAFE_CHECK_PARAMETER (path);
    MPPath* m_path = (MPPath*) path;

    m_path->matrix.reset ();

    return MP_OK;
}

MPStatus 
MGPlusPathScale (HPATH path, float sx, float sy)
{
    SAFE_CHECK_PARAMETER (path);
    MPPath* m_path = (MPPath*) path;

    m_path->matrix.scale(sx, sy);
    return MP_OK;
}

MPStatus 
MGPlusPathRotate(HPATH path, float angle)
{
    SAFE_CHECK_PARAMETER (path);
    MPPath* m_path = (MPPath*) path;

    m_path->matrix.rotate(angle * agg::pi / 180.0);
    return MP_OK;
}

MPStatus 
MGPlusPathTranslate(HPATH path, float dx, float dy)
{
    SAFE_CHECK_PARAMETER (path);
    MPPath* m_path = (MPPath*) path;

    m_path->matrix.translate(dx, dy);
    return MP_OK;
}

MPStatus 
MGPlusPathTransform (HPATH path)
{
    MPPath *m_path = (MPPath *)path;

    if (!m_path)
        return MP_GENERIC_ERROR;

    agg::conv_transform<agg::path_storage> tr(m_path->m_agg_ps, m_path->matrix);

    agg::path_storage m_agg_ps; 
    m_agg_ps.remove_all();
    //m_agg_ps.join_path(tr);
    m_agg_ps.concat_path(tr);

    m_path->m_agg_ps.remove_all();
    m_path->m_agg_ps.concat_path(m_agg_ps);

    return MP_OK;
}

HPATH MGPlusPathCreate (MPFillMode brushMode)
{
    MPPath* m_path = NULL;

    m_path = new MPPath; 

    if (!m_path)
        return MP_INV_HANDLE;

    m_path->m_fill_mode = brushMode;
    m_path->m_agg_ps.remove_all ();               // AGG store path
    m_path->matrix.reset () ;
    m_path->id = 0;

    return (HPATH)m_path;
}

MPStatus MGPlusPathDelete (HPATH path)
{
    MPPath* m_path = (MPPath*) path;

    if (!m_path)
        return MP_GENERIC_ERROR;

    delete (m_path);

    return MP_OK;
}

MPStatus MGPlusPathStartFigure (HPATH path)
{
    MPPath* m_path = (MPPath*) path;

    if (!path)
        return MP_GENERIC_ERROR;

    if (!m_path->m_agg_ps.total_vertices ())
        return MP_OK;

    if (m_path->id >= MAX_PATHID)
        return MP_NOT_ENOUGH_FIGURE_MEMORY;

    m_path->path_id [m_path->id] = m_path->m_agg_ps.start_new_path ();
    m_path->id ++;
    return MP_OK;
}

MPStatus MGPlusPathCloseFigure (HPATH path)
{
    MPPath* m_path = (MPPath*) path;

    if (!m_path)
        return MP_GENERIC_ERROR;

    m_path->m_agg_ps.close_polygon ();
    return MP_OK;
}

MPStatus MGPlusPathReset (HPATH path)
{
    MPPath* m_path = (MPPath*) path;

    if (!m_path)
        return MP_GENERIC_ERROR;

    m_path->m_agg_ps.remove_all ();
    m_path->matrix.reset ();
    return MP_OK;
}

MPStatus MGPlusPathGetPointCount (HPATH path, int* count)
{
    MPPath* m_path = (MPPath*) path;

    if (!m_path || !count)
        return MP_GENERIC_ERROR;

    (*count) = m_path->m_agg_ps.total_vertices();
    return MP_OK;
}

MPStatus MGPlusPathGetPoints (HPATH path, int* count, MPPOINT** pt)
{
    int i = 0;
    MPPath* m_path = (MPPath*) path;
    MPPOINT* m_pt = NULL;

    if (!m_path || !count || !pt)
        return MP_GENERIC_ERROR;

    (*count) = m_path->m_agg_ps.total_vertices();
    m_pt = *pt = new MPPOINT[*count];

    if (!m_pt)
        return MP_GENERIC_ERROR;

    for (i = 0; i < *count; i++) {
        double x = 0, y = 0;
        m_path->m_agg_ps.vertex(&x, &y);
        m_pt[i].x = x;
        m_pt[i].y = y;
    } 

    return MP_OK;
}

MPStatus MGPlusPathGetVertex (HPATH path, int idx, double* x, double* y, int* cmd)
{
    MPPath* m_path = (MPPath*) path;

    if (idx < 0 || !cmd
            || (unsigned)idx > m_path->m_agg_ps.total_vertices())
        return MP_GENERIC_ERROR;

    *cmd = m_path->m_agg_ps.vertex(idx, x, y);

    return MP_OK;
}

MPStatus MGPlusPathAddLines (HPATH path, const MPPOINT* points, int count)
{
    MPPath* m_path = (MPPath*) path;

    if (!m_path || !points || count < 1)
        return MP_GENERIC_ERROR;

    double x,y;
    x = (double) points [0].x;
    y = (double) points [0].y;

    m_path->matrix.transform (&x, &y);

    if (is_path_closed (m_path))
        m_path->m_agg_ps.move_to (x, y);
    else
        m_path->m_agg_ps.line_to (x, y);

    int cur_num = 1;

    while (cur_num < count)
    {
        x = (double)points [cur_num].x;
        y = (double)points [cur_num].y;

        m_path->matrix.transform (&x, &y);
        m_path->m_agg_ps.line_to (x, y);
        cur_num ++;
    }

    return MP_OK;
}



MPStatus MGPlusPathAddLine (HPATH path, float x1, float y1, 
        float x2, float y2)
{
    if (!path)
        return MP_GENERIC_ERROR;

    MPPOINT points[2];

    points[0].x = x1;
    points[0].y = y1;

    points[1].x = x2;
    points[1].y = y2; 

    return MGPlusPathAddLines (path, points, 2);
}

MPStatus MGPlusPathAddLineI (HPATH path, int x1, int y1,
        int x2, int y2)
{
    return MGPlusPathAddLine (path, (float) x1, (float) y1,
            (float) x2, (float) y2);
}

MPStatus MGPlusPathAddArc (HPATH path, float cx, float cy, float rx, 
                 float ry, float start_angle, float sweep_angle)
{
    MPPath* m_path = (MPPath*) path;

    if (!path || rx < 1 || ry < 1 || sweep_angle == 0)
        return MP_GENERIC_ERROR;

#if 0
    agg::bezier_arc ell (cx, cy, rx, ry,
            agg::deg2rad (start_angle - 180), agg::deg2rad (sweep_angle));
    agg::conv_curve <agg::bezier_arc> arc (ell);
    arc.approximation_scale(10.0);

#endif
    //agg::arc ell (cx, cy, rx, ry, agg::deg2rad (start_angle - 180),
    //        agg::deg2rad (start_angle - 180 + sweep_angle));
    agg::arc ell;
    if (sweep_angle > 0)
    {
        ell.init(cx, cy, rx, ry, (start_angle - 180)*agg::pi/180,
                (start_angle + sweep_angle - 180)*agg::pi/180);
    }
    else
    {
        ell.init(cx, cy, rx, ry, (start_angle - 180)*agg::pi/180,
                (start_angle + sweep_angle - 180)*agg::pi/180, false);
    }
    agg::conv_curve <agg::arc> arc (ell);
    arc.approximation_scale(10.0);

    agg::path_storage path_arc;
    path_arc.join_path (arc, 0);
    path_arc.transform (m_path->matrix);

    if (is_path_closed (m_path))
        m_path->m_agg_ps.concat_path (path_arc, 0);
    else
        m_path->m_agg_ps.join_path (path_arc, 0);
    return MP_OK;
}

MPStatus MGPlusPathAddArcI (HPATH path, int cx, int cy, int rx, 
        int ry, int startAngle, int sweepAngle)
{
    return MGPlusPathAddArc (path, (float)cx, (float)cy, (float)rx, 
            (float)ry, (float)startAngle, (float)sweepAngle);
}


MPStatus MGPlusPathAddBezier (HPATH path, float x1, float y1, float x2, 
        float y2, float x3, float y3, float x4, float y4)
{
    MPPath* m_path = (MPPath*) path;

    if (!path)
        return MP_GENERIC_ERROR;

    double d_x [4], d_y[4];
    d_x [0] = (double)x1;
    d_y [0] = (double)y1;

    d_x [1] = (double)x2;
    d_y [1] = (double)y2;

    d_x [2] = (double)x3;
    d_y [2] = (double)y3;

    d_x [3] = (double)x4;
    d_y [3] = (double)y4;

    m_path->matrix.transform (&d_x [0], &d_y [0]);
    m_path->matrix.transform (&d_x [1], &d_y [1]);
    m_path->matrix.transform (&d_x [2], &d_y [2]);
    m_path->matrix.transform (&d_x [3], &d_y [3]);

    agg::curve4 bezier;
    bezier.init (d_x[0], d_y[0], d_x[1], d_y[1], \
            d_x[2], d_y[2], d_x[3], d_y[3]);

    if (is_path_closed (m_path))
        m_path->m_agg_ps.concat_path (bezier, 0);
    else
        m_path->m_agg_ps.join_path (bezier, 0);
    return MP_OK;
}

MPStatus MGPlusPathAddBezierI (HPATH path, int x1, int y1, int x2, 
        int y2, int x3, int y3, int x4, int y4)
{
    return MGPlusPathAddBezier (path, (float) x1, (float) y1, (float) x2,
            (float) y2, (float) x3, (float) y3, (float) x4, (float) y4);
}

MPStatus MGPlusPathAddRectangle (HPATH path, float x, float y,
        float width, float height)
{
    MPPath* m_path = (MPPath*) path;

    if (!path)
        return MP_GENERIC_ERROR;

#if 0
    double d_x, d_y;

    d_x = (double) x;
    d_y = (double) y;
    m_path->matrix.transform (&d_x, &d_y);
    m_path->m_agg_ps.move_to (d_x, d_y);

    d_x = (double) (x + width);
    d_y = (double) y;
    m_path->matrix.transform (&d_x, &d_y);
    m_path->m_agg_ps.line_to (d_x, d_y);

    d_x = (double) (x + width);
    d_y = (double) (y + height);
    m_path->matrix.transform (&d_x, &d_y);
    m_path->m_agg_ps.line_to (d_x, d_y);

    d_x = (double) x;
    d_y = (double) (y + height);
    m_path->matrix.transform (&d_x, &d_y);
    m_path->m_agg_ps.line_to (d_x, d_y);

    d_x = (double) x;
    d_y = (double) y;
    m_path->matrix.transform (&d_x, &d_y);
    m_path->m_agg_ps.line_to (d_x, d_y);
#endif
#if 0
    agg::rounded_rect <int> rect;

    rect.x1 = (int)x;
    rect.y1 = (int)y;
    rect.x2 = (int)(x + width);
    rect.y2 = (int)(x + height);

    agg::path_storage path_rect;
    path_rect.concat_path (rect, 0);
    path_rect.transform (m_path->matrix);

    m_path->m_agg_ps.concat_path (path_rect, 0);
#endif
    //agg::rounded_rect round_rect;
    //round_rect.rect (x, y, x + width, y + height);
    //round_rect.radius (0);
    agg::rounded_rect round_rect (x, y, x + width, y + height, 0);
    round_rect.normalize_radius();

    agg::path_storage path_roundrect;
    path_roundrect.concat_path (round_rect, 0);
    path_roundrect.transform (m_path->matrix);

    if (is_path_closed (m_path))
        m_path->m_agg_ps.concat_path (path_roundrect);
    else
        m_path->m_agg_ps.join_path (path_roundrect);

    return MP_OK;
}

MPStatus MGPlusPathAddRectangleI (HPATH path, int x, int y, 
        int width, int height)
{
    return MGPlusPathAddRectangle (path, (float) x, (float) y,
            (float) width, (float) height); 
}

MPStatus MGPlusPathLineto (HPATH path, float x, float y)
{
    SAFE_CHECK_PARAMETER(path)
    MPPath* m_path = (MPPath*) path;
    double d_x, d_y;
    d_x = (double)x;
    d_y = (double)y;

    m_path->matrix.transform (&d_x, &d_y);
    m_path->m_agg_ps.line_to (d_x, d_y);
    return MP_OK;
}

MPStatus MGPlusPathLinetoI (HPATH path, int x, int y) 
{
    return MGPlusPathLineto (path, (float)x, (float)y);
}


MPStatus MGPlusPathQuadraticto (HPATH path, float x1, float y1, float x2, float y2)
{
    MPPath* m_path = (MPPath*) path;

    if (!path)
        return MP_GENERIC_ERROR;

    double d_x [3], d_y[3];
    double d_last_x, d_last_y;
    //unsigned int flag;

    if (!m_path->m_agg_ps.total_vertices ())
        return MP_GENERIC_ERROR;

    m_path->m_agg_ps.last_vertex (&d_last_x, &d_last_y);

    d_x[0] = d_last_x + (2.0 / 3.0) * (x1 - d_last_x);
    d_y[0] = d_last_y + (2.0 / 3.0) * (y1 - d_last_y);

    d_x[1] = d_x[0] + (1.0 / 3.0) * (x2 - d_last_x);
    d_y[1] = d_y[0] + (1.0 / 3.0) * (y2 - d_last_y);

    d_x [2] = (double)x2;
    d_y [2] = (double)y2;

    m_path->matrix.transform (&d_x [0], &d_y [0]);
    m_path->matrix.transform (&d_x [1], &d_y [1]);
    m_path->matrix.transform (&d_x [2], &d_y [2]);

    if (!m_path->m_agg_ps.total_vertices ())
        return MP_GENERIC_ERROR;

    m_path->m_agg_ps.last_vertex (&d_last_x, &d_last_y);

    agg::curve4 bezier;
    bezier.init (d_last_x, d_last_y, d_x[0], d_y[0], \
            d_x[1], d_y[1], d_x[2], d_y[2]);
    m_path->m_agg_ps.join_path (bezier);

    return MP_OK;
}

MPStatus MGPlusPathQuadratictoI (HPATH path, int x1, int y1, int x2, int y2) 
{
    return MGPlusPathQuadraticto (path, (float)x1, (float)y1, (float)x2, (float)y2);
}


MPStatus MGPlusPathBezierto (HPATH path, float x1, float y1, float x2, float y2, float x3, float y3)
{
    MPPath* m_path = (MPPath*) path;

    if (!path)
        return MP_GENERIC_ERROR;

    double d_x [3], d_y[3];
    double d_last_x, d_last_y;
    //unsigned int flag;

    d_x [0] = (double)x1;
    d_y [0] = (double)y1;

    d_x [1] = (double)x2;
    d_y [1] = (double)y2;

    d_x [2] = (double)x3;
    d_y [2] = (double)y3;

    m_path->matrix.transform (&d_x [0], &d_y [0]);
    m_path->matrix.transform (&d_x [1], &d_y [1]);
    m_path->matrix.transform (&d_x [2], &d_y [2]);

    if (!m_path->m_agg_ps.total_vertices ())
        return MP_GENERIC_ERROR;

    m_path->m_agg_ps.last_vertex (&d_last_x, &d_last_y);

    agg::curve4 bezier;
    bezier.init (d_last_x, d_last_y, d_x[0], d_y[0], \
            d_x[1], d_y[1], d_x[2], d_y[2]);
    m_path->m_agg_ps.join_path (bezier);

    return MP_OK;
}

MPStatus MGPlusPathBeziertoI (HPATH path, int x1, int y1, int x2, int y2, int x3, int y3) 
{
    return MGPlusPathBezierto (path, (float)x1, (float)y1, (float)x2, (float)y2, (float)x3, (float)y3);
}


#if 0
inline bool almost_equal(float A, float B, int maxUlps = 100)
{
    if (A==B) return true;

    int aInt = *(int*)&A;
    if (aInt < 0)
        aInt = 0x80000000 - aInt;

    int bInt = *(int*)&B;
    if (bInt < 0)
        bInt = 0x80000000 - bInt;
    int intDiff = aInt - bInt;
    if (intDiff < 0)
        intDiff = -intDiff;
    if (intDiff <= maxUlps)
        return true;
    return false;
}
#else
union Float_t
{
    Float_t (float num = 0.0f) : f(num) {}
    // Portable extraction of components.
    bool Negative() const { return (i >> 31) != 0; }
    int32_t RawMantissa() const { return i & ((1 << 23) - 1); }
    int32_t RawExponent() const { return (i >> 23) & 0xFF; }

    int32_t i;
    float f;
};

inline bool almost_equal (float A, float B, int maxUlpsDiff = 100)
{
    Float_t uA(A);
    Float_t uB(B);

    // Different signs means they do not match.
    if (uA.Negative() != uB.Negative()) {
        // Check for equality to make sure +0==-0
        if (A == B)
            return true;
        return false;
    }

    // Find the difference in ULPs.
    int ulpsDiff = abs(uA.i - uB.i);
    if (ulpsDiff <= maxUlpsDiff)
        return true;

    return false;
}
#endif

MPStatus MGPlusPathArcto (HPATH path, double x1, double y1, double x2, double y2, double radius)
{
    double x0, y0;
    MPPath* m_path = (MPPath*) path;

    if (m_path->m_agg_ps.last_vertex(&x0, &y0) == agg::path_cmd_stop) {
        return MP_INVALID_PARAMETER;
    }
    m_path->matrix.inverse_transform(&x0, &y0);

    agg::trans_affine_translation xform(-x1, -y1);
    double xform_angle = atan2(y0-y1, x0-x1);
    if (!almost_equal(fmod(xform_angle, 2*agg::pi), 0.0))
    {
        xform *= agg::trans_affine_rotation(xform_angle);
    }

    xform.transform(&x0, &y0);
    xform.transform(&x2, &y2);
    xform.transform(&x1, &y1);

    double center_angle = atan2(y2, x2) / 2;
    bool sweep_flag = (center_angle >= 0) ? false : true;
    double hypotenuse = fabs(radius / sin(center_angle));
    double cx = hypotenuse * cos(center_angle);
    /// double cy = hypotenuse * sin(center_angle);

    if (!almost_equal(x0, cx))
    {
        x0 = cx;
        xform.inverse_transform(&x0, &y0);
        m_path->m_agg_ps.line_to(x0, y0);
    }
    else
    {
        xform.inverse_transform(&x0, &y0);
    }

    double point2_scale = cx / sqrt(x2*x2 + y2*y2);
    x2 *= point2_scale;
    y2 *= point2_scale;
    xform.inverse_transform(&x2, &y2);

    agg::bezier_arc_svg aggarc(x0, y0, radius, radius, 0.0, false, sweep_flag, x2, y2);

    int numverts = aggarc.num_vertices();
    double *vertices = aggarc.vertices();
    double *v = NULL;
    for (int i = 0; i <= numverts/2; i++)
    {
        v = vertices + i*2;
        m_path->matrix.transform(v, v+1);
    }

    agg::conv_curve <agg::bezier_arc_svg> arc(aggarc);
    arc.approximation_scale(10.0);
    m_path->m_agg_ps.join_path(arc, 0);
    return MP_OK;
}

MPStatus MGPlusPathArctoI (HPATH path, int x1, int y1, int x2, int y2, int radius)
{
    return MGPlusPathArcto (path, (double)x1, (double)y1, (double)x2, (double)y2, (double)radius);
}

MPStatus MGPlusPathMoveto (HPATH path, float x, float y)
{
    SAFE_CHECK_PARAMETER(path)
    MPPath* m_path = (MPPath*) path;
    double d_x, d_y;
    d_x = (double)x;
    d_y = (double)y;

    //count = m_path->m_agg_ps.total_vertices();

    //if (count)
    //    m_path->m_agg_ps.modify_command (count - 1, agg::path_cmd_stop);
    m_path->matrix.transform (&d_x, &d_y);
    m_path->m_agg_ps.move_to (d_x, d_y);
    return MP_OK;
}

MPStatus MGPlusPathMovetoI (HPATH path, int x, int y) 
{
    return MGPlusPathMoveto (path, (float) x, (float) y);
}

MPStatus MGPlusPathAddEllipse (HPATH path, float cx, float cy,
        float rx, float ry, BOOL clockwise)
{
    MPPath* m_path = (MPPath*) path;

    if (!path || rx < 1 || ry < 1)
        return MP_GENERIC_ERROR;
    if ((MGPlusPathCloseFigure (path)) != MP_OK)
        return MP_GENERIC_ERROR;

    agg::bezier_arc ell;
    double sweet;

    if (clockwise)
        sweet = agg::deg2rad (360);
    else
        sweet = agg::deg2rad (-360);

    ell.init((double)cx, (double)cy, (double)rx, (double)ry, 
            agg::deg2rad (-180), sweet);
    agg::conv_curve <agg::bezier_arc> arc(ell);
    arc.approximation_scale(10.0);

    agg::path_storage path_arc;
    path_arc.concat_path (arc, 0);
    path_arc.transform (m_path->matrix);

    if (is_path_closed (m_path))
        m_path->m_agg_ps.concat_path (path_arc);
    else
        m_path->m_agg_ps.join_path (path_arc);
    //m_path->m_agg_ps.concat_path (path_arc, 0);

    return MP_OK;
}

MPStatus MGPlusPathAddEllipseI (HPATH path, int cx, int cy,
        int rx, int ry, BOOL clockwise)
{
    return MGPlusPathAddEllipse (path, (float)cx, (float)cy,
            (float)rx, (float)ry, clockwise);
}

MPStatus MGPlusPathAddPath (HPATH path, HPATH add_path)
{
    MPPath* m_path = (MPPath*) path;
    MPPath* m_add_path = (MPPath*) add_path;

    if (!path || !add_path)
        return MP_GENERIC_ERROR;

    if (is_path_closed (m_path))
        m_path->m_agg_ps.concat_path (m_add_path->m_agg_ps);
    else
        m_path->m_agg_ps.join_path (m_add_path->m_agg_ps);

    return MP_OK;
}

MPStatus MGPlusPathAddCurve (HPATH path, const MPPOINT* points, int count)
{
    agg::path_storage poly;
    MPPath* m_path = (MPPath*) path;
    int i;

    if ( !path || !points || count < 1 )
        return MP_GENERIC_ERROR;

    double d_x, d_y;
    d_x = (double) points [0].x;
    d_y = (double) points [0].y;

    m_path->matrix.transform (&d_x, &d_y);
    //poly.move_to (points [0].x, points [0].y);
    poly.move_to (d_x, d_y);

    for (i = 1; i < count; i++) {
        d_x = (double) points [i].x;
        d_y = (double) points [i].y;

        m_path->matrix.transform (&d_x, &d_y);
        //poly.line_to (points [i].x, points [i].y);
        poly.line_to (d_x, d_y);
    }

    typedef agg::conv_bspline<agg::path_storage> conv_bspline_type;
    conv_bspline_type bspline (poly);
    bspline.interpolation_step (1.0 / (double)(20.0));

    if (is_path_closed (m_path))
        m_path->m_agg_ps.concat_path (bspline);
    else
        m_path->m_agg_ps.join_path (bspline);
    return MP_OK;
}

MPStatus MGPlusPathSetAllOrientation (HPATH path, MPOrientation orientation)
{
    if (!path)
        return MP_GENERIC_ERROR;

    MPPath* m_path = (MPPath*) path;

    switch (orientation)
    {
        case MP_ORIENTATION_CW:
            m_path->m_agg_ps.arrange_orientations_all_paths (agg::path_flags_cw);
            break;
        case MP_ORIENTATION_CCW:
            m_path->m_agg_ps.arrange_orientations_all_paths (agg::path_flags_ccw);
            break;
        default:
            return MP_GENERIC_ERROR; 
    }
    return MP_OK;
}

MPStatus MGPlusPathAddRoundRectEx (HPATH path, int x, int y, 
        int width, int height, int rx, int ry)
{
    MPPath* m_path = (MPPath*)path;
    //RECT rc = {x, y, x+width, y+height};

    if (!m_path || rx < 1 || rx < 1)
        return MP_GENERIC_ERROR;

    if (rx >= width/2) rx = width/2;
    if (ry >= height/2) ry = height/2;

    agg::rounded_rect round_rect;
    round_rect.rect (x, y, x + width, y + height);
    round_rect.radius ((double)rx, (double)ry);

    agg::path_storage path_roundrect;
    path_roundrect.concat_path (round_rect, 0);
    path_roundrect.transform (m_path->matrix);


    if (is_path_closed (m_path))
        m_path->m_agg_ps.concat_path (path_roundrect);
    else
        m_path->m_agg_ps.join_path (path_roundrect);

    return MP_OK;
}

#ifdef _MGPLUS_FONT_FT2

HFONT MGPlusCreateFont (const char* font_name, unsigned face_index, 
        MPGlyphRendering ren_type, unsigned int width, unsigned int height, 
        BOOL flip_y) 
{
    MPFont* m_font = NULL;
    m_font = new MPFont; 

    if (!m_font)
        return MP_GENERIC_ERROR;

    m_font->m_feng = new font_engine_type();
    m_font->m_fman = new font_manager_type(*m_font->m_feng);

    /* check font width/height.*/
    if (width > FONT_MAX_SIZE)
        width  = FONT_MAX_SIZE;
    if (height > FONT_MAX_SIZE)
        height = FONT_MAX_SIZE;

    if (width < FONT_MIN_SIZE)
        width  = FONT_MIN_SIZE;
    if (height < FONT_MIN_SIZE)
        height = FONT_MIN_SIZE;

    if(!(m_font->m_feng->load_font(font_name, 0, (agg::glyph_rendering)ren_type))) {
        fprintf(stderr, "load_font %s failded!\n", font_name);
        delete (m_font->m_feng);
        delete (m_font->m_fman);
        delete m_font;
        return MP_INV_HANDLE;
    }
    m_font->m_feng->width (width);
    m_font->m_feng->height (height);
    m_font->m_feng->width (width);
    m_font->m_feng->flip_y (flip_y);

    m_font->fontname = new char[strlen(font_name) + 1];
    if (m_font->fontname)
        strcpy(m_font->fontname, (char*)font_name);

    return (HFONT)m_font;
}

MPStatus MGPlusDeleteFont (HFONT hfont)
{
    MPFont* m_font = (MPFont*) hfont;
    if (!m_font)
        return MP_GENERIC_ERROR;

    if (m_font->fontname)
        delete [] (m_font->fontname);

    delete (m_font->m_feng);
    delete (m_font->m_fman);
    delete (m_font);

    return MP_OK;
}

MPStatus MGPlusGetGlyphOutline (HFONT hfont, unsigned uchar, 
        LPGLYPHMETRICS lpgm, LPGLYPHDATA lpdata)
{
    MPFont* m_font = (MPFont*) hfont;
    if (!hfont) 
        return MP_GENERIC_ERROR;

    const agg::glyph_cache* glyph = m_font->m_fman->glyph(uchar);

    if (!glyph || !lpdata) 
        return MP_GENERIC_ERROR;

    /* retrieves the curve data points in the rasterizer's native
     * format and uses the font's design units. 
     */
    if (lpgm) {
        lpgm->bbox_x = glyph->bounds.x1; 
        lpgm->bbox_x = glyph->bounds.y1; 
        lpgm->bbox_w = glyph->bounds.x2 - glyph->bounds.x1; 
        lpgm->bbox_h = glyph->bounds.y2 - glyph->bounds.y1; 
        lpgm->adv_x  = (short int) glyph->advance_x; 
        lpgm->adv_y  = (short int) glyph->advance_y; 
    }

    m_font->m_fman->init_embedded_adaptors(glyph, 0, 0);
    memcpy (lpdata, glyph, sizeof(GLYPHDATA));

    return MP_OK;
}

#ifdef _DEBUG
template<class VS> void MGPlusDumpPath(VS& path)
{
    unsigned cmd;
    double x, y;
    FILE* fd = fopen("dump_path.dat", "a");

    fprintf(fd, "-------\n");
    path.rewind(0);
    while(!agg::is_stop(cmd = path.vertex(&x, &y))) {
        fprintf(fd, "%02X %8.2f %8.2f\n", cmd, x, y);
    }
    fclose(fd);
}
#endif

HPATH 
MGPlusGetGlyphPath (int x, int y, LPGLYPHDATA lpdata)
{

    typedef agg::serialized_integer_path_adaptor <agg::int32, 6> path_adaptor_type;
    path_adaptor_type path_adaptor;

    HPATH path =  MGPlusPathCreate (MP_PATH_FILL_MODE_WINDING);
    MPPath* m_path = (MPPath*) path;

    /* data_type must be outline.*/
    if (!lpdata || !lpdata->data || !lpdata->data_size || 
            lpdata->data_type != GLYPH_DATA_OUTLINE) {
        MGPlusPathDelete (path);
        return MP_INV_HANDLE;
    }

    path_adaptor.init((agg::int8u*)lpdata->data, lpdata->data_size, x, y, 1.0);

    m_path->m_agg_ps.remove_all();
    m_path->m_agg_ps.concat_path(path_adaptor);

    return (HPATH)path;
}
#endif

MPStatus 
MGPlusPathRotateAroundPoint(HPATH path, const MPPOINT* pt, float angle)
{
    if (!pt || !path)
        return MP_GENERIC_ERROR;
    
    MGPlusPathTranslate(path, -pt->x, -pt->y);
    MGPlusPathRotate(path, angle);
    MGPlusPathTranslate(path, pt->x, pt->y);

    return MP_OK;
}
