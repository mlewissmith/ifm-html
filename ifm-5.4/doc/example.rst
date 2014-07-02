=============================
 Mapping and solving example
=============================

.. include:: refs.rst

.. highlight:: ifm

Here's a more detailed example of how to use IFM, using the example game
*Ruins* to produce a full IFM map containing items and game-solving tasks.

*Ruins* originally existed only in snippets throughout Graham Nelson's
*Inform Designer's Manual* until Roger Firth produced a playable version of
it.  It serves as a good demo of IFM capabilities, since it doesn't give
away any secrets from a "real" game.  The source code for *Ruins* can be
found bundled with the `Inform 6 sources`_.

Mapping the game
================

OK, let's go.  First, let's set the title for the map, which gets printed
out (for example) at the top of each PostScript output page::

    title "Ruins";

On starting the game, we see this:

.. code-block:: transcript

    Days of searching, days of thirsty hacking through the briars of the
    forest, but at last your patience was rewarded. A discovery!

    RUINS
    An Interactive Worked Example
    Copyright (c) 1995 by Graham Nelson.
    Revisited 1998 by Roger Firth.
    Release 2 / Serial number 981117 / Inform v6.31 Library 6/11 S

    Dark Forest
    In this tiny clearing, the pine-needle carpet is broken by stone-cut steps
    leading down into the darkness. Dark olive trees crowd in on all sides, the
    air steams with warm recent rain, midges hang in the air.

    A speckled mushroom grows out of the sodden earth, on a long stalk.

Ooh, that mushroom looks interesting.  Let's map out what we have so far::

    room "Dark Forest" tag Dark_Forest;
    item "speckled mushroom" tag mushroom;

Let's have a closer look at that mushroom:

.. code-block:: transcript

    >GET SPECKLED MUSHROOM
    You stoop to pick the mushroom, neatly cleaving its thin stalk. As you
    straighten up, you notice what looks like a snake in the grass.

    You hear the distant drone of an aeroplane.

    >EXAMINE SNAKE
    A menacing, almost cartoon-like statuette of a pygmy spirit with an
    enormous snake around its neck.

    The plane is much louder, and seems to be flying very low. Terrified birds
    rise in screeching flocks.

    >GET STATUETTE
    Taken.

    You involuntarily crouch as the plane flies directly overhead, seemingly
    only inches above the treetops. With a loud thump, something drops heavily
    to the ground.

    >LOOK

    Dark Forest
    In this tiny clearing, the pine-needle carpet is broken by stone-cut steps
    leading down into the darkness. Dark olive trees crowd in on all sides, the
    air steams with warm recent rain, midges hang in the air.

    Your packing case rests here, ready for you to hold any important cultural
    finds you might make, for shipping back to civilisation.

    The roar dies away; calm returns to the forest.

We've found a statuette that was hidden until we picked the mushroom::

    item "statuette" tag statue hidden need mushroom;

And a packing crate just fell out of the sky!  Looks like that was going to
happen regardless, so nothing to do, map-wise.  Let's continue exploring:

.. code-block:: transcript

    >D

    Square Chamber
    A sunken, gloomy stone chamber, ten yards across, whose floor is covered
    with fine sandy dust. A shaft of sunlight cuts in from the steps above,
    giving the chamber a diffuse light, but in the shadows low lintelled
    doorways to east and south lead into the deeper darkness of the Temple.

    Carved inscriptions crowd the walls and ceiling.

We went down to get here, and have to decide on a compass direction to
represent this on the map; let's choose south.  This might clash with other
rooms later, but we can always come back and change it if needed.

We can also see some exits to other rooms, and can indicate those too, for
later reference::

    room "Square Chamber" tag Chamber dir s go down exit s e;
    item "carvings";

OK, let's have a bit of a look around.

.. code-block:: transcript

    >EXAMINE INSCRIPTIONS
    The carvings cut deep into the rocks from which the chamber is built,
    forming swirling sinuous patterns.

    >EXAMINE MUSHROOM
    The mushroom is capped with blotches, and you aren't at all sure it's not a
    toadstool.

    >EXAMINE SUNLIGHT
    The shaft of sunlight glimmers motes of dust in the air, making it seem
    almost solid.

Hmm... mushroom, sunlight.  Sunlight, mushroom.  Hey, let's do it!

