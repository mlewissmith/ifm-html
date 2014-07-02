%{
/*
 * This file is part of IFM (Interactive Fiction Mapper), copyright (C)
 * Glenn Hutchings 1997-2008.
 *
 * IFM comes with ABSOLUTELY NO WARRANTY.  This is free software, and you
 * are welcome to redistribute it under certain conditions; see the file
 * COPYING for details.
 */

/* Input parser */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vars.h>

#include "ifm-map.h"
#include "ifm-util.h"
#include "ifm-vars.h"

#define SET_LIST(object, attr, list) {                                  \
        vlist *l = vh_pget(object, attr);                               \
        if (l == NULL) {                                                \
            vh_pstore(object, attr, list);                              \
            list = NULL;                                                \
        } else {                                                        \
            vl_append(l, list);                                         \
            vl_destroy(list);                                           \
            list = NULL;                                                \
        }                                                               \
}

#define ATTR(name) \
        (implicit ? "LINK_" #name : #name)

#define RESET RESET_IT; RESET_THEM

#define RESET_IT                                                        \
        RESET_VAR(itroom);                                              \
        RESET_VAR(ititem);                                              \
        RESET_VAR(ittask)

#define RESET_THEM                                                      \
        { if (themitems != NULL) vl_destroy(themitems); themitems = NULL; }

#define RESET_VAR(var) if (var != NULL) { vs_destroy(var); var = NULL; }

#define WARN_IGNORED(attr)                                              \
        warn("attribute '%s' ignored -- no implicit link", #attr)

#define CHANGE_ERROR(attr)                                              \
        err("can't modify '%s' attribute", #attr)

static vhash *curobj = NULL;    /* Current object */

static vlist *currooms = NULL;  /* Current room list */
static vlist *curitems = NULL;  /* Current item list */
static vlist *curtasks = NULL;  /* Current task list */

static vhash *lastroom = NULL;  /* Last room mentioned */
static vhash *lastitem = NULL;  /* Last item mentioned */
static vhash *lasttask = NULL;  /* Last task mentioned */

static vscalar *itroom = NULL;  /* Room referred to by 'it' */
static vscalar *ititem = NULL;  /* Item referred to by 'it' */
static vscalar *ittask = NULL;  /* Task referred to by 'it' */

static vlist *themitems = NULL; /* Items referred to by 'them' */

static int roomid = 0;          /* Current room ID */
static int itemid = 0;          /* Current item ID */

static vlist *curdirs = NULL;   /* Current direction list */

static int modify = 0;          /* Modification flag */
static int implicit = 0;        /* Implicit-link flag */
static int allflag = 0;         /* All-items flag */
static int repeat = 0;          /* String repeat count */
static int instyle = 0;         /* Set variable in different style? */
%}

%union {
    int ival;
    double dval;
    char *sval;
    vscalar *vval;
}

%token	      ROOM ITEM LINK FROM TAG TO DIR ONEWAY HIDDEN NODROP NOTE TASK
%token	      AFTER NEED GET SCORE JOIN GO REQUIRE ANY LAST START GOTO MAP
%token        EXIT GIVEN LOST KEEP LENGTH TITLE LOSE SAFE BEFORE FOLLOW CMD
%token        LEAVE UNDEF FINISH GIVE DROP ALL EXCEPT IT UNTIL TIMES NOLINK
%token        NOPATH NONE STYLE ENDSTYLE WITH IGNORE DO THEM

%token <ival> NORTH EAST SOUTH WEST NORTHEAST NORTHWEST SOUTHEAST SOUTHWEST
%token <ival> UP DOWN IN OUT

%token <ival> INTEGER TRUE FALSE
%token <dval> REAL
%token <sval> STRING ID

%type  <ival> compass otherdir
%type  <sval> strings
%type  <vval> room item task
%type  <dval> number

%expect 1

%%

/************************************************************************/
/* Top-level
/************************************************************************/

stmt_list	: /* empty */
		| stmt_list stmt
		| stmt_list error
		;

stmt		: ctrl_stmt
                | room_stmt
		| item_stmt
		| link_stmt
		| join_stmt
		| task_stmt
		| vars_stmt
                | style_stmt
		;

/************************************************************************/
/* Control commands
/************************************************************************/

ctrl_stmt       : TITLE STRING ';'
                {
                    vh_sstore(map, "TITLE", $2);
                }
                | MAP STRING ';'
                {
                    if (sectnames == NULL)
                        sectnames = vl_create();
                    vl_spush(sectnames, $2);
                }
                | REQUIRE number ';'
                {
                    float version;
                    sscanf(VERSION, "%f", &version);
                    if ($2 - version > 0.001)
                        fatal("version %g of IFM is required (this is %s)",
                              $2, VERSION);
                }
                ;

/************************************************************************/
/* Rooms
/************************************************************************/

room_stmt	: ROOM STRING
		{
                    curobj = vh_create();
		    vh_sstore(curobj, "DESC", $2);
                    vh_istore(curobj, "ID", roomid++);
                    vh_pstore(curobj, "STYLE", current_styles());
                    vh_pstore(curobj, "LINK_STYLE", current_styles());
                    implicit = 0;
                    modify = 0;
		}
                room_attrs ';'
                {
                    vhash *near, *link, *sect = NULL;
                    vlist *list, *dirs;
                    char *str;

                    /* Build new room */
                    vl_ppush(rooms, curobj);

                    if (startroom == NULL)
                        startroom = curobj;

                    /* Put it on appropriate section */
                    if ((near = vh_pget(curobj, "NEAR")) != NULL)
                        sect = vh_pget(near, "SECT");

                    if (sect == NULL) {
                        sect = vh_create();
                        vl_ppush(sects, sect);
                        vh_istore(sect, "NUM", vl_length(sects));
                        vh_pstore(sect, "ROOMS", vl_create());
                        vh_pstore(sect, "LINKS", vl_create());
                    }

                    vh_pstore(curobj, "SECT", sect);
                    list = vh_pget(sect, "ROOMS");
                    vl_punshift(list, curobj);

                    /* Build implicit link (if any) */
                    if ((dirs = vh_pget(curobj, "DIR")) != NULL) {
                        link = vh_create();
                        vh_pstore(curobj, "LINK", link);

                        vh_pstore(link, "FROM", near);
                        vh_pstore(link, "TO", curobj);

                        vh_istore(link, "GO",
                                  vh_iget(curobj, "GO"));
                        vh_istore(link, "ONEWAY",
                                  vh_iget(curobj, "ONEWAY"));
                        vh_istore(link, "NODROP",
                                  vh_iget(curobj, "NODROP"));
                        vh_istore(link, "NOLINK",
                                  vh_iget(curobj, "NOLINK"));
                        vh_istore(link, "NOPATH",
                                  vh_iget(curobj, "NOPATH"));
                        vh_istore(link, "LEN",
                                  vh_iget(curobj, "LEN"));
                        vh_pstore(link, "BEFORE",
                                  vh_pget(curobj, "LINK_BEFORE"));
                        vh_pstore(link, "AFTER",
                                  vh_pget(curobj, "LINK_AFTER"));
                        vh_pstore(link, "NEED",
                                  vh_pget(curobj, "LINK_NEED"));
                        vh_pstore(link, "LEAVE",
                                  vh_pget(curobj, "LINK_LEAVE"));
                        vh_istore(link, "LEAVEALL",
                                  vh_iget(curobj, "LINK_LEAVEALL"));
                        vh_pstore(link, "STYLE",
                                  vh_pget(curobj, "LINK_STYLE"));
                        vh_pstore(link, "FROM_CMD",
                                  vh_pget(curobj, "FROM_CMD"));
                        vh_pstore(link, "TO_CMD",
                                  vh_pget(curobj, "TO_CMD"));

                        if (vh_exists(curobj, "TAG"))
                            set_tag("link", vh_sgetref(curobj, "TAG"),
                                    link, linktags);

                        vh_pstore(link, "DIR", dirs);
                        vl_ppush(links, link);
                    }

                    /* Warn about ignored attributes */
                    if (dirs == NULL || vh_iget(curobj, "NOLINK")) {
                        if (vh_exists(curobj, "GO"))
                            WARN_IGNORED(go);
                        if (vh_exists(curobj, "ONEWAY"))
                            WARN_IGNORED(oneway);
                        if (vh_exists(curobj, "LEN"))
                            WARN_IGNORED(length);
                        if (vh_exists(curobj, "NOPATH"))
                            WARN_IGNORED(nopath);
                        if (vh_exists(curobj, "TO_CMD"))
                            WARN_IGNORED(cmd);
                    }

                    if (dirs == NULL && vh_iget(curobj, "NOLINK"))
                        WARN_IGNORED(nolink);

                    lastroom = curobj;
                    RESET;
                }
                | ROOM ID
                {
                    implicit = 0;
                    modify = 1;
                    if ((curobj = vh_pget(roomtags, $2)) == NULL) {
                        err("room tag '%s' not yet defined", $2);
                        curobj = vh_create();
                    }
                }
                room_attrs ';'
                {
                    RESET;
                }
		;

room_attrs	: /* empty */
		| room_attrs room_attr
		;

room_attr	: TAG ID
		{
                    if (!modify)
                        set_tag("room", $2, curobj, roomtags);
                    else
                        CHANGE_ERROR(tag);
		}
		| DIR dir_list FROM ID
		{
                    vhash *room;

                    if (!modify) {
                        implicit = 1;

                        vh_pstore(curobj, "DIR", curdirs);
                        curdirs = NULL;

                        if ((room = vh_pget(roomtags, $4)) != NULL)
                            vh_pstore(curobj, "NEAR", room);
                        else
                            err("room tag '%s' not yet defined", $4);
                    } else {
                        CHANGE_ERROR(from);
                    }
		}
		| DIR dir_list
		{
                    implicit = 1;

                    if (modify && !vh_exists(curobj, "DIR"))
                        CHANGE_ERROR(dir);

                    vh_pstore(curobj, "DIR", curdirs);
                    curdirs = NULL;

                    if (lastroom == NULL)
                        err("no last room");
                    else
                        vh_pstore(curobj, "NEAR", lastroom);
		}
		| EXIT dir_list
		{
                    viter iter;
                    int dir;

                    v_iterate(curdirs, iter) {
                        dir = vl_iter_ival(iter);
                        room_exit(curobj, dirinfo[dir].xoff,
                                  dirinfo[dir].yoff, 1);
                    }

                    curdirs = NULL;
		}
		| LINK room_list
		{
                    vscalar *elt;
                    vhash *link;
                    viter iter;

                    v_iterate(currooms, iter) {
                        elt = vl_iter_val(iter);
                        link = vh_create();
                        vh_pstore(link, "FROM", curobj);
                        vh_store(link, "TO", vs_copy(elt));
                        vl_ppush(links, link);
                    }

                    vl_destroy(currooms);
                    currooms = NULL;
		}
		| JOIN room_list
		{
                    vscalar *elt;
                    vhash *join;
                    viter iter;

                    v_iterate(currooms, iter) {
                        elt = vl_iter_val(iter);
                        join = vh_create();
                        vh_pstore(join, "FROM", curobj);
                        vh_store(join, "TO", vs_copy(elt));
                        vl_ppush(joins, join);
                    }

                    vl_destroy(currooms);
                    currooms = NULL;
		}
		| GO otherdir
		{
                    vh_istore(curobj, "GO", $2);
		}
		| ONEWAY
		{
                    vh_istore(curobj, "ONEWAY", 1);
		}
		| NODROP
		{
                    vh_istore(curobj, "NODROP", 1);
		}
		| NOLINK
		{
                    vh_istore(curobj, "NOLINK", 1);
		}
		| NOPATH
		{
                    vh_istore(curobj, "NOPATH", 1);
		}
		| START
		{
                    startroom = curobj;
		}
                | FINISH
                {
                    vh_istore(curobj, "FINISH", 1);
                }
                | NEED item_list
                {
                    SET_LIST(curobj, ATTR(NEED), curitems);
                }
		| BEFORE task_list
		{
                    SET_LIST(curobj, ATTR(BEFORE), curtasks);
		}
		| AFTER task_list
		{
                    SET_LIST(curobj, ATTR(AFTER), curtasks);
		}
                | LEAVE item_list_all
                {
                    if (curitems != NULL)
                        SET_LIST(curobj, ATTR(LEAVE), curitems);
                    vh_istore(curobj, ATTR(LEAVEALL), allflag);
                }
		| LENGTH INTEGER
		{
                    vh_istore(curobj, "LEN", $2);
		}
		| SCORE INTEGER
		{
                    vh_istore(curobj, "SCORE", $2);
		}
                | CMD strings
                {
                    while (repeat-- > 0)
                        add_attr(curobj, "TO_CMD", $2);
                }
                | CMD TO strings
                {
                    while (repeat-- > 0)
                        add_attr(curobj, "TO_CMD", $3);
                }
                | CMD FROM strings
                {
                    while (repeat-- > 0)
                        add_attr(curobj, "FROM_CMD", $3);
                }
		| NOTE STRING
		{
                    add_attr(curobj, "NOTE", $2);
		}
                | STYLE style_list
		;

room_list	: room_elt
		| room_list room_elt
		;

room_elt	: room
		{
                    if (currooms == NULL)
                        currooms = vl_create();
                    vl_push(currooms, $1);
		}
		;

room            : ID
                {
                    $$ = vs_screate($1);
                    itroom = vs_copy($$);
                }
                | IT
                {
                    if (itroom == NULL)
                        err("no room referred to by 'it'");
                    else
                        $$ = vs_copy(itroom);
                }
                | LAST
                {
                    if (lastroom == NULL) {
                        err("no room referred to by 'last'");
                    } else {
                        $$ = vs_pcreate(lastroom);
                        itroom = vs_copy($$);
                    }                        
                }
                ;

/************************************************************************/
/* Items
/************************************************************************/

item_stmt	: ITEM STRING
                {
                    curobj = vh_create();
                    vh_sstore(curobj, "DESC", $2);
                    vh_istore(curobj, "ID", itemid++);
                    vh_pstore(curobj, "STYLE", current_styles());
                    modify = 0;
                }
                item_attrs ';'
		{
                    if (!vh_exists(curobj, "IN"))
                        vh_pstore(curobj, "IN", lastroom);

                    lastitem = curobj;
                    vl_ppush(items, curobj);
                    RESET;
		}
                | ITEM ID
                {
                    modify = 1;
                    if ((curobj = vh_pget(itemtags, $2)) == NULL) {
                        err("item tag '%s' not yet defined", $2);
                        curobj = vh_create();
                    }
                }
                item_attrs ';'
                {
                    RESET;
                }
		;

item_attrs	: /* empty */
		| item_attrs item_attr
		;

item_attr	: TAG ID
		{
                    if (!modify)
                        set_tag("item", $2, curobj, itemtags);
                    else
                        CHANGE_ERROR(tag);
		}
		| IN room
		{
                    vh_store(curobj, "IN", $2);
		}
		| NOTE STRING
		{
                    add_attr(curobj, "NOTE", $2);
		}
		| HIDDEN
		{
                    vh_istore(curobj, "HIDDEN", 1);
		}
		| GIVEN         /* obsolete */
		{
                    obsolete("'given' attribute", "task 'give' attribute");
                    vh_istore(curobj, "GIVEN", 1);
		}
		| LOST
		{
                    vh_istore(curobj, "LOST", 1);
		}
		| IGNORE
		{
                    vh_istore(curobj, "IGNORE", 1);
		}
		| KEEP
		{
                    vh_istore(curobj, "KEEP", 1);
		}
		| KEEP WITH item_list
		{
                    SET_LIST(curobj, "KEEP_WITH", curitems);
		}
		| KEEP UNTIL task_list
		{
                    SET_LIST(curobj, "KEEP_UNTIL", curtasks);
		}
                | NEED item_list
                {
                    SET_LIST(curobj, "NEED", curitems);
                }
                | BEFORE task_list
                {
                    SET_LIST(curobj, "BEFORE", curtasks);
                }
                | AFTER task_list
                {
                    SET_LIST(curobj, "AFTER", curtasks);
                }
		| SCORE INTEGER
		{
                    vh_istore(curobj, "SCORE", $2);
		}
                | FINISH
                {
                    vh_istore(curobj, "FINISH", 1);
                }
                | STYLE style_list
		;

item_list	: item_elt
		| item_list item_elt
                | THEM
                {
                    if (themitems != NULL) {
                        if (curitems == NULL)
                            curitems = vl_copy(themitems);
                        else
                            vl_append(curitems, themitems);
                    } else {
                        err("no items referred to by 'them'");
                    }
                }
		;

item_list_all   : item_list                     { allflag = 0; }
                | ALL                           { allflag = 1; }
                | ALL EXCEPT item_list          { allflag = 1; }
                ;

item_elt	: item
		{
                    if (curitems == NULL)
                        curitems = vl_create();

                    vl_push(curitems, $1);

                    if (themitems == NULL)
                        themitems = vl_create();

                    vl_push(themitems, vs_copy($1));
		}
		;

item            : ID
                {
                    $$ = vs_screate($1);
                    ititem = vs_copy($$);
                }
                | IT
                {
                    if (ititem == NULL)
                        err("no item referred to by 'it'");
                    else
                        $$ = vs_copy(ititem);
                }
                | LAST
                {
                    if (lastitem == NULL) {
                        err("no item referred to by 'last'");
                    } else {
                        $$ = vs_pcreate(lastitem);
                        ititem = vs_copy($$);
                    }
                }
                ;

/************************************************************************/
/* Links
/************************************************************************/

link_stmt	: LINK room TO room
                {
                    curobj = vh_create();
                    vh_store(curobj, "FROM", $2);
                    vh_store(curobj, "TO", $4);
                    vh_pstore(curobj, "STYLE", current_styles());
                    modify = 0;
                }
                link_attrs ';'
		{
                    vl_ppush(links, curobj);
                    RESET;
		}
                | LINK ID
                {
                    modify = 1;
                    if ((curobj = vh_pget(linktags, $2)) == NULL) {
                        err("link tag '%s' not yet defined", $2);
                        curobj = vh_create();
                    }
                }
                link_attrs ';'
                {
                    RESET;
                }
		;

link_attrs	: /* empty */
		| link_attrs link_attr
		;

link_attr	: DIR dir_list
		{
                    vh_pstore(curobj, "DIR", curdirs);
                    curdirs = NULL;
		}
		| GO otherdir
		{
                    vh_istore(curobj, "GO", $2);
		}
		| ONEWAY
		{
                    vh_istore(curobj, "ONEWAY", 1);
		}
		| HIDDEN
		{
                    vh_istore(curobj, "HIDDEN", 1);
		}
		| NOPATH
		{
                    vh_istore(curobj, "NOPATH", 1);
		}
                | NEED item_list
                {
                    SET_LIST(curobj, "NEED", curitems);
                }
		| BEFORE task_list
		{
                    SET_LIST(curobj, "BEFORE", curtasks);
		}
		| AFTER task_list
		{
                    SET_LIST(curobj, "AFTER", curtasks);
		}
                | LEAVE item_list_all
                {
                    if (curitems != NULL)
                        SET_LIST(curobj, "LEAVE", curitems);
                    vh_istore(curobj, "LEAVEALL", allflag);
                }
		| LENGTH INTEGER
		{
                    vh_istore(curobj, "LEN", $2);
		}
                | CMD strings
                {
                    while (repeat-- > 0)
                        add_attr(curobj, "TO_CMD", $2);
                }
                | CMD TO strings
                {
                    while (repeat-- > 0)
                        add_attr(curobj, "TO_CMD", $3);
                }
                | CMD FROM strings
                {
                    while (repeat-- > 0)
                        add_attr(curobj, "FROM_CMD", $3);
                }
                | TAG ID
		{
                    if (!modify)
                        set_tag("link", $2, curobj, linktags);
                    else
                        CHANGE_ERROR(tag);
		}
                | STYLE style_list
                ; 

/************************************************************************/
/* Joins
/************************************************************************/

join_stmt	: JOIN room TO room
                {
                    curobj = vh_create();
                    vh_store(curobj, "FROM", $2);
                    vh_store(curobj, "TO", $4);
                    vh_pstore(curobj, "STYLE", current_styles());
                    modify = 0;
                }
                join_attrs ';'
		{
                    vl_ppush(joins, curobj);
                    RESET;
		}
                | JOIN ID
                {
                    modify = 1;
                    if ((curobj = vh_pget(jointags, $2)) == NULL) {
                        err("join tag '%s' not yet defined", $2);
                        curobj = vh_create();
                    }
                }
                join_attrs ';'
                {
                    RESET;
                }
		;

join_attrs	: /* empty */
		| join_attrs join_attr
		;

join_attr	: GO compass
		{
                    vh_istore(curobj, "GO", $2);
		}
                | GO otherdir
		{
                    vh_istore(curobj, "GO", $2);
		}
		| ONEWAY
		{
                    vh_istore(curobj, "ONEWAY", 1);
		}
		| HIDDEN
		{
                    vh_istore(curobj, "HIDDEN", 1);
		}
		| NOPATH
		{
                    vh_istore(curobj, "NOPATH", 1);
		}
                | NEED item_list
                {
                    SET_LIST(curobj, "NEED", curitems);
                }
		| BEFORE task_list
		{
                    SET_LIST(curobj, "BEFORE", curtasks);
		}
		| AFTER task_list
		{
                    SET_LIST(curobj, "AFTER", curtasks);
		}
                | LEAVE item_list_all
                {
                    if (curitems != NULL)
                        SET_LIST(curobj, "LEAVE", curitems);
                    vh_istore(curobj, "LEAVEALL", allflag);
                }
		| LENGTH INTEGER
		{
                    vh_istore(curobj, "LEN", $2);
		}
                | CMD strings
                {
                    while (repeat-- > 0)
                        add_attr(curobj, "TO_CMD", $2);
                }
                | CMD TO strings
                {
                    while (repeat-- > 0)
                        add_attr(curobj, "TO_CMD", $3);
                }
                | CMD FROM strings
                {
                    while (repeat-- > 0)
                        add_attr(curobj, "FROM_CMD", $3);
                }
                | TAG ID
		{
                    if (!modify)
                        set_tag("join", $2, curobj, jointags);
                    else
                        CHANGE_ERROR(tag);
		}
                | STYLE style_list
		;

/************************************************************************/
/* Tasks
/************************************************************************/

task_stmt	: TASK STRING
                {
                    curobj = vh_create();
                    vh_sstore(curobj, "DESC", $2);
                    vh_pstore(curobj, "STYLE", current_styles());
                    modify = 0;
                }
                task_attrs ';'
		{
                    if (vh_iget(curobj, "NOROOM"))
                        vh_pstore(curobj, "IN", NULL);
                    else if (!vh_exists(curobj, "IN"))
                        vh_pstore(curobj, "IN", lastroom);

                    lasttask = curobj;
                    vl_ppush(tasks, curobj);
                    RESET;
		}
                | TASK ID
                {
                    modify = 1;
                    if ((curobj = vh_pget(tasktags, $2)) == NULL) {
                        err("task tag '%s' not yet defined", $2);
                        curobj = vh_create();
                    }
                }
                task_attrs ';'
                {
                    RESET;
                }
		;

task_attrs	: /* empty */
		| task_attrs task_attr
		;

task_attr	: TAG ID
		{
                    if (!modify)
                        set_tag("task", $2, curobj, tasktags);
                    else
                        CHANGE_ERROR(tag);
		}
		| AFTER task_list
		{
                    SET_LIST(curobj, "AFTER", curtasks);
		}
		| NEED item_list
		{
                    SET_LIST(curobj, "NEED", curitems);
		}
		| GIVE item_list
		{
                    SET_LIST(curobj, "GIVE", curitems);
		}
		| GET item_list
		{
                    SET_LIST(curobj, "GET", curitems);
		}
		| DO task_list
		{
                    SET_LIST(curobj, "DO", curtasks);
		}
		| DROP item_list_all
		{
                    if (curitems != NULL)
                        SET_LIST(curobj, "DROP", curitems);
                    vh_istore(curobj, "DROPALL", allflag);
		}
		| DROP item_list_all UNTIL task_list
		{
                    if (curitems != NULL)
                        SET_LIST(curobj, "DROP", curitems);
                    vh_istore(curobj, "DROPALL", allflag);
                    SET_LIST(curobj, "DROPUNTIL", curtasks);
		}
		| DROP item_list_all IN room
		{
                    if (curitems != NULL)
                        SET_LIST(curobj, "DROP", curitems);
                    vh_istore(curobj, "DROPALL", allflag);
                    vh_store(curobj, "DROPROOM", $4);
		}
		| DROP item_list_all IN room UNTIL task_list
		{
                    if (curitems != NULL)
                        SET_LIST(curobj, "DROP", curitems);
                    vh_istore(curobj, "DROPALL", allflag);
                    vh_store(curobj, "DROPROOM", $4);
                    SET_LIST(curobj, "DROPUNTIL", curtasks);
		}
		| DROP IN room
		{
                    vh_store(curobj, "DROPROOM", $3);
		}
		| DROP IN room UNTIL task_list
		{
                    vh_store(curobj, "DROPROOM", $3);
                    SET_LIST(curobj, "DROPUNTIL", curtasks);
		}
                | DROP UNTIL task_list
                {
                    SET_LIST(curobj, "DROPUNTIL", curtasks);
                }
		| LOSE item_list
		{
                    SET_LIST(curobj, "LOSE", curitems);
		}
                | GOTO room
                {
                    vh_store(curobj, "GOTO", $2);
                }
                | FOLLOW task
                {
                    vh_store(curobj, "FOLLOW", $2);
                }
		| IN room
		{
                    vh_store(curobj, "IN", $2);
		}
		| IN ANY
		{
                    vh_istore(curobj, "NOROOM", 1);
		}
		| IGNORE
		{
                    vh_istore(curobj, "IGNORE", 1);
		}
                | SAFE
                {
                    vh_istore(curobj, "SAFE", 1);
                }
		| SCORE INTEGER
		{
                    vh_istore(curobj, "SCORE", $2);
		}
                | FINISH
                {
                    vh_istore(curobj, "FINISH", 1);
                }
                | CMD strings
                {
                    while (repeat-- > 0)
                        add_attr(curobj, "CMD", $2);
                }
                | CMD NONE
                {
                    add_attr(curobj, "CMD", NULL);
                }
		| NOTE STRING
		{
                    add_attr(curobj, "NOTE", $2);
		}
                | STYLE style_list
		;

task_list	: task_elt
		| task_list task_elt
		;

task_elt	: task
		{
                    if (curtasks == NULL)
                        curtasks = vl_create();
                    vl_push(curtasks, $1);
		}
		;

task            : ID
                {
                    $$ = vs_screate($1);
                    ittask = vs_copy($$);
                }
                | IT
                {
                    if (ittask == NULL)
                        err("no task referred to by 'it'");
                    else
                        $$ = vs_copy(ittask);
                }
                | LAST
                {
                    if (lasttask == NULL) {
                        err("no task referred to by 'last'");
                    } else {
                        $$ = vs_pcreate(lasttask);
                        ittask = vs_copy($$);
                    }
                }
                ;

/************************************************************************/
/* Variables
/************************************************************************/

vars_stmt       : set_var
                {
                    if (instyle)
                        pop_style(NULL);
                    instyle = 0;
                }
                ;

set_var         : ID '=' number in_style ';'
                {
                    var_set(NULL, $1, vs_dcreate($3));
                }
                | ID ID '=' number in_style ';'
                {
                    var_set($1, $2, vs_dcreate($4));
                    obsolete("variable assignment", "dotted notation");
                }
                | ID '=' STRING in_style ';'
                {
                    var_set(NULL, $1, vs_screate($3));
                }
                | ID ID '=' STRING in_style ';'
                {
                    var_set($1, $2, vs_screate($4));
                    obsolete("variable assignment", "dotted notation");
                }
                | ID '=' UNDEF in_style ';'
                {
                    var_set(NULL, $1, NULL);
                }
                | ID ID '=' UNDEF in_style ';'
                {
                    var_set($1, $2, NULL);
                    obsolete("variable assignment", "dotted notation");
                }
                ;

in_style        : /* empty */
                | IN STYLE ID
                {
                    push_style($3);
                    instyle++;
                }
                ;

/************************************************************************/
/* Styles
/************************************************************************/

style_stmt      : STYLE ID ';'
                {
                    push_style($2);
                }
                | ENDSTYLE ID ';'
                {
                    pop_style($2);
                }
                | ENDSTYLE ';'
                {
                    pop_style(NULL);
                }
		;

style_list      : style_elt
                | style_list style_elt
                ;

style_elt       : ID
                {
                    add_attr(curobj, "STYLE", $1);
                    ref_style($1);
                }
                ;

/************************************************************************/
/* Directions
/************************************************************************/

dir_list	: dir_elt
		| dir_list dir_elt
		;

dir_elt		: compass
		{
                    if (curdirs == NULL)
                        curdirs = vl_create();
                    vl_ipush(curdirs, $1);
		}
                | compass INTEGER
                {
                    if (curdirs == NULL)
                        curdirs = vl_create();
                    if ($2 <= 0)
                        err("invalid repeat count");
                    while ($2-- > 0)
                        vl_ipush(curdirs, $1);
                }
                | compass TIMES INTEGER /* obsolete */
                {
                    if (curdirs == NULL)
                        curdirs = vl_create();
                    if ($3 <= 0)
                        err("invalid repeat count");
                    while ($3-- > 0)
                        vl_ipush(curdirs, $1);
                    obsolete("'times' keyword", "just the repeat count");
                }
		;

compass		: NORTH		{ $$ = D_NORTH;	    }
		| EAST		{ $$ = D_EAST;	    }
		| SOUTH		{ $$ = D_SOUTH;	    }
		| WEST		{ $$ = D_WEST;	    }
		| NORTHEAST	{ $$ = D_NORTHEAST; }
		| NORTHWEST	{ $$ = D_NORTHWEST; }
		| SOUTHEAST	{ $$ = D_SOUTHEAST; }
		| SOUTHWEST	{ $$ = D_SOUTHWEST; }
		;

otherdir	: IN            { $$ = D_IN;   }
		| OUT           { $$ = D_OUT;  }
		| UP            { $$ = D_UP;   }
		| DOWN          { $$ = D_DOWN; }
		;

/************************************************************************/
/* Numbers and strings
/************************************************************************/

number          : INTEGER               { $$ = $1; }
                | REAL                  { $$ = $1; }
                | TRUE                  { $$ =  1; }
                | FALSE                 { $$ =  0; }
                ;

strings         : STRING
                {
                    $$ = $1;
                    repeat = 1;
                }
                | STRING INTEGER
                {
                    $$ = $1;
                    repeat = $2;
                    if ($2 <= 0)
                        err("invalid repeat count");
                }
                | STRING TIMES INTEGER /* obsolete */
                {
                    $$ = $1;
                    repeat = $3;
                    if ($3 <= 0)
                        err("invalid repeat count");
                    obsolete("'times' keyword", "just the repeat count");
                }
                ;

%%
