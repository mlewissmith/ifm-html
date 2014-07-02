===============================================================
 :program:`ifm2dev`: convert IFM maps to various other formats
===============================================================

.. program:: ifm2dev

.. include:: refs.rst

.. versionadded:: 5.0

:program:`ifm2dev` is a front end to :program:`fig2dev`, which converts
diagrams in Xfig_ format to various other formats.  :program:`ifm2dev`
converts each map section and writes them to separate files. The command
format is::

    ifm2dev [-o template] [fig2dev-options] [-- ifm-options] file

The :keyword:`-o` option sets the file template for the output files. It
must contain an integer format indicator (e.g., ``%d``); this is replaced
with the map section number in filenames. If not set, it defaults to
:file:`prefix-%02d.suffix`, where :file:`prefix` is the file prefix of the
input file (with :file:`.ifm` removed), and suffix is the suffix
appropriate for the type of output.

All :program:`fig2dev` output options are passed through without
change. You can use the :keyword:`-L` option of :program:`fig2dev` to set
the output format. See the :program:`fig2dev` manpage for details.

You can supply options to IFM by first giving the end-of-options indicator
(``--``) and then the options.
