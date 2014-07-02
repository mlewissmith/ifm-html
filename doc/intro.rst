==============
 Introduction
==============

.. include:: refs.rst

About IFM
=========

IFM is a language for keeping track of your progress through an Interactive
Fiction game, and a program for producing various different sorts of output
using it.  You can record each room you visit and its relation to other
rooms, the initial locations of useful items you find, and the tasks you
need to perform in order to solve the game.

The IFM mapping commands are designed so that you can easily add to the map
as you explore the game. You type in the rooms you visit and the directions
you move in to reach other rooms, and IFM calculates the position of each
room in relation to the others.  A map can consist of several independent
sections, allowing you to divide up the map however you like.  See
:doc:`maps` for details of how to make a map.

The IFM task commands, if used, allow you to specify the order in which
game-solving tasks must be done.  The IFM program can then calculate and
print a high-level *walkthrough* of the game.  See :doc:`solving` for
details of how to do this.

A more involved example of IFM in use throughout a complete game as it's
being played is given in :doc:`example`.

Several output formats are available, including PostScript, Fig and ASCII
text---these are described in :ref:`usage`. Some of the output formats have
a set of variables which control the appearance of the output---see
:doc:`vars`.

IFM comes with **ABSOLUTELY NO WARRANTY**.  This is free software, and you
are welcome to redistribute it under certain conditions; see :doc:`license`
for details.

A bit of history
================

When I started playing IF games, years ago, I did what many other people
did---made a rough map on paper of where I'd explored, so I could see which
directions I hadn't tried yet. Inevitably, the maps always fell off the
page and turned into a complete mess.

My first attempt at solving this problem was to make use of the tools I had
available to draw the map for me. So I started typing the map network into
a file, and wrote a small Perl script to convert the data into something
that groff_ could read and draw a map from. I also added a way of recording
which items I'd found, and whether I'd found a use for them yet.

As it stood, this worked quite well---it produced nice maps and lists of
items so I could see where I'd got to. The only problem was that it was
just as tedious making maps this way as on paper, since I had to work out
the offset of each room relative to a fixed origin.

Eventually I decided that enough was enough. I wanted to be able to add
stuff to the map as I played the game, without having to worry about
offsets or anything irrelevant like that. Hence the main design criteria
for IFM. The other criteria were easy---an input language, for flexibility
and extendability, and a program for converting files in this language into
different types of output.

Partway through writing the initial version of IFM, I had the idea that it
would be nice to be able to somehow record not only the map and items found
so far, but what's been done to solve the game. The concept of tasks
appeared, which immediately tripled the size of the program and caused all
sorts of headaches.

But eventually it got to the stage where it might be worth releasing, so in
June 1998 the first version appeared in the `IF archive`_.  And now,
finally, it's finished and I can go back to that IF game I was
playing. [1]_

Getting IFM
===========

You can find IFM in the `mapping tools`_ directory of the `IF archive`_.
See the file :file:`INSTALL` that comes with the IFM source distribution
for installation details.  But it basically boils down to::

    ./configure
    make
    su -c 'make install'

If you want to build IFM on Windows, you'll need to install Cygwin_ first.
Then, use the same commands as above.

.. rubric:: Footnotes

.. [1] Oh all right. Maybe *one* more feature...
