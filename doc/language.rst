==========
 Language
==========

This section gives a complete detailed description of the IFM language.

Symbols
=======

In the following sections, these symbols are used:

:keyword:`ID`
	 A name starting with a letter, followed by any combination of
	 upper- and lowercase letters or numbers or an underscore. For
	 example, ``Dense_Forest``. IDs are not allowed to clash with
	 reserved words (e.g., ``room``, or ``tag``). One way to avoid this
	 is to capitalize all tags (reserved words are all in lowercase).

:keyword:`STRING`
	 Any sequence of characters, in double-quotes.  For example, ``"Black
	 Rod"``. To get a double-quote inside a string, quote it using a
	 backslash, like this::

             "Ground Floor, \"A\" Block"

         You can continue strings on several lines---a
	 newline-and-whitespace sequence is translated into a single space,
	 just like in TADS and Inform.

:keyword:`NUMBER`
	 A number. If the context requires an integer, the number is
	 silently rounded to the larges integer not greater than this
	 value.

:keyword:`COMPASS`
	 A compass direction, which can be abbreviated or in full (e.g.,
	 ``n``, ``se``, ``northwest``, etc).

:keyword:`OTHERDIR`
	 One of ``up``, ``down``, ``in`` or ``out``.

:keyword:`[...]`
	 An optional part.

:keyword:`A | B`
	 Either A or B.

Format
======

