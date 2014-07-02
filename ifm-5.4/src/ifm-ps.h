/*
 * This file is part of IFM (Interactive Fiction Mapper), copyright (C)
 * Glenn Hutchings 1997-2008.
 *
 * IFM comes with ABSOLUTELY NO WARRANTY.  This is free software, and you
 * are welcome to redistribute it under certain conditions; see the file
 * COPYING for details.
 */

/* Postscript output driver header */

#ifndef IFM_PS_H
#define IFM_PS_H

extern mapfuncs ps_mapfuncs;

extern void ps_map_start(void);
extern void ps_map_section(vhash *sect);
extern void ps_map_room(vhash *room);
extern void ps_map_link(vhash *link);
extern void ps_map_endsection(void);
extern void ps_map_finish(void);

#endif
