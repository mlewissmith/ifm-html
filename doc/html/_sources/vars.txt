==================
 Output variables
==================

There are many variables available for customizing output, described below.
Their initial values are set in the IFM initialization file
:file:`ifm-init.ifm`.  You can change this file to set global defaults for
everybody, or alternatively set your own preferences in your personal init
file.

The default values are designed to give nice results in all formats.  The
default for PostScript is to produce maps for monochrome printing on one or
more pages of A4 paper.  The default for Fig is to produce one big page
(which can be split into multiple pages later).

The map output formats differ in their treatment of fonts.  In PostScript
and Fig, the font and font size are specified separately, via the
``*_font`` and ``*_fontsize`` variables.  In Tk, they are both specified
together, via the ``*_fontdef`` variables.

In PostScript and Fig output, a font size represents the maximum desired
size---the actual size may be scaled down in order to fit the text into an
enclosing space (e.g., a room box).

In Tk output, all line width values are interpreted as integers.

General variables
=================

.. list-table::
   :widths: 1 1 3 10 1
   :header-rows: 1
   
   * - Name
     - Type
     - Default
     - Description
     - Outputs

   * - ``colour_file``
     - string
     - ``ifm-rgb.txt``
     - File of colour definitions, which contains the RGB values of each
       colour referred to below (and a good many more).  It's just the
       ``rgb.txt`` file found on Unix/X11 systems.  See that file for a
       list of available colours.  You can use the same colour names for
       the Tk output, since it uses the standard X colours.
     - :ref:`ps <ps>` :ref:`fig <fig>` :ref:`tk <tk>`

   * - ``prolog_file``
     - string
     - ``ifm-pro.ps``
     - Prolog file that gets prepended to all the PostScript output.  This
       defines all the procedures for drawing everything.
     - :ref:`ps <ps>`

   * - ``font_scale``
     - float
     - 1
     - Scale factor which is applied to all fonts.  This is a convenience
       variable to make font adjustments easier.
     - :ref:`ps <ps>` :ref:`fig <fig>`

Page variables
==============

.. list-table::
   :widths: 1 1 3 10 1
   :header-rows: 1
   
   * - Name
     - Type
     - Default
     - Description
     - Outputs

   * - ``page_size``
     - string
     - ``A4``
     - Default page size.  Available page sizes are: ``A3``, ``A4``, ``A``,
       ``B``, ``C``, ``Legal``, ``Letter``.
     - :ref:`ps <ps>` :ref:`fig <fig>`

   * - ``page_width``
     - float
     - undef
     - If both ``page_width`` and ``page_height`` are defined, these set a
       custom page size which overrides the ``page_size`` variable.  Units
       are in cm.
     - :ref:`ps <ps>` :ref:`fig <fig>`

   * - ``page_height``
     - float
     - undef
     - If both ``page_width`` and ``page_height`` are defined, these set a
       custom page size which overrides the ``page_size`` variable.  Units
       are in cm.
     - :ref:`ps <ps>` :ref:`fig <fig>`

   * - ``page_margin``
     - float
     - 2
     - Margin space to be left on each page, in cm.
     - :ref:`ps <ps>` :ref:`fig <fig>`

   * - ``page_rotate``
     - bool
     - undef
     - Whether to rotate each page to landscape.  If not defined, then
       rotation is decided on a per-page basis in order to get the best
       fit.
     - :ref:`ps <ps>`

   * - ``show_page_title``
     - bool
     - true
     - Whether to show the main title on each page.
     - :ref:`ps <ps>`

   * - ``page_title_colour``
     - string
     - ``black``
     - Page title colour.
     - :ref:`ps <ps>`

   * - ``page_title_font``
     - string
     - ``Times-Bold``
     - Page title font.
     - :ref:`ps <ps>`

   * - ``page_title_fontsize``
     - int
     - 18
     - Page title fontsize.
     - :ref:`ps <ps>`

   * - ``show_page_border``
     - bool
     - false
     - Whether to show a border around each page.
     - :ref:`ps <ps>` :ref:`fig <fig>`

   * - ``page_border_colour``
     - string
     - ``black``
     - Colour of the page border (if drawn).
     - :ref:`ps <ps>` :ref:`fig <fig>`

   * - ``page_background_colour``
     - string
     - ``white``
     - Colour of the page background (if border is drawn).
     - :ref:`ps <ps>` :ref:`fig <fig>`

   * - ``fit_page``
     - bool
     - false
     - Whether to scale Fig layout to fit on a single page.  Most of the
       time, this doesn't make things look good.
     - :ref:`fig <fig>`