IFM generally has a free-format layout---i.e., whitespace may be placed
anywhere to increase readability. The only exception is inside quoted
strings, where spaces are significant. Comments may be added, starting with
a hash (``#``) and continuing to the end of the line. All commands are
terminated with a semicolon.

Control
=======

The overall title of the map may be set using the command::

    title STRING;

If a map has several sections, you can set the title of each section using
the command::

    map STRING;

This sets the title of the next map section. If you use this command at
all, then the number of uses should be the same as the actual number of map
sections. It's conventional (but not required) to put the :keyword:`map`
command just before the room that starts a new map section.

If your map uses features that are only present in later versions of IFM,
you can indicate that up front by using the command::

    require NUMBER;

Then, if the IFM version number is less than this number, parsing will
abort immediately, avoiding lots of potentially confusing syntax errors.

.. versionadded:: 5.0
   The :keyword:`require` keyword.

Tags
====

All IFM objects may be given tag names, so that you can refer to them in
other commands. Tags for different types of object are independent---for
example, you could have a room and an item with the same tag.  However,
tags for similar objects must be unique.

In many cases, you are allowed to refer to a tag name anywhere, even
earlier in the file that you defined it (as long as the tag is defined
*somewhere*!). Exceptions are the room_ :keyword:`from ID` clause and tags
in commands that modify existing objects---these tags must be defined
before being used.

Special names
=============

There are three special names that can refer to IFM objects in certain
contexts:

:keyword:`last`
    May be used to refer to the last object of a certain type that was
    defined in a previous statement.

:keyword:`it`
    May be used to refer to the most recent object in the current
    statement.

:keyword:`them`
    May be used to refer to all the items mentioned so far in the current
    statement.

    .. versionadded:: 5.3

Commands
========

There are five different types of object in IFM: rooms, items, links, joins
and tasks. Each is created using its own command, the general format of
which is::

    <type> <body> [attribute-list];

For rooms, items and tasks, ``<body>`` is just a string description. For
links and joins, it specifies two room tags to link or join together.

Many of the attributes or objects accept a list of tags as arguments. All
of these, if specified more than once in the same object, concatenate the
lists together.

Once an object has been declared with a tag name, its attributes can be
modified by later commands referring to that tag, like this::

    <type> ID [attribute-list];

where ``ID`` is the tag name of the object. Note that the tag must be
defined earlier in the file than it is used.

.. _room:

Rooms
-----

A new room is added using the command::

    room STRING [attribute-list];

where ``STRING`` is a description of the room. Room attributes can be:

.. index:: tag

:keyword:`tag ID`
       Give the room a tag name, so that you can refer to it elsewhere.

.. index:: dir

:keyword:`dir COMPASS [NUMBER] [COMPASS [NUMBER]...] [from ID]`
       Specify the position of the room relative to the room with the given
       tag ID (which must be defined before it is used).  If no
       :keyword:`from` clause is specified, the last defined room is used
       instead. There can be more than one direction given---the new room
       is placed relative to the previous one using them. Following a
       direction with a number means to repeat it that many times.

       The :keyword:`dir` clause creates an implicit link between this room
       and the previous one. Some of the room attributes below behave
       differently depending on whether they appear before or after the
       :keyword:`dir` clause in the attribute list.

       If the room is given a tag name, then the implicit link will be
       given the same tag.

.. index:: link

:keyword:`link ID [ID...]`
       Specify other rooms that this room links to.  Note that this creates
       a link with no special attributes---use the standalone
       :keyword:`link` command for that.

.. index:: join

:keyword:`join ID [ID...]`
       Specify rooms on other map sections that this room joins to. Note
       that this creates a join with no special attributes---use the
       standalone :keyword:`join` command for that.

.. index:: exit

:keyword:`exit COMPASS [COMPASS...]`
       Indicate which other directions the room has exits in. Room exits in
       a particular direction are marked on the map only if there is no
       link going to or from the room in that direction.

.. index:: note

:keyword:`note STRING`
       Append a note to the room's note list.

.. index:: score

:keyword:`score NUMBER`
       Indicate that you score the specified number of points when visiting
       this room for the first time.

.. index:: need

:keyword:`need ID [ID...]`
       If this appears before a :keyword:`dir` clause, indicate that you
       can only enter this room after getting the specified items. If it
       appears afterwards, it applies to the implicit link instead.

.. index:: after

:keyword:`after ID [ID...]`
       If this appears before a :keyword:`dir` clause, indicate that you
       can only enter this room after doing the specified tasks. If it
       appears afterwards, it applies to the implicit link instead.

.. index:: before

:keyword:`before ID [ID...]`
       If this appears before a :keyword:`dir` clause, indicate that you
       can only enter this room before doing the specified tasks. If it
       appears afterwards, it applies to the implicit link instead. Those
       tasks are marked unsafe.

.. index:: leave

:keyword:`leave ID [ID...]`
       If this appears before a :keyword:`dir` clause, indicate that the
       specified items, if carried, must be left behind when entering the
       room. If it appears afterwards, it applies to the implicit link
       instead.

:keyword:`leave all [except ID [ID...]]`
       As above, except indicate that all items must be left behind. The
       :keyword:`except` clause can be used to omit specific items.

.. index:: go

:keyword:`go OTHERDIR`
       Indicate that the link to this room is in the specified direction.

.. index:: cmd

:keyword:`cmd STRING`
       Specify the command you type to move to this room from the previous
       one. If no :keyword:`cmd` clause is given, the command is deduced
       from the :keyword:`go` clause. If that isn't specified, the command
       will be deduced from the :keyword:`dir` clause.

:keyword:`cmd from STRING`
       As above, but this specifies the command to go in the other
       direction. This defaults to the :keyword:`cmd to` command, if
       specified.

:keyword:`cmd to STRING`
       This is identical to :keyword:`cmd` on its own, and only exists for
       symmetry.

.. index:: oneway

:keyword:`oneway`
       Indicate that the return journey from this room to the previous one
       is not possible.

.. index:: length

:keyword:`length NUMBER`
       Indicate that the direction link to this room has the specified
       length (default 1). This only affects the calculation of the nearest
       task_ when :ref:`solving the game <finding_solution>`.

.. index:: start

:keyword:`start`
       Indicate that this is the room the player starts in. Default is for
       the first room mentioned to be the start room. If more than one room
       has this attribute, the last one declared takes precedence.

.. index:: finish

:keyword:`finish`
       Indicate that entering this room finishes the game.

.. index:: nodrop

:keyword:`nodrop`
       Indicate that no items should be voluntarily dropped in this room.

       .. versionadded:: 5.0

.. index:: nolink

:keyword:`nolink`
       Indicate that this room does not have an implicit link with the
       previous one via the :keyword:`dir` clause.

.. index:: nopath

:keyword:`nopath`
       Indicate that the implicit link from this room should not be used by
       the game solver.

.. index:: style

:keyword:`style ID [ID...]`
       Add a list of display styles_ to the room (and also the implicit
       link, if any).

Items
-----

An item is introduced using the command::

    item STRING [attribute-list];

where :keyword:`STRING` is the item description. Item attributes can be:

.. index:: tag

:keyword:`tag ID`
       Give the item a tag name, so you can refer to it elsewhere.

.. index:: in

:keyword:`in ID`
       Set the initial location of this item. Default is the last defined
       room. If there is no last room (i.e., an item was declared before
       any room was declared), then this item is initially carried by the
       player.

.. index:: note

:keyword:`note STRING`
       Append a note to the item's note list.

.. index:: score

:keyword:`score NUMBER`
       Indicate that you get points the first time you pick this item up.

.. index:: hidden

:keyword:`hidden`
       Indicate that this item is not immediately obvious when entering the
       room.

.. index:: keep

:keyword:`keep`
       Indicate that this item shouldn't ever be dropped (no "drop" task
       should be generated).

