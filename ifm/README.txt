IFM -- Interactive Fiction Mapper
*********************************

IFM is a language and a program for keeping track of your progress through
an Interactive Fiction game.  You can record each room you visit and its
relation to other rooms, the initial locations of useful items you find,
and the tasks you need to perform in order to solve the game.

The IFM mapping commands are designed so that you can easily add to the map
as you explore the game.  You type in the rooms you visit and the
directions you move in to reach other rooms, and IFM calculates the
position of each room relative to the others.  A map can consist of several
independent sections, allowing you to divide up the map however you like.

The IFM task commands, if used, allow you to specify the order in which
game-solving tasks must be done.  The IFM program can then calculate and
print different styles of walkthrough for the game.

IFM can write hardcopy maps directly to PostScript, suitable for printing
or viewing.  It can also write maps in Fig format, which can be viewed and
edited using Xfig, or converted into many other formats.

Building IFM
************

To build IFM from source, either on Linux or Windows under Cygwin, you'll
need the Vars library (also written by me) to be installed.  You can get it
at:

    http://www.freewebtown.com/zondo/programs

After that, see the file INSTALL for installation details.  But it
basically boils down to:

    ./configure
    make
    make install

Existing IFM Maps
*****************

Dave Chapeskie (dchapes@ddm.wox.org) has set up a web page of IFM maps, in
source and PostScript format, at

      http://www.sentex.net/~dchapes/ifm

At the moment it contains maps of a few Infocom and Inform games.  Some are
complete, others not quite.  A few have tasks set up so that a walkthrough
can be made.

If you produce any maps worthy of distribution, or more complete versions
of those already there, he will be glad to add them to his page.

In addition, the 'demos' subdirectory contains some demonstration maps that
don't give away secrets from real games.

Extra IFM Programs
******************

There are a few utility programs bundled with IFM.  To make the most use of
them, you'll need a few other things installed on your system: Tcl/Tk,
perl, xfig, transfig and TeX.

ifm2dev
=======

ifm2dev is a perl script which is a front end to fig2dev, which converts
diagrams in Fig format to various other formats.  ifm2dev converts each map
section and writes them to separate files.

ifm2tex
=======

ifm2tex is another perl script which uses ifm2dev to dump EPS maps and
include them in a LaTeX document, together with tables of items and tasks.

tkifm
=====

tkifm is a simple graphical interface to IFM.  It provides you with a
window allowing you to edit IFM input (or just view it, if you want to use
your own editor), and menus to view the maps you've generated so far or
export them to PostScript or Fig format.

scr2ifm
=======

scr2ifm is a perl script that takes one or more Infocom-style transcripts
of an IF game and creates a map in IFM format.

scr2ifm is designed for people who want maps of their games but don't want
the hassle of making one themselves (how lazy can you get?).  As long as
you bear in mind a few simple things while playing, the map-making process
is completely automatic.  With a tiny amount of effort, you can also add
other nice IFM features (items and tasks) too.

scr2ifm was inspired by Frobot, another transcript-reading mapper.  But
scr2ifm in combination with IFM does a much better job.

Feedback on how well it does (or not) would be appreciated.  scr2ifm has
been tested on several different types of transcript (Infocom, Inform,
TADS, Magnetic Scrolls), but is probably still far from bullet-proof.

Related Programs
****************

IFMapper is a graphical mapping tool written by Gonzalo Garramuno
(ggarra@advance.dsl.com.ar), and has the ability to read and write IFM
maps.  You can find IFMapper at

     http://www.rubyforge.org/projects/ifmapper

Finally...
**********

IFM and its utility programs are free software, and you may redistribute
them under certain conditions; see the file called COPYING for details.

Feel free to send comments, bug reports, suggestions, etc. to me.  I am
Glenn Hutchings (zondo42@googlemail.com).

Happy mapping!
