# This file is part of IFM (Interactive Fiction Mapper), copyright (C)
# Glenn Hutchings 1997-2008.

# IFM comes with ABSOLUTELY NO WARRANTY.  This is free software, and you
# are welcome to redistribute it under certain conditions; see the file
# COPYING for details.

# scr2ifm -- convert game transcript to IFM commands

require 5.002;
use Getopt::Std;

# Compass direction command -> direction mapping.
%cdirmap = ("n" => "n", "north" => "n", "ne" => "ne", "northeast" => "ne",
	    "e" => "e", "east" => "e", "southeast" => "se", "se" => "se",
	    "south" => "s", "s" => "s", "southwest" => "sw", "sw" => "sw",
	    "west" => "w", "w" => "w", "northwest" => "nw", "nw" => "nw");

# Other direction command -> direction mapping.
%odirmap = ("up" => "up", "u" => "up", "down" => "down", "d" => "down",
	    "in" => "in", "out" => "out");

# Reverse-direction mapping.
%rdirmap = ("n" => "s", "s" => "n", "ne" => "sw", "sw" => "ne", "e" => "w",
	    "w" => "e", "se" => "nw", "nw" => "se", "in" => "out",
	    "out" => "in", "up" => "down", "down" => "up");

# Direction list in order of positioning preference.
@dirlist = ("n", "s", "e", "w", "ne", "se", "sw", "nw");

# List of transcript moves (hash entries).
@moves = ();

# Room tag -> move mapping.
%movemap = ();

# Room tag -> room data mapping.
%roommap = ();

# Linked rooms -> link data mapping.
%linkmap = ();

# List of IFM rooms.
@rooms = ();

# Default room name recognition parameters.
$name_maxwords = 8;
$name_maxuncap = 3;
$name_invalid = '[.!?"]';	# fix emacs highlighting with another "
$name_remove = '\s+\(.+\)';

# Default room description recognition parameters.
$desc_minwords = 20;

# Default command regexps.
$cmd_prompt = '^>\s*';
$cmd_look = '^l(ook)?$';
$cmd_undo = '^undo$';
$cmd_ifm = '^(title|map|item|task)';
$cmd_teleport = '^(restart|restore)$';
$cmd_ignore = '^unscript$';

### Stage 1 -- parse arguments and read input.

# Parse arguments.
$0 =~ s-.*/--;
&getopts('c:hilo:w') or die "Type `$0 -h' for help\n";
&usage if $opt_h;
    
# Read IFM command file if required.
if ($opt_c) {
    open(CMD, $opt_c) or die "Can't open $opt_c: $!\n";

    while (<CMD>) {
	# Skip blanks and comments.
	next if /^\s*#/;
	next if /^\s*$/;

	if (/set_param\s+(.+)/) {
	    # Perl expression.
	    eval $1;
	    &error("%s: %s", $opt_c, $@) if $@;
	} elsif (/set_option\s+(.+)/) {
	    # Set command-line option.
	    eval '$opt_' . $1 . ' = 1';
	    &error("%s: %s", $opt_c, $@) if $@;
	} elsif (/is_room\s+(.+)/) {
	    # Extra strings that are room names.
	    $is_room{$1}++;
	} elsif (/not_room\s+(.+)/) {
	    # Extra strings that aren't room names.
	    $not_room{$1}++;
	} elsif (/use_name\s+(.+)/) {
	    # Room name to be used.
	    $use_name{$1}++;
	} else {
	    # IFM command.
	    push(@ifmcmds, $_);
	}
    }

    close CMD;
}

# Redirect stdout if required.
if ($opt_o) {
    open(STDOUT, "> $opt_o") or die "Can't open $opt_o: $!\n";
}