:keyword:`keep with ID [ID...]`
       Indicate that the item shouldn't be dropped until all the other
       specified items have left the inventory.

       .. versionadded:: 5.0

:keyword:`keep until ID [ID...]`
       Indicate that the item shouldn't be dropped until all the other
       specified tasks have been done.

       .. versionadded:: 5.0

.. index:: ignore

:keyword:`ignore`
       Indicate that this item should be ignored when trying to find a
       solution (i.e., never go out of your way to pick it up).

       .. versionadded:: 5.0

.. index:: given

:keyword:`given`
       Indicate that this item didn't have to be picked up when it entered
       the inventory (no "get" task should be generated).

       .. warning:: 

       	  This attribute is obsolete---you should use the task_
       	  :keyword:`give` clause instead.

.. index:: lost

:keyword:`lost`
       Indicate that this item wasn't dropped when it left the inventory
       (no "drop" task should be generated).  Normally you should use the
       task_ :keyword:`drop` or :keyword:`lose` clauses instead. The only
       use for this attribute is for items that are left behind due to a
       :keyword:`leave` clause.

.. index:: need

:keyword:`need ID [ID...]`
       Indicate that you can only pick this item up after getting the
       specified items.

.. index:: after

:keyword:`after ID [ID...]`
       Indicate you can only pick this item up after the specified tasks
       are done.

.. index:: before

:keyword:`before ID [ID...]`
       Indicate you can only pick this item up before the specified tasks
       are done.

.. index:: finish

:keyword:`finish`
       Indicate that getting this item finishes the game.

.. index:: style

:keyword:`style ID [ID...]`
       Add a list of display styles_ to the item.

Links
-----

You can create extra room links using the command::

    link ID to ID [attribute-list];

and the following attributes may be specified:

.. index:: tag

:keyword:`tag ID`
       Give the link a tag name, so you can refer to it elsewhere.

.. index:: dir

:keyword:`dir COMPASS [COMPASS...]`
       Set the intermediate directions that this link travels in, in the
       same manner as for rooms. Note that if you omit the final direction
       to the linked room, it is added automatically.

.. index:: go

:keyword:`go OTHERDIR`
       Indicate that the link is in the specified direction.

.. index:: cmd

:keyword:`cmd STRING`
       Specify the command you type to go in this direction. If no
       :keyword:`cmd` clause is given, the command is deduced from the
       :keyword:`go` clause. If that isn't specified, the command will be
       deduced from the :keyword:`dir` clause.

:keyword:`cmd from STRING`
       As above, but this specifies the command to go in the other
       direction. This defaults to the :keyword:`cmd to` command, if
       specified.

:keyword:`cmd to STRING`
       This is identical to :keyword:`cmd` on its own, and only exists for
       symmetry.

.. index:: oneway

:keyword:`oneway`
       Indicate that this is a one-way link, in a similar manner to the
       room attribute of the same name.

.. index:: hidden

:keyword:`hidden`
       Indicate that this link should not be drawn on the map. Hidden links
       are still used when solving the game.

.. index:: nopath

:keyword:`nopath`
       Indicate that this link should not be used by the game solver.

.. index:: length

:keyword:`length NUMBER`
       Indicate that this link has the specified length (default 1). This
       only affects the calculation of the nearest task_ when :ref:`solving
       the game <finding_solution>`.

.. index:: need

