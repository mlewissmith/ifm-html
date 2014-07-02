/*
 * This file is part of IFM (Interactive Fiction Mapper), copyright (C)
 * Glenn Hutchings 1997-2008.
 *
 * IFM comes with ABSOLUTELY NO WARRANTY.  This is free software, and you
 * are welcome to redistribute it under certain conditions; see the file
 * COPYING for details.
 */

/* Map function header */

#ifndef IFM_MAP_H
#define IFM_MAP_H

/* External vars */
extern vhash *map, *startroom;
extern vlist *rooms, *links, *joins, *items, *tasks, *sects;
extern vhash *roomtags, *itemtags, *tasktags, *linktags, *jointags;
extern vlist *taskorder, *sectnames;

/* Advertised functions */
extern void init_map(void);
extern void resolve_tags(void);
extern void room_exit(vhash *room, int xoff, int yoff, int flag);
extern void set_tag(char *type, char *tag, vhash *val, vhash *table);
extern void setup_exits(void);
extern void setup_links(void);
extern void setup_rooms(void);
extern void setup_sections(void);

#endif