Map section variables
=====================

.. list-table::
   :widths: 1 1 3 10 1
   :header-rows: 1
   
   * - Name
     - Type
     - Default
     - Description
     - Outputs

   * - ``show_map_title``
     - bool
     - true
     - Whether to show the map title.
     - :ref:`ps <ps>` :ref:`fig <fig>`

   * - ``map_title_colour``
     - string
     - ``black``
     - Colour of the title printed above each map section.
     - :ref:`ps <ps>` :ref:`fig <fig>`

   * - ``map_title_font``
     - string
     - ``Times-Bold``
     - Font of the title printed above each map section.
     - :ref:`ps <ps>` :ref:`fig <fig>`

   * - ``map_title_fontsize``
     - int
     - 14
     - Font size of the title printed above each map section.
     - :ref:`ps <ps>` :ref:`fig <fig>`

   * - ``show_map_border``
     - bool
     - false
     - Whether to show a border around each map section.
     - :ref:`ps <ps>` :ref:`fig <fig>`

   * - ``map_border_colour``
     - string
     - ``black``
     - Colour of the map border (if drawn).
     - :ref:`ps <ps>` :ref:`fig <fig>`

   * - ``map_background_colour``
     - string
     - ``white`` (ps, fig), ``wheat`` (tk)
     - Colour of the map background (if border is drawn).
     - :ref:`ps <ps>` :ref:`fig <fig>` :ref:`tk <tk>`

   * - ``map_section_spacing``
     - int
     - 1
     - Minimum space, in rooms, between map sections when packed together.
     - :ref:`ps <ps>` :ref:`fig <fig>`

   * - ``map_canvas_width``
     - int
     - 8
     - Maximum width of the Tk map canvas window, in rooms.  Sizes bigger
       than this will cause scrollbars to appear.
     - :ref:`tk <tk>`

   * - ``map_canvas_height``
     - int
     - 6
     - Maximum height of the Tk map canvas window, in rooms.  Sizes bigger
       than this will cause scrollbars to appear.
     - :ref:`tk <tk>`

Room variables
==============