.. code-block:: transcript

    >PUT MUSHROOM IN SUNLIGHT
    You drop the mushroom on the floor, in the glare of the shaft of sunlight.
    It bubbles obscenely, distends and then bursts into a hundred tiny insects
    which run for the darkness in every direction, leaving strange trails in
    the sandy floor. No trace of fungus remain.

Well, that was unexpected.  Now what?

.. code-block:: transcript

    >EXAMINE TRAILS
    The insect trails seem to resemble one particular group of carvings on the
    wall.

    >EXAMINE CARVINGS
    You stare at a group of carvings in the middle of one wall, uncannily
    resembling the insects' trail left in the sand, and feel your hand reaching
    out in that direction.

    >PRESS CARVINGS
    The carvings slide inwards, revealing a small cavity.

    >SEARCH CAVITY
    In the cavity among the carvings is a huge iron key.

    >GET IRON KEY
    Taken.

Aha!  A key.  I predict there'll be at least one door in this game.  But I
hope we didn't need that mushroom any more...

Recording what we just did::

    task "Put mushroom in sunlight" need mushroom lose it;
    task "Press carvings" after last;
    item "iron key" tag key hidden after last;

Let's explore a bit more:

.. code-block:: transcript

    >S

    Stooped Corridor
    A low, square-cut corridor, running north to south, stooping over you.
    Patches of moss emit a faint green fluorescent glow, just enough that you
    can see your surroundings.
    The passage is barred by a massive door of yellow stone.

    >EXAMINE MOSS
    The moss grows in rough clumps, surprisingly thick in places. Is that a
    hint of red hidden among all the greenery?

    >SEARCH MOSS
    Something rolls from the moss onto the ground.

    >LOOK

    Stooped Corridor
    A low, square-cut corridor, running north to south, stooping over you.
    Patches of moss emit a faint green fluorescent glow, just enough that you
    can see your surroundings.

    The passage is barred by a massive door of yellow stone.

    You can also see a blood-red ruby here.

    >GET BLOOD-RED RUBY
    Taken.

Haha, more not-very-well-hidden goodies.  And look---a door!  What a
surprise.  Let's do some mapping::

    room "Stooped Corridor" dir s;
    item "moss";
    task "Search moss";
    item "blood-red ruby" tag ruby hidden after last;

OK, let's use that key!

.. code-block:: transcript

    >UNLOCK DOOR WITH KEY
    You unlock the stone door.

    >OPEN DOOR
    You open the stone door.

    >S

    Lofty Shrine
    For an underground chamber, the Shrine is remarkably high, though its upper
    half is hazy and difficult to see clearly. The craggy walls are polished
    natural rock, in which tiny flecks of quartz catch the light of the
    flickering candle.

    The massive yellow stone door is open.

    A great stone slab of a table dominates the Shrine.

    On the slab altar is a flickering candle.

    Behind the slab, a mummified priest stands waiting, barely alive at best,
    impossibly venerable.

How atmospheric.  Continuing with mapping::

    task "Unlock door with key" need key;
    task "Open door" after last;

    room "Lofty Shrine" dir s after last;
    item "stone slab";
    item "mummified priest";

Let's see what we have here:

.. code-block:: transcript

    >EXAMINE SLAB
    It's big enough to lie on, though in the circumstances that might prove to
    be a A Really Bad Idea.

    >EXAMINE PRIEST
    He is dessicated and hangs together only by will-power. Though his first
    language is presumably local Mayan, you have the curious instinct that he
    will understand your speech.

Hmm... let's press him for information, then:

.. code-block:: transcript

    >ASK PRIEST ABOUT RUBY
    "That is one of my people's sacred relics."

    >ASK PRIEST ABOUT SLAB
    "The King (life! propserity! happiness!) is buried deep under this Shrine,
    where you will never go."

    >ASK PRIEST ABOUT KEY
    "That has been lost for generations. My gratitude to anyone who returns it
    will be overwhelming."

OK, er, let's give it back:

.. code-block:: transcript

    >PUT KEY ON SLAB
    His eyes dimly agleam with gratitude, the priest takes the key and conceals
    it among his rags. Then, lifting the candle, he carefully detaches a large
    lump of the mottled wax, and gives it to you.

    >EXAMINE WAX
    On closer examination, the lump appears to be an old honeycomb.