:keyword:`need ID [ID...]`
       Indicate that you can only go in this direction after getting the
       specified items.

.. index:: after

:keyword:`after ID [ID...]`
       Indicate that you can only go in this direction after doing the
       specified tasks.

.. index:: before

:keyword:`before ID [ID...]`
       Indicate that you can only go in this direction before doing the
       specified tasks. These tasks are marked unsafe.

.. index:: leave

:keyword:`leave ID [ID...]`
       Indicate that the specified items, if carried, must be left behind
       when using this connection.

:keyword:`leave all [except ID [ID...]]`
       As above, except indicate that all items must be left behind. The
       :keyword:`except` clause can be used to omit specific items.

.. index:: style

:keyword:`style ID [ID...]`
       Add a list of display styles_ to the link.

Joins
-----

There is a standalone join command which joins two rooms on different map
sections::

    join ID to ID [attribute-list];

The following attributes may be specified:

.. index:: tag

:keyword:`tag ID`
       Give the join a tag name, so you can refer to it elsewhere.

.. index:: go

:keyword:`go COMPASS | OTHERDIR`
       Indicate that the join to this room is in the specified direction.

.. index:: cmd

:keyword:`cmd STRING`
       Specify the command you type to go in this direction. If no
       :keyword:`cmd` clause is given, the command is deduced from the
       :keyword:`go` clause. If that isn't specified, the command will be
       undefined.

:keyword:`cmd from STRING`
       As above, but this specifies the command to go in the other
       direction. This defaults to the :keyword:`cmd to` command, if
       specified.

:keyword:`cmd to STRING`
       This is identical to :keyword:`cmd` on its own, and only exists for
       symmetry.

.. index:: oneway

:keyword:`oneway`
       Indicate that this is a one-way join, in a similar manner to the
       room attribute of the same name.

.. index:: hidden

:keyword:`hidden`
       Indicate that this join should not be drawn on the map. Hidden joins
       are still used when solving the game.

.. index:: nopath

:keyword:`nopath`
       Indicate that this join should not be used by the game solver.

.. index:: length

:keyword:`length NUMBER`
       Indicate that this join has the specified length (default 1). This
       only affects the calculation of the nearest task_ when :ref:`solving
       the game <finding_solution>`.

.. index:: need

:keyword:`need ID [ID...]`
       Indicate that you can only go in this direction after getting the
       specified items.

.. index:: after

:keyword:`after ID [ID...]`
       Indicate that you can only go in this direction after doing the
       specified tasks.

.. index:: before

:keyword:`before ID [ID...]`
       Indicate that you can only go in this direction before doing the
       specified tasks. These tasks are marked unsafe.

.. index:: leave

:keyword:`leave ID [ID...]`
       Indicate that the specified items, if carried, must be left behind
       when using this connection.

.. index:: leave

:keyword:`leave all [except ID [ID...]]`
       As above, except indicate that all items must be left behind. The
       :keyword:`except` clause can be used to omit specific items.

.. index:: style

:keyword:`style ID [ID...]`
       Add a list of display styles_ to the join.

.. _task:

Tasks
-----

You can indicate tasks which need to be done in order to solve the game
using the command::

    task STRING [attribute-list];

and these are the available attributes:

.. index:: tag

:keyword:`tag ID`
       Give the task a tag name, so you can refer to it elsewhere.

.. index:: in

:keyword:`in ID`
       Specify the room the task must be done in. If this clause is
       omitted, it defaults to the last defined room. You can use the
       special word :keyword:`any` to indicate that the task may be
       performed anywhere. A task declared before any room is equivalent to
       saying :keyword:`in any`.

.. index:: need

:keyword:`need ID [ID...]`
       Indicate that the specified items are required before you can do
       this task.

.. index:: after

:keyword:`after ID [ID...]`
       Indicate that this task can only be done after all the specified
       tasks have been done.

.. index:: follow

:keyword:`follow ID`
       Indicate that this task must be done immediately after the specified
       one. Not even a "drop item" task is allowed in between.

.. index:: do

