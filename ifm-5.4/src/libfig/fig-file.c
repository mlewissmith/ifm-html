/*
 * This file is part of IFM (Interactive Fiction Mapper), copyright (C)
 * Glenn Hutchings 1997-2008.
 *
 * IFM comes with ABSOLUTELY NO WARRANTY.  This is free software, and you
 * are welcome to redistribute it under certain conditions; see the file
 * COPYING for details.
 */

/* Fig file functions */

#include "fig-attr.h"
#include "fig-file.h"
#include "fig-object.h"
#include "fig-util.h"

/* Internal functions */
static void fig_write_arrows(vhash *object, FILE *fp);
static void fig_write_object(vhash *object, FILE *fp);

/* Write object arrows to a stream */
static void
fig_write_arrows(vhash *object, FILE *fp)
{
    int arrow_forward = fig_get_ival(object, "ARROW_FORWARD");
    int arrow_backward = fig_get_ival(object, "ARROW_BACKWARD");
    int arrow_type = fig_get_ival(object, "ARROW_TYPE");
    int arrow_style = fig_get_ival(object, "ARROW_STYLE");
    float thickness = fig_get_fval(object, "ARROW_THICK");
    float width = fig_get_fval(object, "ARROW_WIDTH");
    float height = fig_get_fval(object, "ARROW_HEIGHT");

    if (arrow_forward)
        fprintf(fp, "\t%d %d %g %g %g\n",
                arrow_type, arrow_style, thickness, width, height);

    if (arrow_backward)
        fprintf(fp, "\t%d %d %g %g %g\n",
                arrow_type, arrow_style, thickness, width, height);
}

/* Write figure to a stream */
void
fig_write_figure(vhash *figure, FILE *fp)
{
    int orient = fig_get_ival(figure, "ORIENTATION");
    vlist *objects;
    vhash *colours;
    viter iter;

    /* Calculate bounding box */
    fig_calc_bbox(figure);

    /* Write header info */
    fprintf(fp, "#FIG 3.2\n");
    fprintf(fp, "%s\n", orient == FIG_LANDSCAPE ? "Landscape" : "Portrait");
    fprintf(fp, "Center\n");
    fprintf(fp, "%s\n", fig_get_sval(figure, "UNITS"));
    fprintf(fp, "%s\n", fig_get_sval(figure, "PAPERSIZE"));
    fprintf(fp, "100.00\n");
    fprintf(fp, "Single\n");
    fprintf(fp, "-2\n");
    fprintf(fp, "%d 2\n", FIG_RESOLUTION);

    /* Write colour table entries (if any) */
    if ((colours = vh_pget(figure, "COLOURS")) != NULL)
        v_iterate(colours, iter)
            fprintf(fp, "%d %d %s\n", FIG_COLOUR,
                    vh_iter_ival(iter),
                    vh_iter_key(iter));

    /* Write objects */
    if ((objects = vh_pget(figure, "OBJECTS")) != NULL)
        v_iterate(objects, iter)
            fig_write_object(vl_iter_pval(iter), fp);
}