Looks like it might be worth something.  Time for a bit of mapping::

    task "Put key on slab" need key lose it give wax;
    item "wax" tag wax hidden;

Right, let's go back and try another direction we haven't been yet:

.. code-block:: transcript

    >N

    Stooped Corridor

    The massive yellow stone door is open.

    >N

    Square Chamber

    Carved inscriptions crowd the walls and ceiling.

    Strange trails swirl on the sandy floor.

    >E
    As you move into the eastern shadows, you seem to glimpse the word SENE
    scratched on the lintel, but before you can stop to inspect it more
    closely, you find yourself in...

    Darkness
    The darkness of ages presses in on you, and you feel claustrophobic.

    Somewhere, tiny claws are scuttling.

Crikey.  Don't like the sound of that.

.. code-block:: transcript

    >E

    Darkness

    Somewhere, tiny claws are scuttling.

    The scuttling draws a little nearer, and your breathing grows load and
    hoarse.

    >S

    Darkness
    Somewhere, tiny claws are scuttling.

    The perspiration of terror runs off your brow. The creatures are almost
    here!

OK, this is bad.  Think before acting.  What did the word *SENE* mean?
Hmm, maybe it's *two* words...

.. code-block:: transcript

    >SE

    Darkness

    Somewhere, tiny claws are scuttling.

    >NE
    Sensing a slight draught you move in that direction, stumbling over
    something lying on the ground in the dark. Almost inadvertently you grab it
    before gratefully emerging into the gloomy chamber.

    Square Chamber

    Carved inscriptions crowd the walls and ceiling.

    Strange trails swirl on the sandy floor.

Now that was close.  But what have we grabbed?

.. code-block:: transcript

    >I
    You are carrying:
      a silver bangle
      a lump of wax
      a blood-red ruby
      a pygmy statuette

Ooh, a silver bangle.  Plus, we didn't die!

Let's map what we just did.  We moved east from the Square Chamber into the
Web of Darkness.  There are essentially two of those rooms, corresponding
to the two right directions to move in.  Let's give the Web rooms a
``Dark`` style to show they're dark (we can define what ``Dark`` means,
style-wise, later)::

    room "Web of Darkness" style Dark dir e from Chamber oneway;
    item "scuttling claws";

    room "Web of Darkness" style Dark dir se oneway;
    item "scuttling claws";

The act of going northeast gave us a bangle (we tripped over it and picked
it up).  There are two ways of doing this: with an explicit movement task
that puts you back in the Square Chamber, or a "dummy" task that hands you
the bangle in this room without you doing anything (using ``cmd none``).
Let's use the first method::

    item "silver bangle" hidden tag bangle;
    task "Stumble over bangle" give bangle cmd "NE" goto Chamber;

This second room is a one-way trip back to the Square Chamber.  As it
stands, that won't be apparent on the map.  Let's add the connection in the
most direct way::

    link last to Chamber dir ne n w 2 oneway;

Well, there doesn't seem to be anywhere else to go.  Let's get all this
stuff stashed!

.. code-block:: transcript

    >U

    Dark Forest

    Your packing case rests here, ready for you to hold any important cultural
    finds you might make, for shipping back to civilisation.

    >PUT STATUETTE IN CASE
    Safely packed away.

    [Your score has just gone up by ten points.]

    >PUT BANGLE IN CASE
    Safely packed away.
    [Your score has just gone up by twenty points.]

    >PUT RUBY IN CASE
    Safely packed away.

    [Your score has just gone up by thirty points.]

    >PUT WAX IN CASE
    Safely packed away.

    The case is full, and you're feeling really homesick. Now, how to get out
    of here?

    A deep throbbing noise can be heard.

    [Your score has just gone up by forty points.]

OK, we've stashed all the loot we found.  But what's that noise?

.. code-block:: transcript

    >WAIT
    Time passes.

    A helicopter appears, hovering overhead.

    >WAIT
    Time passes.

    The helicopter lowers a rope; you tie it around the packing case, which is
    hauled up into the air, then a rope ladder comes dangling down. With one
    last look around the clearing, you climb the ladder into the waiting craft.
    Back to civilisation, and a decent pint of Brakspear's!

	*** You have won ***

    In that game you scored 100 out of a possible 100, in 44 turns, earning you
    the rank of Master Archaeologist.

