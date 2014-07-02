====================================================
 :program:`scr2ifm`: convert transcripts to IFM map
====================================================

.. include:: refs.rst

.. program:: scr2ifm

:program:`scr2ifm` reads one or more transcripts of an Interactive Fiction
game and produces a map of it in IFM format. It works on Infocom-style
transcripts---those produced by Inform- and TADS-generated games (and of
course Infocom games themselves). The idea is that you play your game in a
carefree manner, without worrying about mapping anything, and after the
session use :program:`scr2ifm` to get a map. Well, that's the theory
anyway.

:program:`scr2ifm` was inspired by Frobot_, another transcript-reading
mapper.  But :program:`scr2ifm` in combination with IFM does a much better
job.

:program:`scr2ifm` offers two basic ways of mapping: you can create an
initial (incomplete) map from your first transcript and then add the rest
"by hand", or you can create a set of transcripts, each exploring different
parts of the game, and then merge them all into a final map. Which you
choose is up to you.

Options
=======

These are the command-line options for :program:`scr2ifm`:

.. cmdoption:: -c file

   Append the given file of IFM and parameter commands to the end of the
   map. This allows you to fix various problems in the resulting map.

.. cmdoption:: -o file

   Write output to the given file instead of ``stdout``.

.. cmdoption:: -i

   Indent the output nicely (according to *my* definition, anyway). This
   makes things look better if you've added item and task commands.
   Default is no indentation at all.

.. cmdoption:: -l

   Add a comment to each IFM command indicating the file and line number of
   the transcript command that generated it.

.. cmdoption:: -w

   Don't print warnings.

.. cmdoption:: -h

   Print a short usage message and exit.

Operation
=========

:program:`scr2ifm` works by recognizing several key things in the
transcript:

* The commands you type. These are easy to spot, because they are preceded
  by a prompt (usually ``>``).

* When the current room changes. This is not quite so easy, but still
  fairly simple. When you enter a new room, a short room title is printed
  (for example, ``West of House``). The program looks for these short
  titles in the text that follows a command. First, a line is checked for
  an invalid format---things that never appear in a title (e.g., ``?`` or
  ``!``). Then the word count is checked---titles are hardly ever more than
  7 or 8 words long. Finally the maximum length of an uncapitalized word is
  examined---this is almost always 3 or less in titles (the length of
  "the").

* When a previously-visited room is visited again. This is the most
  difficult thing to determine, since anything might have changed in the
  room since the last visit. If there is a description, an exact
  description match is tried. If that fails, a substring match is
  attempted. If that fails, the first few words are examined to see if they
  match. If there's no description, an exact room name match is tried. This
  isn't as reliable, which is why you should always create a transcript in
  *verbose* mode.

* Special IFM commands that are (hopefully) ignored completely by the game.

Some of these checks can be tailored to a particular transcript.

.. index:: Colossal Cave; Making transcripts

Making transcripts
==================

For best results with :program:`scr2ifm`, you should follow these
guidelines when making a transcript:

* Always use *verbose* mode. If you don't, then when you revisit a room
  you've been in before, no description will be printed. In that case,
  :program:`scr2ifm` will have to rely on the room name to tell where it
  is. If there's more than one room with that name, it'll get confused (and
  so will you when you look at the map!).

* After starting the script, look around. Otherwise your starting room may
  not be included on the map, since the room description may not get
  printed.

* If there's a bend in a link (e.g., if you leave a room by going east, and
  enter the next room from the south) make sure you do the return journey.
  Otherwise, :program:`scr2ifm` won't know about the bend.

* Avoid places where different rooms have the same room description (i.e.,
  mazes). :program:`scr2ifm` can't resolve cases like this, and will assume
  there's just a single room. Note that this doesn't exclude all
  mazes---the "twisty passages, all different" maze from *Colossal Cave*
  would still work, since the descriptions are different.

* If you play the game over several sessions, and create several separate
  transcripts, pay attention to the starting rooms. If a transcript starts
  in a room already visited in the previous transcript, then its rooms will
  be added to the same map section. If not, a new map section will be
  created (and you should use the map command to give it a name).

Some games will be more amenable to :program:`scr2ifm` than others---in
general the ones with reasonable map connections, where each connection
makes sense in relation to the others. For these games, the order in which
rooms are visited will probably make no difference to the resulting
map. But for perverse games (e.g., *Colossal Cave*, above ground) the order
of visiting makes a big difference. In these cases, it's probably best to
avoid trying to get all the connections on the map---you'll spew forth lots
of IFM warnings otherwise, and the map will look awful. Sometimes it's
worth having a second attempt at a transcript, choosing different
directions to go in, to see if it improves things.

