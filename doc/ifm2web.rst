====================================================
 :program:`ifm2web` -- convert IFM map to Web image
====================================================

.. program:: ifm2web

.. versionadded:: 5.3

:program:`ifm2web` converts an IFM map to an image file suitable for
display on the Interweb (i.e., with transparency).  It can produce two
types of image:

* A map showing one or more map sections, by sending IFM output through
  :program:`fig2dev` and :program:`convert`.

* A task graph, sending IFM output through :program:`dot` (or
  :program:`neato`) and :program:`convert`.

These are the options for printing maps:

.. cmdoption:: -m sections

   Only process the map sections specified.  Format is the same as the IFM
   ``-m`` option (see :ref:`usage`).

.. cmdoption:: -z zoom

   Set the :program:`fig2dev` magnification (zoom) factor.  Default: 1.

.. cmdoption:: -t

   Include map section titles.

These are the options for printing task graphs:

.. cmdoption:: -g

   Write task graph instead of a map.

.. cmdoption:: -r

   Group the tasks by rooms.

.. cmdoption:: -n

   Use :program:`neato` instead of :program:`dot`.

Other options:

.. cmdoption:: -S style

   Use the specified IFM style.

.. cmdoption:: -s scale

   Set the :program:`convert` scale factor, as a percentage.  Default: 100.

.. cmdoption:: -o file

   Write to the specified file.  If not given, the filename is built from
   the input file prefix and the image format.

.. cmdoption:: -w

   Don't actually run anything; just print what would be done.

.. cmdoption:: -h

   Print a short usage message and exit.