foreach $file (@ARGV) {
    open(IN, $file) or die "Can't open $file: $!\n";

    # Skip input until first prompt.
    while (<IN>) {
	last if /$cmd_prompt/io;
    }

    # Read command/reply blocks.
    while (1) {
	# Get command.
	s/$cmd_prompt//io;
	chop($cmd = $_);
	$cmd =~ s/\s+$//;
	$line = $.;

	# Read reply.
	$reply = [];
	while (<IN>) {
	    last if /$cmd_prompt/io;
	    chop;
	    s/\s+$//;
	    push(@$reply, $_);
	}

	if ($cmd =~ /$cmd_undo/io) {
	    # Undo -- pop previous move (if any).
	    pop(@moves) unless $undone++;
	} else {
	    # Record move.
	    $move = {};

	    $move->{CMD} = $cmd;
	    $move->{REPLY} = $reply;
	    $move->{FILE} = $file;
	    $move->{LINE} = $line;
	    $move->{TELE}++ if $teleport;
	    push(@moves, $move);

	    undef $teleport;
	    undef $undone;
	}

	last if eof(IN);
    }

    close IN;
    $teleport++;
}

### Stage 2 -- scan moves for movement commands.

foreach $move (@moves) {
    undef $roomflag;
    undef $descflag;
    undef $desc;

    # Check for ignored command.
    next if $move->{CMD} =~ /$cmd_ignore/io;

    # Check for teleport command.
    $move->{TELE}++ if $move->{CMD} =~ /$cmd_teleport/io;

    for (@{$move->{REPLY}}) {
	$blank = /^\s*$/;

	# Check for room name.
	if (!$roomflag && !$blank && &roomname($_)) {
	    $roomflag++;
	    s/$name_remove//o if $name_remove;
	    $name = $_;
	    next;
	}

	# If got room name, decide if there's a blank line between
	# it and the description.
	$desc_gap = $blank if $roomflag && !defined $desc_gap;

	# If there isn't a blank line, but this line is, then there's
	# no description.
	last if $roomflag && !$desc_gap && $blank;

	# Check for room description.
	if ($roomflag && !$blank) {
	    $desc .= "# " . $_ . "\n";
	    $descflag++;
	    next;
	}

	# Check for end of description.
	last if $descflag && $blank;
    }

    # Store room info (if any).
    if ($roomflag) {
	$move->{NAME} = $name;
	$move->{DESC} = $desc if $desc;
	$move->{LOOK} = 1 if $move->{CMD} =~ /$cmd_look/io;

	&warning("room `%s' description is missing", $name)
	    if !$desc && !$descwarn{$name}++;
    }
}

### Stage 3 -- Build IFM room and link lists.

foreach $move (@moves) {
    $name = $move->{NAME};
    $desc = $move->{DESC};
    $file = $move->{FILE};
    $line = $move->{LINE};
    $cmd = $move->{CMD};

    # Check for IFM command.
    if ($cmd =~ /$cmd_ifm/io) {
	&ifmcmd($here, $file, $line, $cmd);
	next;
    }

    # Skip it if no room is listed.
    next unless $name;

    # If we teleported, try to find where we are now.
    $here = &findroom($name, $desc) if $move->{TELE};

    # If it's a LOOK command, or we don't know where we are yet,
    # set current room.
    if ($move->{LOOK} || !$here) {
	$here = &newroom($move, $name, $desc) unless $here;
	next;
    }

    # Otherwise, assume we moved in some way.  Try to find the new room.
    $there = &findroom($name, $desc);

    # If the new room looks like this one, do nothing.
    next if $there eq $here;

    if      ($cdirmap{$cmd}) {
	# Standard compass direction.
	$dir = $cdirmap{$cmd};
	undef $go;
	undef $cmd;
    } elsif ($odirmap{$cmd}) {
	# Other direction.
	$dir = &choosedir($here);
	$go = $odirmap{$cmd};
	undef $cmd;
    } else {
	# Weird direction.
	$dir = &choosedir($here);
	undef $go;
    }

    if (!$there) {
	# Unvisited -- new room.
	$here = &newroom($move, $name, $desc, $dir, $here, $go, $cmd);
    } else {
	# Visited before -- new link.
	&newlink($move, $here, $there, $dir, $go, $cmd);
	$here = $there;
    }
}

# Give first room a default map section name if required.
$room = $rooms[0];
if ($section > 1 && $room && !$room->{JOIN}) {
    $tag = $room->{TAG};
    $move = $movemap{$tag};
    $move->{MAP} = "Start";
}