Another problem with mapping is that some games have these inconvenient
things called puzzles, which sometimes block you from exploring everywhere
so you can map it properly. Come on, IF authors, let's get our priorities
right!

IFM commands
============

While you're making your transcript, you can use a subset of IFM commands
to add extra things that :program:`scr2ifm` will recognize. (Of course, the
game almost certainly won't understand these commands, and will print a
suitable indignant reply. But that shouldn't affect the state of things.)
The commands recognized are:

:keyword:`title <name>`
       Give the map a title. The name must be in double-quotes. This
       command can appear anywhere, and has the same effect each time.

:keyword:`map <name>`
       Indicate that the current room starts a new map section with the
       given name. The name must be in double-quotes. This is useful if it
       seems like the map divides into different sections (e.g., the floors
       of a house).

       If you use the :keyword:`map` command, you'll end up with two or
       more map sections. The first map section is the one that contains
       the very first room, and will have a default name unless you give it
       one using another :keyword:`map` command.

:keyword:`item <name> [attrs]`
       Declare an item in the current room.  The name must be in
       double-quotes. If you don't give it a tag attribute, one is
       generated automatically by changing all invalid tag characters in
       the name to underscores.

:keyword:`item [attrs]`
       Add the given attributes to the last declared item, if any.

:keyword:`item delete`
       Delete the last declared item, if any.

:keyword:`task <name> [attrs]`
       Declare a task in the current room in a similar manner to items. The
       name must be in double-quotes.

:keyword:`task [attrs]`
       Add the given attributes to the last declared task, if any.

:keyword:`task delete`
       Delete the last declared task, if any.

Note that :program:`scr2ifm` knows almost nothing about IFM command
syntax. If there's a syntax error in a command, you'll have to manually
edit the resulting transcript (or delete the item/task and do it again).

Fixing problems
===============

The map produced by :program:`scr2ifm` will not be perfect in a lot of
cases. When running it through IFM you might get warnings about overlapping
rooms, or crossed link lines. These problems are usually fixed by
stretching the link lines of some of the rooms. Some overlapping-room
problems are caused by the program making a bad choice of direction to put
an up/down/in/out connection in. Another problem might be not recognizing
when a room is the same as a previously-visited room, because the room
description changed too much between visits.

You can fix most of these problems by creating a command file and
specifying it with the :keyword:`-c` option.  There are two types of entry:
:program:`scr2ifm` commands and IFM commands. In a command file, blank
lines and comment lines (those starting with a ``#``) are ignored. If a
command isn't recognized as a :program:`scr2ifm` command, it's assumed to
be an IFM command, and is passed straight into the output verbatim. You can
use these to resolve conflicts in the generated map, by stretching certain
links or hiding them altogether.

Here's a list of the commands available:

:keyword:`is_room TEXT`
       Declare a line of text that is actually the name of a room. Normally
       room names get recognized properly, but there may be some special
       cases that aren't.

:keyword:`not_room TEXT`
       Declare a line of text that definitely isn't the name of a
       room. This is for opposite cases to the above.

:keyword:`use_name TEXT`
       Use only the room name to decide whether a room has been previously
       visited. This is for cases where a room description changes too much
       between visits to be called the same room. Note that this implies
       that there can only ever be one room with this name.

:keyword:`set_option LETTER`
       This is a convenience that lets you specify :program:`scr2ifm`
       command-line options in the command file instead. E.g.,
       :keyword:`set_option i` will indent output nicely.

:keyword:`set_param VAR = VALUE`
       This evaluates the given Perl expression. You can use this to set
       various parameters that control how the transcript is parsed (see
       below).  Setting a value of :keyword:`undef` will remove that
       parameter (so that it won't be used).

Here's a list of the parameters that control how the transcript is parsed,
and their defaults. You can use the :program:`scr2ifm` :keyword:`set_param`
command to set them in the command file.

:keyword:`$name_remove`
       Matched text to remove before seeing if a line of text is a room
       name. This is for getting rid of stuff like " (in the dodgem
       car)". Default: ``\s+\(.+\)``

:keyword:`$name_maxwords`
       Maximum no. of words in a room name. Default: 8

:keyword:`$name_maxuncap`
       Maximum length of an uncapitalized word in a room name. Default: 3

:keyword:`$name_invalid`
       Regexp which, if matched, signals an invalid room name. Default:
       ``[.!?"]``

:keyword:`$desc_minwords`
       Minimum no. of matching words required to match a room
       description. Default: 20

:keyword:`$cmd_prompt`
       Regexp matching a command prompt. Default: ``^>\s*``

:keyword:`$cmd_look`
       Regexp matching a 'look' command (case-insensitive). Default:
       ``^l(ook)?$``

:keyword:`$cmd_undo`
       Regexp matching an UNDO command (case-insensitive). It's assumed
       that only a single level of UNDO is available. Default: ``^undo$``

:keyword:`$cmd_teleport`
       Regexp matching commands that are known to cause a teleport to an
       arbitrary room (case-insensitive). Default: ``^(restart|restore)$``

Writing the map
===============

Output from :program:`scr2ifm` is as follows. First, if the title of the
map was specified with the :keyword:`title` command, it is printed. Then,
if there's more than one map section, a list of map section titles is
printed.

Next, there's a list of IFM commands derived from the transcript. Each time
a new room is seen, a room command is output for it. Each room is given a
tag name formed by the initials of each capitalized word in its name. To
make tags unique, the second and subsequent occurrences of a tag name have
that number appended.  For example, room "West of House" would get tag
``WH``.

If a movement command was up, down, in or out, then a compass direction is
chosen for it that doesn't clash with other rooms (if possible) and the
:keyword:`go` attribute is used to mark its real direction. If a movement
command isn't recognized, the same is done except this time the
:keyword:`cmd` attribute is used to record the command instead.

If movement is seen between two already-visited rooms, a :keyword:`link`
command is output (or a :keyword:`join`, if the rooms appear to be on
different map sections). Link tags are built by concatenating the room tags
of the linked rooms with an underscore in between. Link movement is dealt
with in the same manner as for rooms.

If the special :keyword:`item` or :keyword:`task` commands are seen, then
an item or task command is output following the room that it appears in. If
an item or task hasn't been given a tag, it is assigned one. The tag is
made by translating all invalid tag characters in the name to underscores,
capitalizing (to avoid clashes with IFM commands) and, if the tag clashes
with a previously-defined tag, adding a numerical suffix to make it
unique. For example, "black rod" might get a tag ``Black_rod_2``.

Finally, the IFM commands from the file indicated by the ``-c`` option are
echoed (if any).

.. index:: Colossal Cave; Example scr2ifm session

Example session
===============

Here's a simple example of :program:`scr2ifm` in action on *Colossal
Cave*. First, the transcript in file :file:`advent.scr` (with annotation):

.. literalinclude:: examples/advent1.scr
   :language: transcript

The previous two commands are recommended after starting the transcript.

.. literalinclude:: examples/advent2.scr
   :language: transcript

The previous four commands declare the given items to the program. You
don't have to bother with this if you don't want item locations on your
map.

.. literalinclude:: examples/advent3.scr
   :language: transcript

Note that in this game we don't go in all possible directions---there's no
logic to the connections, and it'd just make a complete mess on the map.

.. literalinclude:: examples/advent4.scr
   :language: transcript

That last command declared a task---again, you don't have to bother with
this if you don't want to.

.. literalinclude:: examples/advent5.scr
   :language: transcript

Oops! Stumbled into the dark by mistake. Without the ``UNDO``, that would
have meant ``Darkness`` appeared on the map as a room.

.. literalinclude:: examples/advent6.scr
   :language: transcript

Goodness me! That was unexpected---we've been teleported. The link almost
certainly won't look good on the map. We have the option of UNDOing, right
now, or fixing the map later. Let's do the latter.

.. literalinclude:: examples/advent7.scr
   :language: transcript

Ok, now to create a map. To do that, use this command::

    scr2ifm -o advent.ifm advent.scr

To check for problems, just type::

    ifm advent.ifm

On the first run through, there are complaints about an overlapping room,
due to a bad choice of direction for the "down" link from ``Outside
Grate``, and the expected problems with the link between the ``Debris
Room`` and the ``Building``. A command file, :file:`advent.cmd`, will fix
these problems:

.. literalinclude:: examples/advent.cmd
   :language: ifm

Now, if we invoke::

    scr2ifm -o advent.ifm -i -c advent.cmd advent.scr

we get the following map:

.. literalinclude:: examples/advent.ifm
   :language: ifm

Printed, it looks like this:

.. image:: images/advent.png
   :align: center

Limitations
===========

Here's a list of things that :program:`scr2ifm` doesn't currently deal
with:

* Mazes. Different maze rooms that look identical (i.e., have identical
  names and descriptions) will simply appear as a single room.

* One-way links. These aren't detected, so you'll have to add the
  appropriate IFM attribute yourself (in a command file, or by fixing the
  map directly).
