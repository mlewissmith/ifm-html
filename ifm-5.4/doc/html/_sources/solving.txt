==================
 Solving the game
==================

.. highlight:: ifm

As well as making a map of your game, you can record the steps you took to
solve it.  IFM can then calculate a (fairly) optimal solution.  This
section is a guide to how to do it.  Again, it's not a complete
specification---see :doc:`language` for that.

.. index::
   single: task; Introduction to tasks
   single: safe; Introduction to tasks
   single: unsafe; Introduction to tasks
   single: in; Introduction to tasks
   single: in any; Introduction to tasks

Introduction to tasks
=====================

The basic game-solving action is called a *task* in IFM-speak.  To
introduce a task, you use the task command, like this::

    task "Put the fluffy bunny in the incinerator";

Most tasks need to be done in a certain room.  The default is that a task
must be done in the last room mentioned.  You can change that by using the
:keyword:`in` clause, just as for items.  Some tasks can be done
anywhere---you can say :keyword:`in any` to indicate that.  As usual, you
can add tags to tasks in order to refer to them later.

The game solver in IFM divides tasks into two fundamental types: *safe* and
*unsafe*.  An unsafe task is one that, if done, might get you into a
position where solving the game is impossible.  The game solver avoids
doing unsafe tasks until it really has to.

Task dependencies
=================

.. index::
   single: after; Requiring tasks
   single: after last; Requiring tasks
   single: follow; Requiring tasks

Requiring tasks
---------------

A lot of tasks require a previous task to be done first.  To say this, you
use the :keyword:`after` clause::

    task "Press incinerator start switch" tag press_switch;

    # other stuff

    task "Put the fluffy bunny in the incinerator" after press_switch;

As a shortcut, to avoid having to tag many tasks, you can say
:keyword:`after last` to indicate the last task mentioned.  For example, if
the two tasks above could be done in the same room, you could say::

    task "Press incinerator start switch" tag press_switch;
    task "Put the fluffy bunny in the incinerator" after last;

Alternatively, you could merge the two into a single task---the simplicity
or complexity of tasks is up to you.

The :keyword:`after` clause only says that a task will come after
another---it doesn't specify how soon after.  But in some situations it is
essential that a task immediately follows a specific previous task, without
deviation.  You can use the task :keyword:`follow` clause to specify this.
For example::

    room "Mission Control";
    task "Activate launch sequence" tag activate;

    # other stuff

    room "Rocket Cabin";
    task "Fasten seat belt" follow activate;

The :keyword:`follow` clause creates a chain of tasks that must be done one
after the other, with only movement commands allowed in between.  The game
solver will not attempt the first task in the chain until it knows that all
the tasks are possible (i.e., all the prerequisites for each task in the
chain are satisfied).  Also, if one of the tasks in the chain is unsafe,
then *all* previous tasks in the chain are also marked unsafe.

Of course, you can only have a single task in a :keyword:`follow`
clause---the immediately preceding task.  It is an error for two or more
tasks to try to immediately follow the same task.

.. index::
   single: need; Requiring items

Requiring items
---------------

For a lot of tasks, you need to have one or more items in your possession.
You can indicate this by using the :keyword:`need` clause, like this::

    task "Put the fluffy bunny in the incinerator" need bunny;

Here, ``bunny`` is the tag name of the corresponding item.  You can list
more than one item tag---e.g., :keyword:`need bunny asbestos_gloves`.

Note that you don't need to add tasks to get required items yourself---the
game solver does that automatically.  It knows it has to get all the items
which appear in :keyword:`need` clauses.

.. index::
   single: get; Obtaining items
   single: before; Obtaining items
   single: after; Obtaining items
   single: need; Obtaining items
   single: give; Obtaining items

Obtaining items
---------------

Sometimes a task needs to be done before you can get an item.  One way to
indicate this is with the task :keyword:`get` clause::

    task "Put money in drinks machine" need coin get lemonade;

This naturally implies that all tasks which supply an item (via the
:keyword:`get` clause) must be done before any task which needs that item.

An alternate way to phrase this is with the item :keyword:`after` clause,
which says that the item can't be picked up until a specified task is
done.  This is a common combination in IFM::

    task "Put money in drinks machine" need coin;
    item "lemonade" hidden after last;