### Stage 4 -- Write IFM output.

print "## IFM map created by $0\n";

print "\ntitle \"$title\";\n" if $title;

if ($section > 1) {
    print "\n## Map sections.\n";

    foreach $move (@moves) {
	$map = $move->{MAP};
	print "map \"$map\";\n" if $map;
    }
}

print "\n## Commands generated by transcript.\n";

foreach $move (@moves) {
    $list = $move->{IFM};
    next unless $list;

    undef $desc;
    foreach $obj (@$list) {
	$type = $obj->{TYPE};
	$file = $obj->{FILE};
	$line = $obj->{LINE};
	$name = $obj->{NAME};
	$tag = $obj->{TAG};
	$dir = $obj->{DIR};
	$from = $obj->{FROM};
	$to = $obj->{TO};
	$go = $obj->{GO};
	$cmd = $obj->{CMD};
	$join = $obj->{JOIN};
	$attr = $obj->{ATTR};

	if ($opt_i) {
	    if ($type eq "room" || $type eq "link") {
		print "\n" if $linecount++;
	    } else {
		print "  ";
	    }
	}

	if ($type eq "link" && $join) {
	    $type = "join";
	    $go = $dir if $dir;
	}

	print "$type ";

	if ($type eq "link" || $type eq "join") {
	    print "$from to $to";
	    print " dir $dir" if $dir && !$join;
	    undef $join;
	} else {
	    $name =~ s/"/\\"/g;
	    print "\"$name\"";
	    print " dir $dir" if $dir && !$join;
	    print " from $from" if $from && !$join;
	}

	print " go $go" if $go && !$join;
	print " cmd \"$cmd\"" if $cmd && !$join;
	print " $attr" if $attr;
	print " tag $tag" if $tag;

	print ";";
	print " \# $file ($line)" if $opt_l;
	print "\n";
    }
}

if (@ifmcmds > 0) {
    print "\n## Customization commands.\n";
    print @ifmcmds;
}

# Return whether a text line looks like a room name.
sub roomname {
    my $line = shift;

    # Remove unwanted stuff.
    $line =~ s/$name_remove//go if $name_remove;

    # User overrides.
    return 1 if $is_room{$line};
    return 0 if $not_room{$line};

    # Quick check for invalid format.
    return 0 if $name_invalid && $line =~ /$name_invalid/io;
    return 0 unless $line =~ /[A-Za-z]/;

    # Check word count.
    my @words = split(' ', $line);
    return 0 if $name_maxwords && @words > $name_maxwords;

    # Check uncapitalized words.
    return 0 if $line =~ /^[ a-z]/;

    for (@words) {
	return 0 if $name_maxuncap && /^[a-z]/ && length() > $name_maxuncap;
    }

    return 1;
}

# Add a new room to the room list.
sub newroom {
    my ($move, $name, $desc, $dir, $from, $go, $cmd) = @_;
    my $tag = &newtag("room", $name);

    my $room = {};
    push(@rooms, $room);
    push(@{$move->{IFM}}, $room);
    $movemap{$tag} = $move;

    $room->{TYPE} = "room";
    $room->{NAME} = $name;
    $room->{DESC} = $desc;
    $room->{WORDS} = [ split(' ', $desc) ];
    $room->{TAG} = $tag;

    $room->{DIR} = $dir if $dir;
    $room->{FROM} = $from if $from;
    $room->{GO} = $go if $go;
    $room->{CMD} = $cmd if $cmd;
    $room->{FILE} = $move->{FILE};
    $room->{LINE} = $move->{LINE};

    $section++ unless $from;
    $room->{SECT} = $section;

    $roommap{$tag} = $room;

    if ($from && $dir) {
	&moveroom($from, $dir);
	$roommap{$from}{$dir} = $tag;
	$linkmap{$from}{$tag} = $room;
    }

    &warning("more than one room with name `$name'")
	if ++$seenroom{$name} == 2;

    return $tag;
}

