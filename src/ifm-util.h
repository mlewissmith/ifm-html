/*
 * This file is part of IFM (Interactive Fiction Mapper), copyright (C)
 * Glenn Hutchings 1997-2008.
 *
 * IFM comes with ABSOLUTELY NO WARRANTY.  This is free software, and you
 * are welcome to redistribute it under certain conditions; see the file
 * COPYING for details.
 */

/* Utility function header */

#ifndef IFM_UTIL_H
#define IFM_UTIL_H

/* Direction stuff */
struct d_info {
    char *sname, *lname;
    int dir, odir;
    int xoff, yoff;
};

enum {
    D_NONE, D_NORTH, D_SOUTH, D_EAST, D_WEST, D_NORTHEAST, D_SOUTHWEST,
    D_NORTHWEST, D_SOUTHEAST, D_UP, D_DOWN, D_IN, D_OUT
};

extern struct d_info dirinfo[];

/* Advertised functions */
extern void add_attr(vhash *obj, char *attr, char *fmt, ...);
extern void add_list(vhash *obj, char *attr, vhash *thing);
extern char *find_file(char *name);
extern int get_direction(int xoff, int yoff);
extern int get_papersize(char *pagesize, float *width, float *height);
extern void indent(int num);
extern void obsolete(char *old, char *new);
extern int pack_sections(int xmax, int ymax);
extern void put_string(char *fmt, ...);
extern void setup_room_names(void);
extern vlist *split_line(char *string, double ratio);
extern void truncate_points(vlist *x, vlist *y, double wid, double ht);

#endif
