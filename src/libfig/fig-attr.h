/*
 * This file is part of IFM (Interactive Fiction Mapper), copyright (C)
 * Glenn Hutchings 1997-2008.
 *
 * IFM comes with ABSOLUTELY NO WARRANTY.  This is free software, and you
 * are welcome to redistribute it under certain conditions; see the file
 * COPYING for details.
 */

/* Attribute function header */

#ifndef FIG_ATTR_H
#define FIG_ATTR_H

#include <vars.h>

#define FIG_RESOLUTION 1200

#define FIG_TEXT_WSCALE 7
#define FIG_TEXT_HSCALE 11.25

#define FIG_UNDEF -1

/* Orientation */
enum {
    FIG_LANDSCAPE, FIG_PORTRAIT
};

/* Units */
enum {
    FIG_METRIC, FIG_INCHES
};

/* Indexed colours */
enum {
    FIG_BLACK, FIG_BLUE, FIG_GREEN, FIG_CYAN, FIG_RED, FIG_MAGENTA,
    FIG_YELLOW, FIG_WHITE
};

/* Line styles */
enum {
    FIG_SOLID, FIG_DASH, FIG_DOTTED, FIG_DASH_DOT, FIG_DASH_2_DOTS,
    FIG_DASH_3_DOTS
};

/* Join styles */
enum {
    FIG_MITRE, FIG_BEVEL, FIG_ROUND
};

/* Arrow types */
enum {
    FIG_STICK, FIG_CLOSED, FIG_INDENTED, FIG_POINTED
};

/* Arrow styles */
enum {
    FIG_HOLLOW, FIG_FILLED
};

/* Text justification types */
enum {
    FIG_JUSTIFY_LEFT, FIG_JUSTIFY_CENTRE, FIG_JUSTIFY_RIGHT
};

extern void fig_set_arrow(vhash *object, int forward, int backward);
extern void fig_set_arrowstyle(vhash *object, int type, int style,
                               float thick, float width, float height);
extern void fig_set_colour(vhash *object, float r, float g, float b);
extern void fig_set_depth(vhash *object, int depth);
extern void fig_set_fillcolour(vhash *object, float r, float g, float b);
extern void fig_set_fillstyle(vhash *object, int style);
extern void fig_set_font(vhash *object, char *font, float fontsize);
extern void fig_set_joinstyle(vhash *object, int style);
extern void fig_set_justify(vhash *object, int justify);
extern void fig_set_linestyle(vhash *object, int style);
extern void fig_set_linewidth(vhash *object, int width);
extern void fig_set_name(vhash *object, char *fmt, ...);
extern void fig_set_orientation(vhash *figure, int orient);
extern void fig_set_papersize(vhash *figure, char *size);
extern void fig_set_shape(vhash *object, int num, float shape);

#endif