# Add a new link to the link list if required.
sub newlink {
    my ($move, $from, $to, $dir, $go, $cmd) = @_;
    my $link;

    if ($link = $linkmap{$from}{$to}) {
	# Link this way exists already -- do nothing.
    } elsif ($link = $linkmap{$to}{$from}) {
	# Opposite link exists -- see if it needs modifying.
	unless ($link->{MODIFIED}++) {
	    my $odir = $link->{DIR};
	    my $rdir = $rdirmap{$dir};
	    $link->{DIR} .= " " . $rdir
		unless $rdir eq $odir || $link->{GO} || $go;
	    $link->{GO} = $rdirmap{$go} if $go && !$link->{GO};
	    $link->{CMD} = $go if $cmd && !$link->{CMD};
	}
    } else {
	# No link exists -- create new one.
	$link = {};
	$move->{LINK} = $link;
	push(@{$move->{IFM}}, $link);

	$link->{TYPE} = "link";
	$link->{FROM} = $from;
	$link->{TO} = $to;
	$link->{TAG} = $from . "_" . $to;
	$link->{DIR} = $dir if $dir;
	$link->{GO} = $go if $go;
	$link->{CMD} = $cmd if $cmd;
	$link->{FILE} = $move->{FILE};
	$link->{LINE} = $move->{LINE};

	my $sectfrom = $roommap{$from}{SECT};
	my $sectto = $roommap{$to}{SECT};
	$link->{JOIN} = ($sectfrom != $sectto);

	&moveroom($from, $dir);
	$roommap{$from}{$dir} = $to;
	$linkmap{$from}{$to} = $link;
    }
}

# Add a new join to the join list if required.
sub newjoin {
    my ($move, $from, $to, $go, $cmd) = @_;

    $join = {};
    push(@{$move->{IFM}}, $join);

    $join->{TYPE} = "join";
    $join->{FROM} = $from;
    $join->{TO} = $to;
    $join->{TAG} = $from . "_" . $to;
    $join->{GO} = $go if $go;
    $join->{CMD} = $cmd if $cmd;
    $join->{LINE} = $move->{LINE};
}

# Find a room and return its tag given a name and description.
sub findroom {
    my ($name, $desc) = @_;
    my ($score, $best, $bestscore);
    my (@words, $match, $room, $i);

    foreach $room (@rooms) {
	undef $score;

	if ($desc && !$use_name{$name}) {
	    # We have a description -- try exact match first.
	    $score += 10 if $room->{DESC} eq $desc;

	    # Try substring match.
	    $score += 5 if index($room->{DESC}, $desc) >= 0;

	    # If still no luck, try first N words.
	    unless ($score) {
		$match = 1;
		@words = split(' ', $desc) unless @words;

		foreach $i (1 .. $desc_minwords) {
		    if ($words[$i] ne $room->{WORDS}->[$i]) {
			undef $match;
			last;
		    }
		}

		$score += 2 if $match;
	    }
	} else {
	    # Just the name -- not so good.
	    $score += 1 if $room->{NAME} eq $name;
	}

	next if $score <= $bestscore;
	$bestscore = $score;
	$best = $room->{TAG};
    }

    return $best;
}

# Move an up/down/in/out room if required.
sub moveroom {
    my ($from, $dir) = @_;

    # Check there's a room there.
    my $to = $roommap{$from}{$dir};
    return unless $to;

    # Check it's up/down/in/out.
    my $room = $roommap{$to};
    if ($room->{GO}) {
	# Put room in another direction.
	$dir = &choosedir($from);
	$roommap{$from}{$dir} = $to;
	$room->{DIR} = $dir;
    } else {
	# Warn about identical exits.
	$room = $roommap{$from};
	&warning("room `%s' has multiple exits (%s)",
		 $room->{NAME}, uc $dir);
    }
}

# Choose a direction to represent up/down/in/out.
sub choosedir {
    my $room = shift;
    my ($best, $bestscore, $score, $rdir, $dir);

    foreach $dir (@dirlist) {
	$rdir = $rdirmap{$dir};

	$score = 0;
	$score++ unless $roommap{$room}{$dir};
	$score++ unless $roommap{$room}{$rdir};
	next if defined $bestscore && $score <= $bestscore;

	$bestscore = $score;
	$best = $dir;
    }

    &warning("no up/down/in/out direction left for room `%s'",
	     $room->{NAME}) unless $bestscore;

    return $best;
}