:keyword:`do ID [ID...]`
       Indicate that doing this task also causes the specified other tasks
       to be done (if they aren't done already). These other tasks are done
       immediately, without regard for any prerequisite items or tasks they
       might need, and their effects are carried out---including any
       :keyword:`do` clauses they might have, recursively.

       .. versionadded:: 5.0

.. index:: get

:keyword:`get ID [ID...]`
       Indicate that doing this task enables you to get the specified
       items, and must be done before you can get them.

.. index:: give

:keyword:`give ID [ID...]`
       Indicate that doing this task puts the specified items straight into
       your inventory, wherever they happen to be.

.. index:: lose

:keyword:`lose ID [ID...]`
       Indicate that doing this task causes the specified items to be
       lost. This implies that all tasks which need these items must be
       done before this one.

.. index:: drop

:keyword:`drop ID [ID...] [in ID] [until ID [ID...]]`
       Indicate that doing this task drops the specified items in the
       current room (or the room indicated by the in clause) if you're
       carrying them. No "drop" message is generated. If there's an
       :keyword:`until` clause, you can't retrieve the items until the
       specified tasks have been done.

:keyword:`drop all [except ID [ID...]] [in ID] [until ID [ID...]]`
       As above, but drop everything you're carrying. The :keyword:`except`
       clause can be used to omit specific items.

.. index:: goto

:keyword:`goto ID`
       Indicate that you get "teleported" to the specified room when this
       task is done. This happens after :keyword:`give` and :keyword:`drop`
       actions.

.. index:: safe

:keyword:`safe`
       Mark this task as safe---i.e., one that can't cause the game solver
       to get stuck.

.. index:: ignore

:keyword:`ignore`
       Don't ever do this task explicitly when solving the game. The task
       may still be done via a :keyword:`do` action.

       .. versionadded:: 5.0

.. index:: finish

:keyword:`finish`
       Indicate that doing this task finishes the game.

.. index:: score

:keyword:`score NUMBER`
       Indicate that you get the specified score for doing this task.

.. index:: note

:keyword:`note STRING`
       Append a note to the task's note list.

.. index:: cmd

:keyword:`cmd STRING [NUMBER]`
       Specify the exact command you type to do the task. If a number
       follows the command, do the command that many times. Multiple
       :keyword:`cmd` clauses concatenate into a list of commands.

:keyword:`cmd none`
       Indicate that no command is required to do this task.

.. index:: style

:keyword:`style ID [ID...]`
       Add a list of display styles_ to the task.

.. index:: true, false

Variables
=========

Various aspects of output are controlled by :doc:`vars`.  These are set
using the following syntax::

    [FORMAT.]ID = NUMBER | STRING | true | false | undef [in style ID];

``FORMAT``, if specified, is the name of a specific output format---the
variable then applies only to that output format.

``ID`` is the name of the variable, and it can take a numeric or string
value. Note that setting a variable to the value :keyword:`undef`
effectively removes it.

The values :keyword:`true` and :keyword:`false` correspond to the integer
values 1 and 0 respectively.

If the :keyword:`style` clause is present, this means to only set the
variable to this value in the specified style.

.. versionadded:: 5.3
   The :keyword:`true` and :keyword:`false` keywords.

.. _style:

.. index::
   pair: Styles; style
   pair: Styles; endstyle

Styles
======

A *style* defines a set of variables with particular values, so that those
values can be referred to together. IFM keeps track of the currently active
list of styles, and there are two commands which change this list. The
command::

    style ID;

pushes the specified style onto the style list. This style becomes the
current style. Any IFM objects declared while a style list is in force will
by default be output in those styles. Any variable setting is by default in
the current style (though you can specify a particular style using the
:keyword:`in style` clause).

The command::

    endstyle [ID];

pops the current style from the style list. The previous style on the list
(if any) becomes the current style. The ``ID``, if specified, should match
the ``ID`` in the corresponding style command, or a warning is given.

Each display style has its own set of values for customization
variables. On output, when the value of a variable is needed for displaying
an object, the style list for that object is searched in reverse order of
declaration. The value used is from the first style to define this
variable. If no style defines it, then the default value is used.

If a style is referenced by an object but not defined anywhere in the
input, then its definition is assumed to be in a separate file, which is
searched for using the standard search path. The name of this file is
formed by adding a :file:`.ifm` suffix to the style name. If the file is
not found, or it does not define the required style, a warning is given.
