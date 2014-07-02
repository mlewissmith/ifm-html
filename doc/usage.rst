.. _usage:

===========
 Using IFM
===========

.. include:: refs.rst

Running the program
===================

.. program:: ifm

IFM is run from the command line.  The general form of the command is::

    ifm [options] [file...]

On startup, IFM does the following.  Firstly, the system initialization
file is read.  This sets defaults used by everyone.  This file is called
:file:`ifm-init.ifm`, and is found by searching a standard set of
directories.  You can adjust the search path by setting the environment
variable :envvar:`IFMPATH`.

Then, if you have a personal initialization file, that is read.  This file
is found in the directory specified by your :envvar:`HOME` environment
variable.  It can be called :file:`.ifmrc` or :file:`ifm.ini`; if both
exist, they are both read.  You can use this file to customize the default
variable settings for different types of output.

Then input from the file(s) on the command-line is read.  If no files were
specified, :keyword:`stdin` is read.  A filename equal to :file:`-` also
indicates that :keyword:`stdin` should be read at that point.

If any of the :option:`-map`, :option:`-items`, :option:`-tasks` or
:option:`-show` options was specified, the appropriate output is produced.
If not, only a syntax check of the input is done.

When producing output, the output format specified by the :option:`-format`
option is used.  If this was not specified, the first format in the list
which supports this type of output is chosen.

Some of the output formats use additional library files to do their work.
For example, the PostScript output format prepends a standard "prologue"
file to all output.  These files are found using the same search path as
the system initialization file (see above).

Here's a summary of the command-line options (which can be abbreviated),
starting with the output options:

.. cmdoption:: -m, --map [sections]

   Draw a map of the game.  You can optionally specify a list of the map
   sections to print.  The list is a comma-separated set of map section
   numbers (starting from 1) and can include ranges.  For example, the
   argument ``1,3-5`` would print map sections 1, 3, 4 and 5.  If the list
   isn't specified, all sections are printed.

   .. versionadded:: 5.0
      Optional list of map sections.

.. cmdoption:: -i, --items

   Print a list of items which appear in the game.

.. cmdoption:: -t, --tasks

   Print a list of tasks required to solve the game.

.. cmdoption:: -f, --format=FORMAT

   Specify the output format.

.. cmdoption:: -o, --output=FILE

   Write to the specified file, instead of stdout.

Next comes the auxiliary options:

.. cmdoption:: -I, --include=DIR

   Prepend the specified directory to the library and include file search
   path.  This option may be repeated.

.. cmdoption:: -S, --style=STYLE

   Set a global style.  See Customization_ for more details.  This option
   may be repeated.

   .. versionadded:: 5.0

.. cmdoption:: -s, --set VAR=VALUE

   Set a customization variable.  This overrides any settings in the input
   files.  This option may be repeated.

   .. versionadded:: 5.0

.. cmdoption:: --noinit

   Don't read your personal init file.

.. cmdoption:: -w, --nowarn

   Don't print warnings.

.. cmdoption:: -e, --errors=NUM

   Print this many errors before aborting (default: 10).  If set to zero,
   print all errors.

   .. versionadded:: 5.2

Finally, here are the information options:

.. cmdoption:: --show=TYPE

   Show one of several types of information, and exit.  The ``TYPE``
   argument can be one of:

   ``maps``
	   Show a list of all the map sections defined in the input.  This
	   is useful for finding the numbers of the map sections you want
	   to print.

	   .. versionadded:: 5.0

   ``path``
	   Show the directories that are searched for library and include
	   files.

   ``vars``
	   Show a complete list of defined variables, in a format suitable
	   for feeding back into IFM.  See :doc:`vars`.

.. cmdoption:: -v, --version

   Print the program version.

.. cmdoption:: -h, --help

   Just print some usage information.

Types of output
===============

.. |X| replace:: Y

