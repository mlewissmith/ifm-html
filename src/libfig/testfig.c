/*
 * This file is part of IFM (Interactive Fiction Mapper), copyright (C)
 * Glenn Hutchings 1997-2008.
 *
 * IFM comes with ABSOLUTELY NO WARRANTY.  This is free software, and you
 * are welcome to redistribute it under certain conditions; see the file
 * COPYING for details.
 */

/* Figlib test */

#include "fig.h"

#define WIDTH 3.0
#define HEIGHT 2.0

#define TXT "Hello there!"
#define FNT 23.0

#define TEXT \
        "Hello there.  This is a very long line to test out the word " \
        "wrapping gimmick of the textbox function.  Neat huh?  Let's " \
        "try another sentence to make sure it works properly."

int
main(void)
{
    vhash *figure, *box, *group, *text, *line, *spline;
    float width, height;

    v_debug(V_DBG_PTRS);

    figure = fig_create(FIG_INCHES, 1.0);
    fig_set_orientation(figure, FIG_PORTRAIT);

    text = fig_create_textbox(figure, "Helvet", 20, FIG_JUSTIFY_CENTRE,
                              1.0, 1.0, WIDTH, HEIGHT, TEXT);
    fig_set_name(text, "text");
    fig_set_colour(text, 1.0, 0.0, 0.0);
    box = fig_create_box(figure, 1.0, 1.0, WIDTH, HEIGHT);
    fig_set_name(box, "box around text");
    fig_set_depth(box, 200);

#if 0
    group = fig_create_compound(figure);
    text = fig_create_text(group, 6.0, 6.0, "Hi there");
    box = fig_create_box(group, 3.0, 3.0, 2.0, 2.0);
    line = fig_create_line(figure, 1.0, 1.0, 7.0, 3.0);
    spline = fig_create_spline(figure, FIG_OPEN_INTERP);

    fig_create_point(spline, 1.0, 1.0);
    fig_create_point(spline, 1.0, 2.0);
    fig_create_point(spline, 2.0, 2.0);
    fig_create_point(spline, 2.0, 3.0);

    fig_set_colour(spline, 0.0, 0.0, 1.0);
    fig_set_font(text, "Helvet", 20.0);
    fig_set_fillcolour(box, 1.0, 1.0, 0.0);
    fig_set_colour(box, 0.0, 1.0, 0.0);
    fig_set_colour(text, 1.0, 0.0, 0.0);
    fig_set_name(group, "My group");
    fig_set_name(box, "My box");
    fig_set_arrow(line, 1, 0);
    fig_set_arrow(spline, 1, 0);
#endif

    fig_write_figure(figure, stdout);

    return 0;
}