.. list-table::
   :widths: 1 1 3 10 1
   :header-rows: 1
   
   * - Name
     - Type
     - Default
     - Description
     - Outputs

   * - ``room_size``
     - float
     - 3
     - Space allocated for each room, in cm.  In PostScript and Fig, this
       is the maximum size -- the actual size may be reduced in order to
       fit things on the page.
     - :ref:`ps <ps>` :ref:`fig <fig>` :ref:`tk <tk>`

   * - ``room_width``
     - float
     - 0.8
     - Proportion of the room space that's taken up by the room width.
       Should be less than 1 or you'll have no space left for link lines.
     - :ref:`ps <ps>` :ref:`fig <fig>` :ref:`tk <tk>`

   * - ``room_height``
     - float
     - 0.65
     - Proportion of the room space that's taken up by the room height.
       Should be less than 1 or you'll have no space left for link lines.
     - :ref:`ps <ps>` :ref:`fig <fig>` :ref:`tk <tk>`

   * - ``room_colour``
     - string
     - ``white``
     - Default background colour of rooms.
     - :ref:`ps <ps>` :ref:`fig <fig>` :ref:`tk <tk>`

   * - ``room_shadow_xoff``
     - float
     - 0.05
     - X offset of room 'shadows', as a proportion of allocated room space.
       This is a pseudo-3D effect which makes rooms look raised off the
       page.  Note that you can change the direction of the room shadow by
       negating one or both of the offsets.
     - :ref:`ps <ps>` :ref:`fig <fig>` :ref:`tk <tk>`

   * - ``room_shadow_yoff``
     - float
     - 0.05
     - Y offset of room 'shadows', as a proportion of allocated room space.
       This is a pseudo-3D effect which makes rooms look raised off the
       page.  Note that you can change the direction of the room shadow by
       negating one or both of the offsets.
     - :ref:`ps <ps>` :ref:`fig <fig>` :ref:`tk <tk>`

   * - ``room_shadow_colour``
     - string
     - ``grey50``
     - Colour of room 'shadows'.
     - :ref:`ps <ps>` :ref:`fig <fig>` :ref:`tk <tk>`

   * - ``room_border_width``
     - float
     - 1
     - Width of the room box lines.
     - :ref:`ps <ps>` :ref:`fig <fig>` :ref:`tk <tk>`

   * - ``room_border_colour``
     - string
     - ``black``
     - Colour of the room box lines.
     - :ref:`ps <ps>` :ref:`fig <fig>` :ref:`tk <tk>`

   * - ``room_border_dashed``
     - bool
     - false
     - Whether to draw dashed room borders.
     - :ref:`ps <ps>` :ref:`fig <fig>`

   * - ``room_exit_width``
     - float
     - 1
     - Width of the room exit lines.
     - :ref:`ps <ps>` :ref:`fig <fig>` :ref:`tk <tk>`

   * - ``room_exit_colour``
     - string
     - ``black``
     - Colour of the room exit lines.
     - :ref:`ps <ps>` :ref:`fig <fig>` :ref:`tk <tk>`

   * - ``room_text_colour``
     - string
     - ``black``
     - Colour of room description text.
     - :ref:`ps <ps>` :ref:`fig <fig>` :ref:`tk <tk>`

   * - ``room_text_font``
     - string
     - ``Times-Bold``
     - Font of room description text.
     - :ref:`ps <ps>` :ref:`fig <fig>`

   * - ``room_text_fontsize``
     - int
     - 10
     - Font size of room description text.
     - :ref:`ps <ps>` :ref:`fig <fig>`

   * - ``room_text_fontdef``
     - string
     - ``Times 10 bold``
     - Font and fontsize of room description text.
     - :ref:`tk <tk>`

   * - ``show_items``
     - bool
     - true
     - Whether to show non-hidden item descriptions in rooms.
     - :ref:`ps <ps>` :ref:`fig <fig>` :ref:`tk <tk>`

   * - ``item_text_colour``
     - string
     - ``black``
     - Colour of item description text (if shown).
     - :ref:`ps <ps>` :ref:`fig <fig>` :ref:`tk <tk>`

   * - ``item_text_font``
     - string
     - ``Times-Italic``
     - Font of item description text (if shown).
     - :ref:`ps <ps>` :ref:`fig <fig>`

   * - ``item_text_fontsize``
     - int
     - 6
     - Font size of item description text (if shown).
     - :ref:`ps <ps>` :ref:`fig <fig>`

   * - ``item_text_fontdef``
     - string
     - ``Times 8 italic``
     - Font and fontsize of item description text (if shown).
     - :ref:`tk <tk>`

   * - ``show_tags``
     - bool
     - false
     - Whether to append room tag names to room descriptions.  If so, they
       are appended in square brackets, like [this].
     - :ref:`ps <ps>` :ref:`fig <fig>` :ref:`tk <tk>`

Link style variables
====================

.. list-table::
   :widths: 1 1 3 10 1
   :header-rows: 1
   
   * - Name
     - Type
     - Default
     - Description
     - Outputs

   * - ``link_line_width``
     - float
     - 1
     - Width of link lines.
     - :ref:`ps <ps>` :ref:`fig <fig>` :ref:`tk <tk>`

   * - ``link_colour``
     - string
     - ``black``
     - Colour of link lines.
     - :ref:`ps <ps>` :ref:`fig <fig>` :ref:`tk <tk>`

   * - ``link_arrow_size``
     - float
     - 0.1
     - Size of oneway link arrows, as a proportion of the allocated room
       space.
     - :ref:`ps <ps>` :ref:`tk <tk>`

   * - ``link_spline``
     - bool
     - true
     - Whether to draw link lines as splines.
     - :ref:`fig <fig>` :ref:`tk <tk>`

   * - ``link_dashed``
     - bool
     - false
     - Whether to draw dashed link lines.
     - :ref:`ps <ps>` :ref:`fig <fig>`

   * - ``link_text_font``
     - string
     - ``Times-Roman``
     - Font of text that's associated with link lines.
     - :ref:`ps <ps>` :ref:`fig <fig>`

   * - ``link_text_fontsize``
     - int
     - 6
     - Font size of text that's associated with link lines.
     - :ref:`ps <ps>` :ref:`fig <fig>`

   * - ``link_text_colour``
     - string
     - ``black`` (ps, fig), ``red`` (tk)
     - Colour of text that's associated with link lines.
     - :ref:`ps <ps>` :ref:`fig <fig>` :ref:`tk <tk>`

   * - ``link_text_fontdef``
     - string
     - ``Times 8 bold``
     - Font and font size size of text that's associated with link lines.
     - :ref:`tk <tk>`

   * - ``link_updown_string``
     - string
     - ``U/D``
     - Text strings indicating up/down on links.  PostScript is currently a
       special case: the strings either side of the ``/`` are extracted and
       printed at either ends of the link, near the room they come from.
     - :ref:`ps <ps>` :ref:`fig <fig>` :ref:`tk <tk>`

   * - ``link_inout_string``
     - string
     - ``I/O``
     - Text strings indicating in/out on links.  PostScript is currently a
       special case: the strings either side of the ``/`` are extracted and
       printed at either ends of the link, near the room they come from.
     - :ref:`ps <ps>` :ref:`fig <fig>` :ref:`tk <tk>`

