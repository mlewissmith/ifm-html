/*
 * This file is part of IFM (Interactive Fiction Mapper), copyright (C)
 * Glenn Hutchings 1997-2008.
 *
 * IFM comes with ABSOLUTELY NO WARRANTY.  This is free software, and you
 * are welcome to redistribute it under certain conditions; see the file
 * COPYING for details.
 */

/* Object function header */

#ifndef FIG_OBJECT_H
#define FIG_OBJECT_H

#include <vars.h>

/* Object types */
enum {
    FIG_COLOUR, FIG_ELLIPSE, FIG_POLYLINE, FIG_SPLINE, FIG_TEXT, FIG_ARC,
    FIG_COMPOUND, FIG_ROOT
};

/* Polyline types */
enum {
    FIG_LINE = 1, FIG_BOX, FIG_POLYGON, FIG_ARCBOX, FIG_PICTURE
};

/* Ellipse types */
enum {
    FIG_ELLIPSE_RADII = 1, FIG_ELLIPSE_DIAM, FIG_CIRCLE_RADIUS,
    FIG_CIRCLE_DIAM
};

/* Arc types */
enum {
    FIG_WEDGE, FIG_OPEN
};

/* Spline types */
enum {
    FIG_OPEN_APPROX, FIG_CLOSED_APPROX, FIG_OPEN_INTERP, FIG_CLOSED_INTERP,
    FIG_OPEN_XSPLINE, FIG_CLOSED_XSPLINE
};

extern vhash *fig_create(int orient, float scale);
extern vhash *fig_create_arc(vhash *parent, int subtype, float cx, float cy,
                             float x1, float y1, float x2, float y2, float x3,
                             float y3);
extern vhash *fig_create_box(vhash *parent, float x, float y,
                             float width, float height);
extern vhash *fig_create_compound(vhash *parent);
extern vhash *fig_create_ellipse(vhash *parent, int subtype);
extern vhash *fig_create_line(vhash *parent, float x1, float y1, float x2,
                              float y2);
extern vhash *fig_create_picture(vhash *parent, float x, float y,
                                 float width, float height, char *file);
extern int fig_create_point(vhash *parent, float x, float y);
extern vhash *fig_create_polygon(vhash *parent, float x1, float y1, float x2,
                                 float y2, float x3, float y3);
extern vhash *fig_create_polyline(vhash *parent, int subtype);
extern vhash *fig_create_spline(vhash *parent, int subtype);
extern vhash *fig_create_text(vhash *parent, float x, float y, char *fmt,
                              ...);
extern vhash *fig_create_textbox(vhash *parent, char *font, float fontsize,
                                 int justify, float x, float y, float width,
                                 float height, char *fmt, ...);
extern void fig_destroy(vhash *figure);

#endif