# Make a new tag and return it.
sub newtag {
    my ($type, $name) = @_;
    my $prefix;

    # Build prefix.
    if ($type eq "room") {
	for (split(' ', $name)) {
	    $prefix .= $1 if /^([A-Z])/;
	}
    } else {
	$prefix = ucfirst $name;
	$prefix =~ s/[^A-Za-z0-9]/_/g;
    }

    # Make it unique.
    my $tag = $prefix;
    my $num = 1;
    while ($tagused{$type}{$tag}) {
	$tag = $prefix;
	$tag .= "_" unless $type eq "room";
	$tag .= ++$num;
    }

    $tagused{$type}{$tag}++;
    return $tag;
}

# Parse IFM command.
sub ifmcmd {
    my ($roomtag, $file, $line, $cmd) = @_;
    my $move = $movemap{$roomtag};
    my $room = $roommap{$roomtag};

    $cmd =~ s/\s*;\s*$//;

    if ($cmd =~ /^title\s+"(.+?)"/) {
	# Set title.
	$title = $1;
    } elsif ($cmd =~ /^map\s+"(.+?)"/) {
	# Start new map section in this room.
	$move->{MAP} = $1;
	my $dir = $room->{DIR};

	if ($dir) {
	    my @list = split(' ', $dir);
	    my $go = shift @list;
	    $go = $rdirmap{$go};
	    $go = $room->{GO} if $room->{GO};
	    &newjoin($move, $room->{FROM}, $room->{TAG}, $go, $room->{CMD});
	}

	$room->{JOIN}++;
	$room->{SECT} = ++$section;
    } elsif ($cmd =~ /^(\S+)\s+"(.+?)"\s*(.*)/) {
	# Define object.
	my $obj = {};

	$obj->{TYPE} = $1;
	$obj->{NAME} = $2;
	$obj->{ATTR} = $3;
	$obj->{FILE} = $file;
	$obj->{LINE} = $line;
	push(@{$move->{IFM}}, $obj);

	if ($cmd =~ /^item/) {
	    $lastitem = $obj;
	} else {
	    $lasttask = $obj;
	}

	if ($obj->{ATTR} !~ /\btag\b/) {
	    my $tag = &newtag($obj->{TYPE}, $obj->{NAME});
	    $obj->{ATTR} .= " " if $obj->{ATTR};
	    $obj->{ATTR} .= "tag $tag";
	}
    } elsif ($cmd =~ /^(\S+)\s*(.*)/) {
	# Add object attributes.
	my $type = $1;
	my $attr = $2;

	if      ($type eq "item" && $lastitem) {
	    $lastitem->{ATTR} .= " " if $lastitem->{ATTR};
	    $lastitem->{ATTR} .= $attr;
	} elsif ($type eq "task" && $lasttask) {
	    $lasttask->{ATTR} .= " " if $lasttask->{ATTR};
	    $lasttask->{ATTR} .= $attr;
	}

	if ($attr eq "delete") {
	    if ($type eq "item") {
		$lastitem->{DEL} = 1 if $lastitem;
	    } else {
		$lasttask->{DEL} = 1 if $lasttask;
	    }
	}
    }
}

# Print a warning.
sub warning {
    return if $opt_w;

    my $fmt = shift;
    print STDERR "$0: warning: ";
    printf STDERR $fmt, @_;
    print STDERR "\n";
}

# Print an error and exit.
sub error {
    my $fmt = shift;
    print STDERR "$0: error: ";
    printf STDERR $fmt, @_;
    print STDERR "\n";
    exit 1;
}

# Print a usage message and exit.
sub usage {
    print "Usage: $0 [options] file [file...]\n";
    print "   -c file        add ifm commands from file\n";
    print "   -o file        write to given file\n";
    print "   -i             indent and space output\n";
    print "   -l             print line number comments\n";
    print "   -w             don't print warnings\n";
    print "   -h             this help message\n";

    exit 0;
}
