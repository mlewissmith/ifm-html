/*
 * This file is part of IFM (Interactive Fiction Mapper), copyright (C)
 * Glenn Hutchings 1997-2008.
 *
 * IFM comes with ABSOLUTELY NO WARRANTY.  This is free software, and you
 * are welcome to redistribute it under certain conditions; see the file
 * COPYING for details.
 */

/* Raw output driver header */

#ifndef IFM_RAW_H
#define IFM_RAW_H

extern mapfuncs raw_mapfuncs;
extern itemfuncs raw_itemfuncs;
extern taskfuncs raw_taskfuncs;

extern void raw_map_start(void);
extern void raw_map_section(vhash *sect);
extern void raw_map_room(vhash *room);
extern void raw_map_link(vhash *link);
extern void raw_map_join(vhash *join);

extern void raw_item_entry(vhash *item);

extern void raw_task_entry(vhash *task);

#endif
