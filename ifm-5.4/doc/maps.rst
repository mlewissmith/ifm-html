=============
 Making maps
=============

.. highlight:: ifm

This section gives you a tour of the main commands for making maps.  It's
not complete; see :doc:`language` for a full list of commands.

Introduction to maps
====================

The traditional Infocom-style way of drawing Interactive Fiction maps is
the "boxes-and-lines" method, like this:

.. image:: images/infocom.png
   :align: center

This is the style of map that IFM produces.  Rooms are represented as boxes
on a square grid, and links between the rooms are drawn as lines connecting
them.  Links emanate from rooms in any of the eight standard compass
directions, and also follow the grid.  In the following sections, we'll
introduce the IFM commands that can be used to draw this example map.

.. index::
   single: room; Adding rooms
   single: tag; Adding rooms
   single: dir; Adding rooms
   single: from; Adding rooms
   single: oneway; Adding rooms

Adding rooms
============

To draw the example map from the previous section, you first choose an
arbitrary start location: the kitchen (when mapping a real game, you'd
choose your actual start location).  To add the kitchen, just type this::

    room "Kitchen";

Now you're in the kitchen.  Suppose, if this were a real game, that you
first went south to explore the garage.  That can be added to the map like
this::

    room "Garage" dir south;

Now you've said which way you went to get to the garage, and since you were
in the kitchen, IFM knows that the garage is south of the kitchen.  By the
way, :keyword:`south` can be abbreviated :keyword:`s` (and similarly for
all other directions), just like in the games.

Ok, you're in the garage.  Unfortunately, that's a dead end and you have to
retrace your steps to the kitchen.  You've already mapped that, so there's
no need to do anything.  Now you head off east to the lounge.  Now, you're
moving from the kitchen again but IFM thinks you're in the garage (IFM's
idea of "where you are" is always the last room mentioned).  You need a way
of referring to the kitchen again---to do that, you add a *tag* to it by
changing the "kitchen" line like this::

    room "Kitchen" tag Kitchen;

The tag name can be any name you like.  You might think that you could
refer to the kitchen by using the name in quotes, but that would mean you
could never have two distinct rooms with the same name.  Another advantage
of tags is that they can be much shorter than the real room names.  The tag
``K`` would be just as valid in the example above (though not as readable).

Now you can refer to the kitchen by its tag, so you can move east from it
into the lounge like this::

    room "Lounge" dir e from Kitchen;

The from clause tells IFM where you're moving from.  If it's omitted, it
assumes you're moving from the last room mentioned.

Continuing your exploration, you move south into the dining room::

    room "Dining Room" dir s;

You exit the dining room to the east, and turn a corner north before
entering the study.  How can you represent the corner faithfully on the
map?  Like this::

    room "Study" dir e n;

This says that you move east, then north, to get to the study.  Now, what
if someone locked the study door behind you and the only way out was
through the window?  That's a one-way trip into the study, which you can
indicate using the :keyword:`oneway` attribute like this::

    room "Study" dir e n oneway;

This is indicated on the map by an arrow.

.. index::
   single: link; Adding links

Adding links
============

The map as it stands is not complete---the diagonal link between the
kitchen and the dining room is missing (because you didn't go that way in
visiting the kitchen or the dining room).  To add it, you need to modify
the dining room command like this::

    room "Dining Room" dir s link Kitchen;

The :keyword:`link` clause creates a straight-line link between the current
room and the room with the specified tag name (in this case, the kitchen).

Note that if this link needed to turn corners, as in the study example
above, then that method of linking the rooms wouldn't have worked.  In that
case, you'd have to use the stand-alone :keyword:`link` command.  For
example::

    link Diner to Kitchen dir n nw;

This assumes you've given the dining room the tag name ``Diner``.  The link
starts off going north, then turns northwest, and finally goes toward the
kitchen.  Note that in a :keyword:`link` command, if you omit the final
direction which leads to the linked room, it is added automatically.

.. index::
   single: go; Other directions
   single: up; Other directions
   single: down; Other directions
   single: in; Other directions
   single: out; Other directions

Other directions
================

Suppose that there were steps down from the kitchen into the garage, and
that you wanted to indicate that you could up or down as well.  You could
do that using the go clause, like this::

    room "Garage" dir s go down;

This indicates that the actual direction travelled is downwards, but it is
still represented as south on the map.  The :keyword:`go` clause accepts
:keyword:`up`, :keyword:`down`, :keyword:`in` and :keyword:`out`.  As with
compass directions, :keyword:`up` and :keyword:`down` may be abbreviated as
:keyword:`u` and :keyword:`d`.

.. index::
   single: exit; Room exits

Room exits
==========

