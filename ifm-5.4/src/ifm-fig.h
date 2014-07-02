/*
 * This file is part of IFM (Interactive Fiction Mapper), copyright (C)
 * Glenn Hutchings 1997-2008.
 *
 * IFM comes with ABSOLUTELY NO WARRANTY.  This is free software, and you
 * are welcome to redistribute it under certain conditions; see the file
 * COPYING for details.
 */

/* Fig output driver header */

#ifndef IFM_FIG_H
#define IFM_FIG_H

extern mapfuncs fig_mapfuncs;

extern void fig_map_start(void);
extern void fig_map_section(vhash *sect);
extern void fig_map_room(vhash *room);
extern void fig_map_link(vhash *link);
extern void fig_map_endsection(void);
extern void fig_map_finish(void);

#endif