Some items are only available before doing a certain task.  You can use the
:keyword:`before` clause to say that::

    item "precious diamond" before trigger_alarm;

Some items can only be picked up if you're already carrying another---use
the :keyword:`need` clause for that::

    item "hot coal" need tongs;

Sometimes doing a task not only allows you to get an item, but also puts it
in your inventory.  You can say that using the :keyword:`give` clause::

    task "Buy beer" need money give beer_mug;

The :keyword:`give` clause overrides all other restrictions placed on
getting items; the item is just teleported into your possession.

.. index::
   single: ignore; Ignoring tasks

Ignoring tasks
--------------

In some circumstances, all the effects of doing a task occur before the
task is done.  If this happens, the task will be ignored.  For example, if
a task *A* gives an item, but that item is first given by task *B*, then
task *A* will be ignored (provided it doesn't do anything else of
importance).

If a task has no effect, it is not ignored---it's assumed that you've
recorded that you did something but don't know why yet.  Also, tasks which
finish the game or score points are never ignored.

You can explicitly ignore a task using the :keyword:`ignore`
attribute.  This is useful while you're solving the game (see `Tweaking the
Solution`_) and when the task can be done by other tasks (see the next
section).

.. index::
   single: do; Doing tasks

Doing other tasks
-----------------

You can arrange for a task to automatically do other tasks after it is
done, using the :keyword:`do` clause.  For example::

    room "Control Room";
    task "Press airlock button" do open_airlock;

    # other stuff

    room "Outer Airlock";
    task "Open airlock" tag open_airlock;

    room "Inner Airlock" dir s after last;

In this example, the airlock can be opened in one of two ways: manually,
when in the Outer Airlock, or via the button in the Control Room.  Pressing
the button will cause the "open airlock" task to be done immediately
afterwards.  Note that if the manual method is used first, the press-button
task will be ignored.

The :keyword:`do` clause causes any task to be done---even tasks that have
prerequisites, and explicitly ignored ones.  This is useful in that you can
create special ignored tasks that can be done by any of a set of other
tasks, whichever gets there first.  The :keyword:`do` clause is also
recursive: a task can do another task which in turn does another, and so
on. [1]_

Note that any task which does an unsafe task in this way is itself marked
unsafe.

Handling items
==============

.. index::
   single: need; Inventory items
   single: get; Inventory items
   single: give; Inventory items
   single: keep; Inventory items
   single: keep_unused_items; Inventory items

.. index:: Variables; keep_unused_items

Inventory items
---------------

Items can be split into two types: *useful* and *useless*.  A useful item
one that is needed by at least one task, or is required in order to be able
to move somewhere; all other items are useless.  The game solver will try
to go and get all useful items, and will ignore the useless ones.  It keeps
track of the items it's carrying, and knows when a useful item is no longer
needed. [2]_ At that point, it will be dropped.

If the solver obtains a useless item (via a task :keyword:`get` or
:keyword:`give` clause, or an item :keyword:`need` clause) it will never
drop it.  This is just a default; you can change it by setting the variable
:keyword:`keep_unused_items` to ``false``.  In that case, useless items
will be dropped as soon as possible.

The reason for the default is that useless items obtained in this way
probably do have a purpose---you just don't know what it is yet.  This is
relevant when you're creating a recording from a partially-played game; see
:ref:`recording` for details.

If you want to make sure that an item is never dropped in any circumstance,
you can mark it with the :keyword:`keep` attribute.  This is handy for
items that act as general containers for other items.

Sometimes a useful item needs to be kept for longer than usual.  In the hot
coal example above, the tongs would be dropped as soon as the coal was
picked up, leaving you with a burnt hand.  What's needed here is to carry
the tongs for as long as you have the coal.  You can use the :keyword:`keep
with` clause to say that::

    item "hot coal" tag coal need tongs;
    item "tongs" tag tongs keep with coal;

Now the tongs won't be dropped until after the coal is, even if they have
no other use.  Similarly, there's also a :keyword:`keep until` clause,
which keeps an item until a specific task is done.

Finally, if a room has the attribute :keyword:`nodrop` set, no items will
be voluntarily dropped in that room.  Any items which need to be dropped
will then be dropped after the next task that happens in a room where
dropping is allowed.

.. index::
   single: lose; Losing items

Losing items
------------

Sometimes, doing a task causes items to be destroyed.  You can say that
with the :keyword:`lose` clause::

    task "Light bonfire" need match lose match;

This naturally implies that all other tasks which need the item must be
done before the task that destroys it.  A "drop" task is never generated
for items that are lost in this way.

Incidentally, you can use the special tag :keyword:`it` to refer to the
last room, item or task tag name within a command.  So the previous example
could also have been written::

    task "Light bonfire" need match lose it;

.. index::
   single: drop; Dropping items
   single: all; Dropping items
   single: except; Dropping items
   single: until; Dropping items

Dropping items
--------------

As mentioned in `Inventory items`_, IFM knows when a useful item is no
longer needed, and drops it automatically.  But sometimes items need to be
dropped temporarily, even though they're needed later.  You can do that
using the :keyword:`drop` clause::

    task "Throw spear at tree stump" need spear drop it;

In this example, the spear is dropped in the same room that the task was
done in.  If you ever need the spear for anything else, it will be picked
up again by the game solver.  Note that an item will only be dropped if it
is being carried---mentioning an item in a :keyword:`drop` clause does not
imply that it's needed to do the task.

Sometimes items must be dropped in a different room to the one you're
in.  You can use the :keyword:`in` clause to modify things::

    room "Top of Chute"; 
    task "Put laundry in chute" need laundry drop it in Bottom_of_Chute;

In other cases, you need to drop all the items you're carrying, or all
except certain items.  You can use :keyword:`drop all` and :keyword:`drop
all except` to say that.

Normally, if an item is dropped but is needed again for some other task,
there is nothing to stop the game solver picking it up again (provided
there's a path to the room the item was dropped in).  But sometimes you
need to drop an item and not pick it up again until you've done something
else.  You can use the :keyword:`until` clause to say that::

    task "Put coin in slot" give chocolate drop coin until open_machine;

A task which drops items will be marked unsafe if there is no path back to
the dropped items.

.. index::
   single: leave; Leaving items
   single: all; Leaving items
   single: except; Leaving items
   single: nodrop; Leaving items

Leaving items
-------------

There are some situations where your movement is blocked if you are
carrying particular items.  You can use the :keyword:`leave` attribute of
rooms, links and joins to specify a list of items that must be left behind
before using them.  For example::

    room "Bottom of Canyon"; 
    item "heavy boulder" tag boulder;

    room "Top of Canyon" dir n go up leave boulder;

If the :keyword:`leave` clause appears before the :keyword:`dir` clause,
that means the items must be dropped before entering the room (from any
direction).  It is generally the case that, if an attribute could apply to
a room or its implicit link with the previous one, its position relative to
the :keyword:`dir` clause is what decides it.

You can also say :keyword:`leave all`, which means that you must leave all
the items you're currently carrying, and :keyword:`leave all except`, which
omits certain items from being left behind.

When finding a solution, the game solver will carry items until it is
forced to drop them.  If the dropped items are needed later, the game
solver will try to come back and get them.  If it is trying to do a task
which requires items, it will choose a route to get to the task room which
doesn't involve dropping any of the needed items.

Note that the :keyword:`leave` clause overrides the room :keyword:`nodrop`
attribute; items will be dropped even in those rooms.

Moving around
=============

.. index::
   single: need; Limiting movement
   single: after; Limiting movement
   single: before; Limiting movement

Limiting movement
-----------------

Sometimes an item is required, or a task needs to be done, before movement
in a certain direction is possible.  To represent this, you can give
:keyword:`need` and :keyword:`after` clauses to rooms, links and joins.
For example::

    room "Cemetery" dir s from winding_path;
    task "Unlock the iron door" need rusty_key;

    room "Crypt" dir s go down after last;

Here's another example::

    room "Dimly-lit Passage" dir e;

    room "Dark Passage" dir e need candle;

In this case it is the link between the two rooms that is blocked off until
the candle is obtained.  If the :keyword:`need` clause had appeared before
the :keyword:`dir` clause, the restriction would apply to the room itself
(i.e., no entering the room from any direction without the candle).

In some cases, doing a task closes off a room, link or join so that it
can't be used any more.  You can use the :keyword:`before` clause to
indicate this::

    room "Bank Vault" tag Vault;

    room "Bank Entrance" tag Entrance dir e before trigger_alarm;

All tasks which close things off like this are marked unsafe, since they
could block off a crucial path through the game.

Sometimes in a game there is the situation where a path is closed off and,
later on in the game, reopened again.  A single link or join can't
represent this.  However, there's nothing to stop you from using two or
more joins between the same rooms.  If you mark them with the
:keyword:`hidden` attribute, they won't appear on the map either.  For
example, this line could be added to the previous example to provide an
escape route::

    join Vault to Entrance go e after disable_alarm hidden;

.. index::
   single: cmd; Movement tasks
   single: goto; Movement tasks

.. index:: Colossal Cave; Movement tasks

Movement tasks
--------------

There are several different ways of moving around in a game.  The usual way
is to say the direction you want to go in.  Another way is to do something
else which results in movement.  A good example is the magic word ``XYZZY``
from *Colossal Cave*.  It acts exactly like a movement command, in that you
can use it again and again and it moves you somewhere predictable.  The
best way to represent this in IFM is to use a join to connect the two
rooms, and specify the command used to do the movement via the
:keyword:`cmd` clause, like this::

    join Debris_Room to Building after examine_wall cmd "XYZZY";

Yet another way of moving around is a one-off event that "teleports" you to
a different room.  You can indicate that this happens using the task
:keyword:`goto` clause [3]_ and supplying the tag name of the destination
room.

For example::

    task "Get captured by goblins" goto Dungeon;

As soon as the task is done, you teleport to the new location---no
intervening rooms are visited.  Note that because each task is only done
once, this method of travel can only be used once.  Note also that the
:keyword:`drop` and :keyword:`leave` actions are done before teleporting
you to the new location (so if you drop items in the "current room", you
will be teleported away from the dropped items).

Other game features
===================

.. index::
   single: score; Scoring points

Scoring points
--------------

Many games have some sort of scoring system, whereby you get points for
doing various things.  In IFM you can record this using the
:keyword:`score` clause, which can apply to rooms, items or tasks.  It
takes one integer argument---a score value:

* For rooms, it's the score you get when visiting it for the first time.
* For items, it's the score for first picking it up.
* For tasks, it's the score for doing that task.

If an item has a score, but is given to the player via a task
:keyword:`give` clause, then its score is added to the score for that task
instead.

.. index::
   single: finish; Finishing the game

Finishing the game
------------------

Usually a game finishes when you complete some final task.  You can
indicate which task this is using the :keyword:`finish` attribute.  This
attribute can attach to rooms, items or tasks, giving three different types
of finish condition: entering a room, picking up an object or doing a task.
If the game solver ever manages to do something which is flagged with the
:keyword:`finish` attribute, it considers the game solved and stops.  Any
extra things left to do will not be done, even if they score points.

.. index::
   single: unsafe; Finding a solution
   single: safe; Finding a solution
   single: inventory; Finding a solution

.. _finding_solution:

Finding a solution
==================

Here's what the game solver does in order to come up with a solution to the
game.  First, extra internal tasks are generated.  These are tasks to:

* get items which are required for explicitly-mentioned tasks to be done,
* get items which are required to get other items,
* get items which are needed to go in certain directions,
* get items which are scored,
* go to rooms which are scored.

Next, all the rooms are connected using their links and joins.  This means
that for each room, a list is made of all other rooms reachable in one
move.  Note that it is possible for some rooms to be unreachable---for
example, all rooms in a section where there is no "join" to rooms on other
sections.

Then the task *dependencies* are calculated.  A dependency is where one
task must be done before another.  The task dependencies are examined to
see if there are any *cycles*: chains of tasks where each one must be done
before the next, and the last must be done before the first.  If there are
any, then the game is unsolvable, since none of the tasks in a cycle can be
done.

If there are no cyclic dependencies, the task list is *traversed* to find a
sequence which solves the game while satisfying the task dependencies.  The
start room is the room which was first mentioned in the input (but this can
be changed---see :doc:`language`).  While there are tasks left in the task
list, the following steps are performed:

1. The inventory is examined to see if there are any unwanted items; if so,
   and dropping items in the current room is allowed, they are dropped.  An
   item is wanted if at least one of the following is true:

   * it's needed for movement,

   * it's needed for a task that hasn't been done yet,

   * it's being kept unconditionally,

   * it's being kept with another item that's carried,

   * it's being kept until a certain task is done.

2. The map is traversed to find the distances of all rooms from the current
   room.  Then the task list is sorted in order of ascending distance of the
   rooms they must be done in.  Tasks which can be done in any room count as
   having distance zero.

3. The sorted task list is scanned to find the nearest 
   possible task.  For a task to be possible, the player must:

   * have all required items,

   * have done all required previous tasks,

   * be able to get from the current room to the task room via a path which
     doesn't require items not yet collected, or tasks not yet done, or
     which involves dropping needed items on the way.

   Priority is given to *safe* tasks.  For a task to be safe,

   * it must not have previously been marked unsafe (e.g., because it
     closes off map connections),

   * there must be a return path from the task room back to the current
     one.  This is to avoid taking a one-way trip before preparing properly.

   If there are any safe tasks, the nearest one will be done next
   regardless of how close an unsafe task is.  If there are no safe task,
   the nearest unsafe task will be chosen.

4. If there was a possible task, do it and remove it from the list.  Move
   to the room the task was done in (if any).  If not, then the game is
   unsolvable.  Give up.

5. Finally, examine the list of remaining tasks to see if any are now
   redundant and can be removed.  A redundant task is one that only does
   something that's already been done (e.g., go and get an item that you've
   already been given).

Tweaking the solution
=====================

There will be some situations (quite a few, actually) where the game solver
doesn't do things the way you want it to.  This section gives a few tips,
and some new keywords, for modifying things.

.. index::
   single: safe; Making things safe
   single: all_tasks_safe; Making things safe
   single: Variables; all_tasks_safe

Making things safe
------------------

Firstly, the game solver is completely paranoid.  It has to be, because it
doesn't do any lookahead past the current task.  It won't do anything unsafe
(e.g., go to a room to do a task when there's no immediate return journey)
unless there's nothing safe left to do.  It will quite happily plod halfway
across the map to pick something up rather than do something a bit scary in
the next room.

However, you can reassure it with the task :keyword:`safe`
attribute.  Adding this to a task tells the solver that this task is safe,
regardless of what it thinks.  So if you know that a one-way trip can
eventually be returned from, by doing other tasks, you can stop the solver
from avoiding it.  But bear in mind that by doing this you are taking full
responsibility if the solver gets stuck.

If you want to be seriously reckless, you can set the variable
:keyword:`all_tasks_safe` to a nonzero value.  Then, all tasks will be
considered safe.

.. index::
   single: length; Changing path lengths

.. _changing_lengths:

Changing path lengths
---------------------

Another thing the solver doesn't know about is how easy or difficult it is
to get from place to place on the map.  Suppose you're in a game which is on
two levels separated by a tiresome set of access doors with ID cards.  The
connection between the levels may only be two rooms on the map, but it's a
lot more in terms of typing.  You can avoid unnecessary trips through these
doors by artificially changing the *length* of the connection between
levels, by using the :keyword:`length` attribute of links and joins::

    room "Level A" tag LA;

    room "Level B" tag LB dir e length 50;

In this way, by choosing an appropriate number for the length, you make it
appear to the solver that all the rooms in level *A* are closer to each
other than any of the rooms in level *B*.  This means that priority will be
given to tasks in rooms in the same level as you are now, (hopefully)
minimizing the number of level changes.  Note that the :keyword:`length`
attribute doesn't affect map drawing at all.

.. index::
   single: nopath; Closing off paths

Closing off paths
-----------------

There may be times when you want a map connection to appear on the map, but
not be used in solving the game---for example, it may be certain death to
go that way.  You can use the :keyword:`nopath` attribute of rooms, links
and joins to indicate this.  It doesn't affect map output in any way.

Another use for this attribute is to force the game solver to do things in
a different order.  This might be preferable to adding fake task
dependencies.

.. index::
   single: ignore; Ignoring parts of the solution
   single: do; Ignoring parts of the solution

Ignoring parts of the solution
------------------------------

Sometimes it's useful to be able to ignore certain parts of the
solution---for example, if you want to generate a sequence of game commands
that get you to a particular position as quickly as possible.  To that end,
you can mark tasks and items with the :keyword:`ignore` attribute.  An
ignored task will never be attempted, and an ignored item will never be
picked up.  This means that anything dependent on those tasks or items will
not be done either.  The game will very probably be unsolvable as a result,
unless you've ignored an unused item, or ignored a task that's done
elsewhere via a :keyword:`do` clause.

Keeping fixes together
----------------------

It's probably best to keep all your "game tweaks" together, separate from
the "pure" game, and commented appropriately.  You can do this by using
commands which just modify existing objects, instead of creating new ones,
by referring to their tags.  As an example, suppose you have the following
situation::

    room "Top of Chute";

    room "Bottom of Chute" dir s go down oneway;

    task "Do something weird" tag weird_task;

Suppose you're at the top of the chute, and that there's some stuff to be
done at the bottom, but no immediate way back up.  As usual, the game solver
balks at taking a one-way trip and will do anything to avoid it.  But
suppose you know that, as long as you have your giant inflatable
cheeseburger, you can get back out again.  You can say::

    # Bottom of chute isn't that scary.

    task weird_task need burger safe;

which modifies the task at the bottom of the chute to (a) require the
burger (so that you won't go down there without it), and (b) be considered
safe by the game solver.  So it will happily slide down the chute without
getting stuck at the bottom.

This way of modifying previous objects applies all types of object, even
links and joins---these can be tagged too, in the normal way.  The single
exception is the implicit link created by the room :keyword:`dir`
clause.  These links automatically get tagged when the room does, and with
the same name.  So the two-level example above could be split into::

    room "Level A" tag LA;

    room "Level B" tag LB dir e;

    # other stuff

    # Stop gratuitous travel between levels.
    link LB length 50;

.. index::
   single: solver_messages; Displaying solver messages
   single: Variables; solver_messages

Displaying solver messages
--------------------------

Finally, you can gain an insight into what the game solver's up to by
setting the :keyword:`solver_messages` variable (either in one of the input
files, or via the :option:`-set` command-line option).  This produces reams
of output giving details of the game solver's thoughts before it does
anything. [4]_

.. index::
   single: inventory; Limitations
   single: length; Limitations
   single: follow; Limitations
   single: leave; Limitations
   single: need; Limitations

Limitations
===========

Given the wild imaginations of today's IF authors, there are bound to be
some game solving situations that can't easily be dealt with using
IFM.  Some of the things that IFM ignores are:

* Random events.  For example, the Carousel room in Zork, and all the NPCs
  in *Colossal Cave*.  There's no way to address this problem, but then
  again, hand-written walkthroughs have the same difficulty.  However, if
  you're trying to tailor recording output so that it will play back
  properly in an interpreter, there is a workaround---see :ref:`recording`.

* Carrying capacity.  A solution may require you to carry more than you're
  allowed.  This might be addressed in a future version, but
  inventory-juggling puzzles are out of fashion these days (if they were
  ever in) so this is not too much of a problem.  Some games provide you
  with an item that can carry stuff for you---if so, a workaround is to add
  some special tasks that periodically put everything you're carrying into
  it.

There are some other limitations that are the result of certain keyword
combinations in the current implementation of IFM.  These are fixable, and
hopefully will be in a later version.  They are:

* If you have more than one link or join which connects the same two rooms,
  then if any of them set the :keyword:`length` attribute, they must all
  set it---and to the same value.  Otherwise IFM will give an error.

* Unsafe tasks in a :keyword:`follow` task chain normally cause all the
  previous tasks in the chain to be marked unsafe too (so the solver will
  avoid trying the first, knowing it'll be forced to do something
  distasteful later).  However, some tasks are not known to be unsafe until
  just before they might be done---specifically, those for which there is
  no return path.  This is because whether there's a return path depends on
  where you are now.  So a :keyword:`follow` chain could possibly lead to a
  game-solving dead end.

* There's a problem with the :keyword:`leave/need` attribute
  combination.  The game solver could select a path from one room to another
  in which an item must be left behind at one point, but is needed for
  movement later on in the path.  This sort of path should be invalid, but
  isn't.

.. rubric:: Footnotes

.. [1] However, you can't create an infinite loop since each task can only
       be done once.

.. [2] It has a magic crystal ball that can see into the future.

.. [3] All the best languages have a :keyword:`goto` statement, you know.

.. [4] It's supposed to be self-explanatory, but my view is slightly
       biased.  Detailed documentation may follow (a) if enough people ask
       for it, and (b) if I ever get around to it.