At various points in a game, you arrive in a room with many directions to
explore.  It is useful to be able to mark some of these directions as
unexplored, so that you can come back and explore them later.  You could
mark these by creating dummy rooms in those directions, but this is
tedious.  Alternatively, you can use the :keyword:`exit` clause, like
this::

    room "Dining Room" dir s exit nw e;

This says that there are two unexplored exits from this room, in the
northwest and east directions.  When a map is drawn, this fact will be
displayed by a small line poking out of the room in those directions.

When you come to actually explore those directions, and add links to new
rooms, the corresponding room exit markers will no longer be drawn.  So you
can leave the :keyword:`exit` clauses in if you want.

.. index::
   single: map; Map sections
   single: join; Map sections
   single: nolink; Map sections

Map sections
============

In IFM, rooms are divided into groups called *map sections*.  Each room in
a map section has an explicit spatial relationship to all the other rooms
in that section.  A room which is obtained by moving via a :keyword:`dir`
clause from a previous room is on the same map section as the previous
room, since its co-ordinates can be calculated relative to it.

There are several reasons why it might be a good idea to split a game map
into different sections:

* Some maps can be very large, and may not look good on a single piece of
  paper.

* It might be awkward to put rooms in relation to each other because of,
  say, a lot of up/down connections which have to be "flattened out".

* The game might naturally divide into sections---a prologue, middle-game
  and end-game, for example.

IFM manages independent map sections automatically, by deciding which rooms
are on which section.  No special command is needed to start a new map
section---simply define a room which has no connection to any previous
room, by leaving out the dir clause (note that that's how the kitchen
starts out, in the example).

Rooms on different map sections are completely separate, and you may not
link them via the link command.  However, you can indicate where a room on
one section is connected to a room on another, using the join command::

    join Room1 to Room2;

As usual, ``Room1`` and ``Room2`` are tag names.  You can also use
:keyword:`join` as a clause in a :keyword:`room` command (usually done with
the room starting in a new section)::

    room "Basement" join Ground_Floor;

The "joined" status of the two rooms is indicated after their description
text; the default is to use an increasing number.

Each map section can be given a title using the :keyword:`map` command,
like this::

    map "Kensington Gardens";

This names the next map section that hasn't been named.  Note that you
should have as many :keyword:`map` commands as you have map sections,
although this isn't enforced---any names that are missing will be assigned
default names, and extra names will be ignored.  It's conventional to give
a :keyword:`map` command just before starting a new map section.

In rare circumstances (e.g., a three-dimensional maze) you may need to have
rooms on the same map section which are not connected to each other.  The
room :keyword:`dir` clause creates an implicit link from the previous room
by default, but you can stop this from happening by using the
:keyword:`nolink` attribute.  As a trivial example::

    room "One Side of Wall" tag this_side;
    room "Other Side of Wall" dir e nolink tag other_side;
    room "Underground Passage" tag passage_1;
    room "Underground Passage" tag passage_2 dir e;
    join this_side to passage_1 go down;
    join passage_2 to other_side go up;

In this example, there are two map sections: above ground, and below
ground.  But the two above-ground rooms are not connected directly.

.. index::
   single: item; Adding items
   single: note; Adding items
   single: in; Adding items

Adding items
============

As well as rooms, IFM can indicate the initial rooms of various items found
in the game.  To add an item, use the :keyword:`item` command like this::

    item "Spoon" in Kitchen;

The :keyword:`in` clause can be omitted, and then the room defaults to the
last room mentioned.  You can also add an arbitrary note to each item (e.g.,
to remind you what it's for) using the :keyword:`note` attribute::

    item "Spoon" in Kitchen note "Stirs tea";

Here's the completed map description for the above example, with a few
other items thrown in:

.. literalinclude:: examples/example.ifm

And this is what it looks like as rendered by IFM:

.. image:: images/example.png
   :align: center

.. index::
   single: dir; Mapping problems

.. index:: Colossal Cave; Mapping problems

Mapping problems
================

After creating a map from a real game and sending the results through IFM,
you may get warnings which talk about things overlapping.  This is due to
two rooms, or a room and a link, wanting to occupy the same space on the
map.  There are several ways that this could occur:

* The game designer made some room links longer than others, and you
  haven't taken that into account.  To extend the length of a link, just add
  a length indicator after the direction in the dir clause (e.g.,
  :keyword:`dir e 2` instead of :keyword:`dir e`).

* One of the links turned a corner, so that the direction you use to go
  back isn't the opposite of the one you used to get here.  In that case,
  you need to add the corner-turn in the link (e.g., :keyword:`dir e s`
  instead of :keyword:`dir e`).

* The map is multi-level, in which case it's probably best to split it into
  different map sections.

* The map is just weird. *Colossal Cave* is a good example, particularly
  the above-ground section and the mazes.  There seems to be no logic tying
  the rooms together.  You're on your own.