Huzzah!  We found all the "cultural finds" and just need to get them to the
museum. [1]_

Let's add some tasks that record storing all the treasure.  This happens in
the starting room, so we'll just go back and put them just after the
declaration for that room.  We could just have a single task that requires
all the treasure together and gets the full 100 points, but let's spell out
the points for each one::

    task "Put statuette in case" tag pack_statue need statue lose it score 10;
    task "Put bangle in case" tag pack_bangle need bangle lose it score 20;
    task "Put ruby in case" tag pack_ruby need ruby lose it score 30;
    task "Put wax in case" tag pack_wax need wax lose it score 40;

After the treasure is stashed, we just waited for the helicopter and won
the game::

    task "Wait for helicopter" after pack_statue pack_bangle pack_ruby pack_wax cmd "WAIT" 2 finish;

The only thing remaining to do, map-wise, is to define the display style
we've used for the dark rooms.  Let's just set the room background colour
to be darker.  Note that we could also define the ``Dark`` style by putting
it in a file called :file:`Dark.ifm`, and then putting that file in a
directory which is searched by IFM.  That way, many maps could use the same
style definition.  Here's the definition::

    room_colour = "gray70" in style Dark;

Putting all these IFM snippets together (and indenting in my preferred
style) the completed map looks like this:

.. literalinclude:: examples/ruins.ifm

.. highlight:: text

Drawing the map
===============

At any point during the mapping of the game, we could run IFM and get a map
of where we've been (and where to explore next, since we marked room exits
with :keyword:`exit`).  We can do that, for example, like this::

    ifm -m -o ruins.ps ruins.ifm

and using our favourite PostScript viewer to view the map.  Here's what
the completed *Ruins* map looks like:

.. image:: images/ruins.png
   :align: center

Listing items
=============

We can see a complete item list using::

    ifm -i ruins.ifm

Here's what gets output with the completed *Ruins* map:

.. literalinclude:: examples/ruins-items.txt

Generating a solution
=====================

We can get a basic walkthrough of the game using::

    ifm -t ruins.ifm

and it looks like this:

.. literalinclude:: examples/ruins-tasks.txt

As you can see, IFM's solution is slightly different from the way we played
it: it stashes the statue straight away.  In the actual game, doing that
this early wouldn't work---the packing case hasn't been dropped yet.
Unless we're trying to make a recording that will play back in Frotz_ or
similar, this doesn't really matter.  If we *did* want to, we'd have to fix
this and create a recording file using::

    ifm -t -f rec ruins.ifm

This produces:

.. literalinclude:: examples/ruins-rec.txt

Another way to view the solution is using a *task dependency graph*.  This
uses IFM's :ref:`dot <dot>` output format, like this::

    ifm -t -o ruins.dot -f dot ruins.ifm
    dot -Tps -o ruins-graph.ps ruins.dot

Here's what *Ruins* looks like:

.. image:: images/ruins-graph.png
   :align: center

Yet another way to show this is by grouping tasks together according to the
rooms they occur in, by setting the ``task_graph_rooms`` variable::

    ifm -t -o ruins.dot -f dot -s task_graph_rooms=true ruins.ifm
    dot -Tps -o ruins-graph.ps ruins.dot

This is what you get:

.. image:: images/ruins-graph-byroom.png
   :align: center

Tweaking the solution
=====================

If we want to wait for the packing case to appear before trying to put the
statue in it, we could do it like this::

    task "Wait for packing case" in Dark_Forest need statue cmd "WAIT" 2;

Also, The solution as it stands is not as short as it could be: the game
solver takes a trip back up to the packing case to dump two treasures and
only then goes to get the bangle.  This is because the stashing-treasure
tasks are marginally closer at that point.

What we'd like is for the solver to grab all the underground treasure
*before* returning to the surface.  This can be done by extending the
:ref:`length <changing_lengths>` (i.e., movement cost) of the implicit link
between above-ground and below-ground::

    link Chamber length 10;

After adding this line, the solver behaves the way we want.

.. rubric:: Footnotes

.. [1] Yeah, right.  You can if you want.  I'm off to Ebay.
