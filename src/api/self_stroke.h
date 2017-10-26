
#ifndef SELF_STROKE_H
#define SELF_STROKE_H

//#include "agg_conv_stroke.h"

//============================================================================
template<class Source> struct self_stroke_draft
{
    Source& s;
    self_stroke_draft(Source& src, double w) : 
        s(src)
    {
    }

    void rewind(unsigned path_id) { s.rewind(path_id); }
    unsigned vertex(double* x, double* y) { return s.vertex(x, y); }
};

#endif