/* Write an object to a stream */
static void
fig_write_object(vhash *object, FILE *fp)
{
    int radius = 1, npoints, type, subtype, pen_style = 0, cap_style = 0;
    float style_val = 1.0, width, height;
    int i, font_flags = 4, direction = 0;
    vlist *objects, *xp, *yp, *shape;
    viter iter;

    type = vh_iget(object, "TYPE");
    subtype = vh_iget(object, "SUBTYPE");

    xp = vh_pget(object, "XP");
    yp = vh_pget(object, "YP");
    shape = vh_pget(object, "SHAPE");
    npoints = (xp != NULL ? vl_length(xp) : 0);

    if (vh_exists(object, "NAME"))
        fprintf(fp, "# %s\n", vh_sgetref(object, "NAME"));

    switch (type) {

    case FIG_ELLIPSE:
        /* FINISH ME */
        break;

    case FIG_POLYLINE:
        fprintf(fp, "%d ", type);
        fprintf(fp, "%d ", subtype);
        fprintf(fp, "%d ", fig_get_ival(object, "LINESTYLE"));
        fprintf(fp, "%d ", fig_get_ival(object, "LINEWIDTH"));
        fprintf(fp, "%d ", fig_get_ival(object, "PENCOLOUR"));
        fprintf(fp, "%d ", fig_get_ival(object, "FILLCOLOUR"));
        fprintf(fp, "%d ", fig_get_ival(object, "DEPTH"));
        fprintf(fp, "%d ", pen_style);
        fprintf(fp, "%d ", fig_get_ival(object, "FILLSTYLE"));
        fprintf(fp, "%.3f ", style_val);
        fprintf(fp, "%d ", fig_get_ival(object, "JOINSTYLE"));
        fprintf(fp, "%d ", cap_style);
        fprintf(fp, "%d ", radius);
        fprintf(fp, "%d ", fig_get_ival(object, "ARROW_FORWARD"));
        fprintf(fp, "%d ", fig_get_ival(object, "ARROW_BACKWARD"));
        fprintf(fp, "%d\n", npoints);

        if (subtype == FIG_PICTURE)
            fprintf(fp, "\t0 %s\n", fig_get_sval(object, "PICFILE"));
        else
            fig_write_arrows(object, fp);

        fprintf(fp, "\t");
        for (i = 0; i < npoints; i++)
            fprintf(fp, " %d %d", vl_iget(xp, i), vl_iget(yp, i));
        fprintf(fp, "\n");

        break;

    case FIG_SPLINE:
        fprintf(fp, "%d ", type);
        fprintf(fp, "%d ", subtype);
        fprintf(fp, "%d ", fig_get_ival(object, "LINESTYLE"));
        fprintf(fp, "%d ", fig_get_ival(object, "LINEWIDTH"));
        fprintf(fp, "%d ", fig_get_ival(object, "PENCOLOUR"));
        fprintf(fp, "%d ", fig_get_ival(object, "FILLCOLOUR"));
        fprintf(fp, "%d ", fig_get_ival(object, "DEPTH"));
        fprintf(fp, "%d ", pen_style);
        fprintf(fp, "%d ", fig_get_ival(object, "FILLSTYLE"));
        fprintf(fp, "%.3f ", style_val);
        fprintf(fp, "%d ", cap_style);
        fprintf(fp, "%d ", fig_get_ival(object, "ARROW_FORWARD"));
        fprintf(fp, "%d ", fig_get_ival(object, "ARROW_BACKWARD"));
        fprintf(fp, "%d\n", npoints);

        fig_write_arrows(object, fp);

        fprintf(fp, "\t");
        for (i = 0; i < npoints; i++)
            fprintf(fp, " %d %d", vl_iget(xp, i), vl_iget(yp, i));
        fprintf(fp, "\n");

        fprintf(fp, "\t");
        for (i = 0; i < npoints; i++)
            fprintf(fp, " %g", vl_dget(shape, i));
        fprintf(fp, "\n");

        break;

    case FIG_TEXT:
        fprintf(fp, "%d ", type);
        fprintf(fp, "%d ", fig_get_ival(object, "JUSTIFY"));
        fprintf(fp, "%d ", fig_get_ival(object, "PENCOLOUR"));
        fprintf(fp, "%d ", fig_get_ival(object, "DEPTH"));
        fprintf(fp, "%d ", pen_style);
        fprintf(fp, "%d ", fig_get_ival(object, "FONT"));
        fprintf(fp, "%g ", fig_get_fval(object, "FONTSIZE"));
        fprintf(fp, "%g ", fig_get_fval(object, "ANGLE"));
        fprintf(fp, "%d ", font_flags);
        fprintf(fp, "%g ", fig_get_fval(object, "HEIGHT"));
        fprintf(fp, "%g ", fig_get_fval(object, "WIDTH"));
        fprintf(fp, "%d ", fig_get_ival(object, "X"));
        fprintf(fp, "%d ", fig_get_ival(object, "Y"));
        fprintf(fp, "%s\\001\n", fig_get_sval(object, "TEXT"));
        break;

    case FIG_ARC:
        fprintf(fp, "%d ", type);
        fprintf(fp, "%d ", subtype);
        fprintf(fp, "%d ", fig_get_ival(object, "LINESTYLE"));
        fprintf(fp, "%d ", fig_get_ival(object, "LINEWIDTH"));
        fprintf(fp, "%d ", fig_get_ival(object, "PENCOLOUR"));
        fprintf(fp, "%d ", fig_get_ival(object, "FILLCOLOUR"));
        fprintf(fp, "%d ", fig_get_ival(object, "DEPTH"));
        fprintf(fp, "%d ", pen_style);
        fprintf(fp, "%d ", fig_get_ival(object, "FILLSTYLE"));
        fprintf(fp, "%.3f ", style_val);
        fprintf(fp, "%d ", cap_style);
        fprintf(fp, "%d ", direction);
        fprintf(fp, "%d ", fig_get_ival(object, "ARROW_FORWARD"));
        fprintf(fp, "%d ", fig_get_ival(object, "ARROW_BACKWARD"));

        for (i = 0; i < npoints; i++)
            fprintf(fp, " %g %g", vl_fget(xp, i), vl_fget(yp, i));

        fprintf(fp, "\n");
        fig_write_arrows(object, fp);
        break;

    case FIG_COMPOUND:
        fprintf(fp, "%d ", type);
        fprintf(fp, "%d ", fig_get_ival(object, "XMAX"));
        fprintf(fp, "%d ", fig_get_ival(object, "YMAX"));
        fprintf(fp, "%d ", fig_get_ival(object, "XMIN"));
        fprintf(fp, "%d\n", fig_get_ival(object, "YMIN"));

        if ((objects = vh_pget(object, "OBJECTS")) != NULL)
            v_iterate(objects, iter)
                fig_write_object(vl_iter_pval(iter), fp);

        fprintf(fp, "-6\n");
        break;
    }
}