IFM has three different types of output (a map, a list of items, and a list
of tasks) and several different output formats, which are described in the
following sections.  Not all types of output are produced by each output
format.  The table below shows what's available for each format.

    ====== ========== === === ==== === === ===
    Output PostScript Fig Tk  Text Rec Dot Raw
    ====== ========== === === ==== === === ===
    Map    |X|        |X| |X|
    Items                     |X|          |X|
    Tasks                     |X|  |X| |X| |X|
    ====== ========== === === ==== === === ===

All the map output formats display map sections in the same way, so that
what you get with one format looks much the same as another. [1]_

.. index::
   pair: PostScript; Types of output

.. _ps:

PostScript maps (``ps``)
------------------------

This produces a PostScript map suitable for printing.  Several map sections
may be printed per page, and the maps are printed over as many pages as it
takes.  Automatic packing is done to try to get a good fit on the
page.  Also, portrait or landscape is chosen depending on whichever gives
the best fit.  Fonts of room and item text are scaled to fit them in the
room boxes, if required.

.. index::
   pair: Fig; Types of output

.. _fig:

Fig maps (``fig``)
------------------

.. versionadded:: 5.0

This produces a map which can be read (and edited) by Xfig_, and any other
programs which understand Fig format.  The map sections are packed to get a
best fit automatically, in a similar manner to PostScript, but since Fig
has no concept of pages, it is most useful when you're printing each map
section individually.  There's a utility program called :doc:`ifm2dev
<ifm2dev>` which automatically does this.

Fig format is also useful if you want to print poster-sized maps over
several pages.  The :option:`-M` option of :program:`fig2dev` (part of the
transfig package) will automatically do this.

.. index::
   pair: Tk; Types of output

.. _tk:

Tk drawing commands (``tk``)
----------------------------

This produces map commands for input to :doc:`tkifm <tkifm>`, a simple
graphical interface to IFM.  It isn't very useful to produce this output
yourself---:program:`tkifm` does that internally to build its map pictures.
But you can control its display by setting variables in the usual way.

.. index::
   pair: ASCII; Types of output

.. _text:

ASCII text (``text``)
---------------------

This produces human-readable output for items and tasks.  The output should
be fairly self-explanatory.

.. index::
   pair: Recording; Types of output

.. _recording:
.. _rec:

Recording commands (``rec``)
----------------------------

This output produces a list of commands suitable for feeding to IF
interpreters in playback mode.  All the commands in the output are
converted to uppercase.

In order for this to work properly, you have to give commands that the game
will understand.  The cmd attribute of rooms, links, joins and tasks can
help with this.  Currently there's no item :keyword:`cmd` attribute, so you
have to make sure that the item description is recognized by the game (for
:keyword:`get` and :keyword:`drop` commands).  Also, if a task is
implicitly done in the game without you having to type any commands (e.g.,
visiting a room), you can indicate this by using :keyword:`cmd none`.

Of course, a recording will only play back properly in an interpreter if it
provides correct game commands.  Random events can't be dealt with by IFM,
and will probably cause playback to fail.  But you can work around this
with an interpreter that is able to fix the random seed at startup (e.g.,
Frotz_.  This should eliminate most (but not all) of the problems of
randomness.

.. index::
   pair: Task dependencies; Types of output

.. _dot:

Task dependencies (``dot``)
---------------------------

.. versionadded:: 5.0

This produces a graph of the dependencies of tasks on each other, in
Graphviz (dot) format.  You'll need to have Graphviz_ installed in order to
display the graph.

.. _raw:

.. index::
   pair: Raw data; Types of output

Raw data (``raw``)
------------------

This produces raw data for all output formats, intended for use by other
programs (and the IFM regression test suite).  Each entry consists of a
number of data lines, and is separated from other entries by a blank line.
Each data line consists of an attribute, a colon, and its value.  The
attributes should be self-explanatory. [2]_

.. index::
   single: style; Customization

.. index:: Colossal Cave; Customizing rooms

Customization
=============

.. highlight:: ifm

You can change the appearance of many output features according to your
taste.  You do this by setting the values of the variables that control
those features.  This section tells you how to use variables---for a
complete list of the customization variables available, see :doc:`vars`.

As a first example, the background colour of rooms is determined by the
variable :keyword:`room_colour`.  Its default value is ``white``.  It can
be changed like this::

    room_colour = "beige";

Setting a variable like this will affect all output formats.  But in some
cases you don't want to do that.  A good example is the one above---if you
don't have a colour printer, you may not want to have beige rooms printed
(they'll come out greyish).  To get around that, you can set variables that
are specific to a particular output format::

    tk.room_colour = "beige";

This says to set the variable to ``beige`` only if producing Tk output.
The default for all other formats is still ``white``.

You can also customize the appearance of individual rooms and links on the
map, by using different display styles.  A display style is just a group of
variable settings with a given name.  For example, suppose you're making a
map of *Colossal Cave* and want to mark rooms where you can refill your
water bottle.  You can define a style called, say, ``Water``, like this::

    style Water;
      room_colour = "light blue";
    endstyle;

The values of variables that are set between the :keyword:`style` and
:keyword:`endstyle` clauses only apply to things drawn in that style.  Now,
if you declare rooms like this::

    room "At End Of Road";

    room "Inside Building" style Water dir e go in;

then the room "Inside Building" will be drawn with a light blue background.
You can customize individual links in a similar manner.

An alternative way to define a variable in a particular style is to use the
:keyword:`in style` clause, like this::

    room_colour = "light blue" in style Water;

If a style only changes a single variable, this may be more convenient.

If you assign a style (say, called ``newstyle``) to an object, but don't
define it anywhere in your input, then IFM will look for a file called
:file:`newstyle.ifm` using the standard search path.  If the file exists,
it is expected to define style ``newstyle``.  For example, you could put
the ``Water`` style definition above into a file called :file:`Water.ifm`
somewhere on the IFM search path, and it would be read automatically.  This
is useful if, for example, you want to use the same style in several
different maps.

You can define global styles using the :option:`--style` command-line
option; these apply to all IFM objects.  Global styles are most useful when
setting variables that affect the overall appearance of the output, in
conjunction with the file search method described above (e.g., a file
containing general colour and font definitions).

.. index::
   single: style; Predefined styles
   pair: helvetica; Predefined styles
   pair: reckless; Predefined styles
   pair: verbose; Predefined styles
   pair: puzzle; Predefined styles
   pair: special; Predefined styles

Predefined styles
=================

IFM comes with a few predefined style files, as shown in the table below:

    ========== ====== ====================================
    Style      Scope  Description
    ========== ====== ====================================
    helvetica  global Use Helvetica fonts everywhere in maps
    reckless   global Treat all tasks as safe when solving the game
    verbose    global Print verbose solver messages
    puzzle     room   Mark room as containing a puzzle
    special    link   Mark link as being special in some way
    ========== ====== ====================================

If you create any generally useful or nice-looking styles, you might want
to send me a copy so I can include them with the next version of IFM.  The
**Scope** field indicates which type of IFM object it applies to.  Styles
that have global scope can meaningfully be used by the :option:`--style`
command-line option.

Environment variables
=====================

IFM uses the following environment variables:

.. envvar:: IFMPATH

   A colon-separated list of directories to search for IFM files.  These
   directories are searched before the standard directories.

.. envvar:: HOME

   Considered to be your home directory when looking for initialization
   files.

Diagnostics
===========

This section describes the possible error and warning messages which might
be produced by IFM, and what they mean.  Note that individual output
formats may print their own errors and/or warnings.  These lists only cover
the standard ones.

Error messages
--------------

Here's the list of error messages.  If any errors occur, no output is
produced.

``error: invalid repeat count``
       You've given a repeat count of zero or less for a string or
       direction, which doesn't make much sense.

``error: no last room``
       You've given the very first room a dir clause.

``error: no [type] referred to by 'last'``
       You've said last to refer to the last room, item or task that was
       defined, but none of that type of object have been defined yet.

``error: no [type] referred to by 'it'``
       You've said :keyword:`it` to refer to the last room, item or task
       tag that was mentioned in the current command, but no tags of that
       type of object have been mentioned.

``error: no items referred to by 'them'``
       You've said :keyword:`them` to refer to all the items mentioned in
       the current command, but no items have been mentioned.

``error: [type] tag [name] already defined``
       You've given two similar objects the same tag name.

``error: [type] tag [name] not defined``
       You've referred to a tag name that hasn't been defined anywhere in
       the input.

``error: [type] tag [name] not yet defined``
       You're referring to a tag at a point where it hasn't yet been
       defined, in a situation where it must be (e.g., the room
       :keyword:`from` clause, or a command that modifies attributes of a
       previously-defined object).

``error: can't modify [name] attribute``
       You're attempting to modify an attribute of an object which can't be
       changed once it's set (e.g., a tag name).  This is because it would
       create inconsistencies between objects.

``error: can't link [name1] and [name2] -- different map sections``
       The rooms you're trying to link are on different sections of the
       map, and have no spatial relation to one another.  You might have
       forgotten to link a previous room in the list.  Or you meant to use
       a join.

``error: can't link [name] to itself without at least one direction``
       To link a room to itself, you need to specify at least one
       direction.  Otherwise, the link goes nowhere.

``error: links between [name1] and [name2] have differing lengths``
       You've defined more than one link or join between the given rooms,
       but given them different values for the :keyword:`length`
       attribute.  This isn't allowed.

``error: more than one task needs to follow [task] immediately``
       You've given two or more tasks an identical :keyword:`follow` tag.
       Only one task can "follow" a given task.

``error: [num] cyclic task dependencies``
       The game isn't solvable because there's one or more chains of tasks
       where each must be done before the next, but the last must be done
       before the first.

``error: variable [name] is not defined``
       A customization variable needed by an output format is not
       defined.  You should only see these errors if you have modified or
       overridden the system initialization file.  The remedy is to define
       the variable somewhere.

Warning messages
----------------

Here's the list of warning messages.  If only warnings occur, then output
is still produced.

``warning: attribute [attr] ignored -- no implicit link``
       You've given a room with no :keyword:`dir` clause an attribute that
       is associated with that link (e.g., :keyword:`oneway`).  Most likely
       you're putting the attribute in the wrong place---if you want, say,
       a join to have one of these attributes, you must define it using the
       standalone :keyword:`join` command instead.

``warning: link from [name1] to [name2] outside grid``
       The destination room for a link is not in a compass direction from
       the last specified position.

``warning: rooms [name1] and [name2] overlap``
       The coordinates of the specified rooms are the same.

``warning: room [name] crossed by link line between [name] and [name]``
       A link line passes through the coordinates of a room.

``warning: room [name] has multiple [dir] links``
       More than one link connects to the specified room in a particular
       direction.

``warning: can't solve game ([num] tasks not done)``
       The game is unsolvable according to the current set of tasks.  This
       can be due to part of the map being inaccessible, or IFM stupidly
       choosing the wrong order of doing things.  Hopefully the latter
       shouldn't happen very often.

``warning: can't solve game ([num] tasks ignored)``
       The game is unsolvable because you're explicitly ignoring tasks
       and/or items, using the :keyword:`ignore` attribute.

``warning: no matching style command``
       You've used :keyword:`endstyle` without a matching
       :keyword:`style`.  You probably have too many :keyword:`endstyle`
       commands.

``warning: unexpected style: [name] (expected [name])``
       You've used :keyword:`endstyle` with an argument that doesn't match
       the argument of the corresponding :keyword:`style`.  You might have
       missed out another :keyword:`endstyle` somewhere, or have too many.

``warning: style [name] referenced but not defined``
       An object in your input uses the specified style, but it isn't
       defined anywhere and the style definition file :file:`name.ifm`
       doesn't exist in the search path (or if it does, it doesn't define
       the required style).

``warning: [syntax] is obsolete -- use [phrase] instead``
       You've used an obsolete syntax.  Consult the documentation and then
       try the suggested alternative instead.  Note that multiple uses of
       the same obsolete syntax only result in a single warning.

.. rubric:: Footnotes

.. [1] Well, that's the goal anyway.  But there are still some very minor
       differences.

.. [2] Programmer-speak for "I couldn't be bothered to document it."