Join style variables
====================

.. list-table::
   :widths: 1 1 3 10 1
   :header-rows: 1
   
   * - Name
     - Type
     - Default
     - Description
     - Outputs

   * - ``show_joins``
     - bool
     - true
     - Whether to indicate joins in the room text.
     - :ref:`ps <ps>` :ref:`fig <fig>` :ref:`tk <tk>`

   * - ``join_format``
     - string
     - ``number``
     - Join string format (gets put in parentheses in those rooms that join
       to other rooms).  The value should be ``number`` or ``letter``.
     - :ref:`ps <ps>` :ref:`fig <fig>` :ref:`tk <tk>`

Game solver variables
=====================

.. list-table::
   :widths: 1 1 3 10 1
   :header-rows: 1
   
   * - Name
     - Type
     - Default
     - Description
     - Outputs

   * - ``keep_unused_items``
     - bool
     - true
     - Whether to keep unused items (i.e., those which were obtained via
       some task or other, but currently have no use).
     - :ref:`text <text>` :ref:`rec <rec>`

   * - ``all_tasks_safe``
     - bool
     - false
     - Whether to treat all tasks as safe (reckless mode!).
     - :ref:`text <text>` :ref:`rec <rec>`

   * - ``solver_messages``
     - bool
     - false
     - Whether to print game solver info messages (helps with figuring out
       what it's up to).
     - :ref:`text <text>` :ref:`rec <rec>`

   * - ``finish_room``
     - string
     - 
     - Comma-separated list of extra room tags which are assigned the
       :keyword:`finish` attribute.

       .. versionadded:: 5.3

     - :ref:`text <text>` :ref:`rec <rec>`

   * - ``finish_item``
     - string
     - 
     - Comma-separated list of extra item tags which are assigned the
       :keyword:`finish` attribute.

       .. versionadded:: 5.3

     - :ref:`text <text>` :ref:`rec <rec>`

   * - ``finish_task``
     - string
     - 
     - Comma-separated list of extra task tags which are assigned the
       :keyword:`finish` attribute.

       .. versionadded:: 5.3

     - :ref:`text <text>` :ref:`rec <rec>`

Task dependency variables
=========================

.. list-table::
   :widths: 1 1 3 10 1
   :header-rows: 1
   
   * - Name
     - Type
     - Default
     - Description
     - Outputs

   * - ``task_graph_rooms``
     - bool
     - false
     - Whether to group tasks by the room they're done in.  This can either
       enhance the task structure or make it look a complete mess.
     - :ref:`dot <dot>`

   * - ``task_graph_orphans``
     - bool
     - false
     - Whether to show orphan tasks (those with no previous/next
       dependencies).  Useful for completeness, but it clutters things up a
       lot.
     - :ref:`dot <dot>`

   * - ``task_graph_attr``
     - string
     - 
     - Graph attributes, in Graphviz format.
     - :ref:`dot <dot>`

   * - ``task_graph_node``
     - string
     - ``shape=box``
     - Node attributes, in Graphviz format.
     - :ref:`dot <dot>`

   * - ``task_graph_link``
     - string
     - 
     - Link attributes, in Graphviz format.
     - :ref:`dot <dot>`

   * - ``task_graph_wrap``
     - int
     - 12
     - Word wrap length of nodes, in characters.

       .. versionadded:: 5.3

     - :ref:`dot <dot>`

   * - ``task_graph_font``
     - string
     - ``Times-Roman``
     - Font name.

       .. versionadded:: 5.3

     - :ref:`dot <dot>`
