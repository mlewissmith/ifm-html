# This file is part of IFM, the Interactive Fiction Mapper, copyright (C)
# Glenn Hutchings 1997-2006.

# IFM comes with ABSOLUTELY NO WARRANTY.  This is free software, and you
# are welcome to redistribute it under certain conditions; see the file
# COPYING for details.

# ifm2tex -- convert IFM map to LaTeX document

require 5.002;
use Getopt::Std;

# Parse arguments.
$0 =~ s-.*/--;
getopt('', \%opts);
usage() if $opts{h};

# Get IFM file.
$file = shift(@ARGV) or moan("no IFM file specified");

$prefix = $file;
$prefix =~ s/\.ifm$//;

$mainfile = $prefix . ".tex";
$mapsfile = $prefix . "-maps.tex";
$itemfile = $prefix . "-items.tex";
$taskfile = $prefix . "-tasks.tex";
$template = $prefix . "-map-%02d.eps";

# Parse any extra IFM arguments.
$ifmopts = 'sSI';
getopt($ifmopts, \%ifmopts);

# Build IFM command.
$ifm = "ifm";
foreach $opt (keys %ifmopts) {
    $ifm .= " -$opt";
    $ifm .= " $ifmopts{$opt}" if $ifmopts =~ /$opt/;
}

# Read raw IFM data.
$data = ifmraw($file) or moan("can't get map info");

# Get file info.
@sects = @{$data->{section}};
@rooms = @{$data->{room}};
@items = @{$data->{item}};
@tasks = @{$data->{task}};
@order = @{$data->{taskorder}};

$wantmaps = $opts{m};
$wantitems = $opts{i} && @items > 0;
$wanttasks = $opts{t} && @order > 0;

# Open main output file.
open(STDOUT, "> $mainfile") or moan("can't open $mainfile: $!");

# Print document header.
print "\\documentclass{article}\n";
print "\\usepackage[T1]{fontenc}\n";
print "\\usepackage{geometry}\n";
print "\\usepackage{verbatim}\n";
print "\\usepackage{longtable}\n";
print "\\usepackage{graphicx}\n";

# Start document.
print "\\begin{document}\n";

# Print title.
printf "\\title{%s}\n", texstr($data->{title});
print "\\maketitle\n";
print "\\vspace{1cm}\n";
print "\\listoffigures\n" if $wantmaps;
print "\\listoftables\n" if $wantitems || $wanttasks;
print "\\titlepage\n";

# Write map sections.
if ($opts{m}) {
    print "\\newpage\n";
    printf "\\input{%s}\n", $mapsfile;

    open(OUT, "> $mapsfile") or moan("can't open $mapsfile: $!");

    # Convert each map section.
    system "ifm2dev -o $template -- -w $file";

    # Get dimensions of each one.
    @data = `$ifm -nowarn -show maps $file`;
    shift(@data);

    # Write each map section.
    for ($i = 0; $i < @sects; $i++) {
	$mapfile = sprintf($template, $i + 1);
	($num, $count, $w, $h, $name) = split(' ', $data[$i], 5);
	chomp($name);
	$scale = 1.0;

	if      ($w >= 10 && $h >= 10) {
	    $opts = "width=$scale\\textwidth,height=$scale\\textheight,keepaspectratio";
	} elsif ($w >= 10) {
	    $opts = "width=$scale\\textwidth";
	} elsif ($h >= 10) {
	    $opts = "height=$scale\\textheight";
	} else {
	    $opts = "scale=0.7";
	}

	print OUT "\\addcontentsline{lof}{figure}{" . texstr($name) . "}\n";
	print OUT "\\begin{center}\n";
	print OUT "\\includegraphics[$opts]{$mapfile}\n";
	print OUT "\\end{center}\n";
	print OUT "\\vspace{1cm}\n";
    }

    close(OUT);
}

# Write item list.
if ($wantitems) {
    print "\\newpage\n";
    printf "\\input{%s}\n", $itemfile;

    open(OUT, "> $itemfile") or moan("can't open $itemfile: $!");

    print OUT "\\addcontentsline{lot}{table}{Items}\n";
    print OUT "{\\small\n";
    print OUT "\\begin{longtable}{|l|p{1.2in}|p{3in}|}\n";
    print OUT "\\hline\n";
    print OUT "\\textbf{Item} & \\textbf{Room} & \\textbf{Details} \\\\\n";
    print OUT "\\hline\n";
    print OUT "\\hline\n";
    print OUT "\\endhead\n";

    foreach $item (sort { $a->{name} cmp $b->{name} } @items) {
	$room = room($item);
	$name = $item->{name};
	$score = $item->{score};
	$roomname = defined($room) ? $room->{name} : "";

	undef @details;

	if ($item->{after}) {
	    undef $str;
	    foreach $id (@{$item->{after}}) {
		$task = $tasks[$id];
		next unless $task;

		$room = room($task);

		if ($str) {
		    $str .= ", ";
		} else {
		    $str = "Obtained after ";
		}

		$str .= "\\textbf{" . texstr($task->{name}) . "}";
		$str .= " (" . texstr($room->{name}) . ")" if $room;
	    }

	    $str .= ".";
	    push(@details, $str);
	}

	if ($item->{needed}) {
	    undef $str;
	    foreach $id (@{$item->{needed}}) {
		$task = $tasks[$id];
		next unless $task;

		$room = room($task);

		if ($str) {
		    $str .= ", ";
		} else {
		    $str = "Needed for ";
		}

		$str .= "\\textbf{" . texstr($task->{name}) . "}";
		$str .= " (" . texstr($room->{name}) . ")" if $room;
	    }

	    $str .= ".";
	    push(@details, $str);
	}

	if ($item->{enter}) {
	    undef $str;
	    foreach $id (@{$item->{enter}}) {
		$room = $rooms[$id];

		if ($str) {
		    $str .= ", ";
		} else {
		    $str = "Needed to enter ";
		}

		$str .= "\\textbf{" . texstr($room->{name}) . "}";
	    }

	    $str .= ".";
	    push(@details, $str);
	}

	if ($item->{move}) {
	    undef $str;
	    foreach $entry (@{$item->{move}}) {
		($id1, $id2) = split(' ', $entry);
		$room1 = $rooms[$id1];
		$room2 = $rooms[$id2];

		if ($str) {
		    $str .= ", ";
		} else {
		    $str = "Needed to move ";
		}

		$str .= "from \\textbf{" . texstr($room1->{name}) . "}";
		$str .= " to \\textbf{" . texstr($room2->{name}) . "}";
	    }

	    $str .= ".";
	    push(@details, $str);
	}

	push(@details, "Scores $score points.") if $score;
	push(@details, "Finishes the game.") if $item->{finish};
	push(@details, "May have to be left behind.") if $item->{leave};
	push(@details, "Hidden.") if $item->{hidden};
	push(@details, texstr(join(". ", @{$item->{note}})) . ".")
	    if $item->{note};

	printf OUT "%s &\n", texstr($name);
	printf OUT "%s &\n", texstr($roomname);
	printf OUT "%s \\\\\n", join(" ", @details);
	print OUT "\\hline\n";
    }

    print OUT "\\end{longtable}\n";
    print OUT "}\n";

    close(OUT);
}

# Write task list.
if ($wanttasks) {
    print "\\newpage\n";
    printf "\\input{%s}\n", $taskfile;

    open(OUT, "> $taskfile") or moan("can't open $taskfile: $!");

    print OUT "\\addcontentsline{lot}{table}{Tasks}\n";
    print OUT "{\\small\n";
    print OUT "\\begin{longtable}{|l|p{1.5in}|p{1in}|p{1.3in}|}\n";
    print OUT "\\hline\n";
    print OUT "\\textbf{Room} & \\textbf{Task} & ";
    print OUT "\\textbf{Commands} & \\textbf{Details} \\\\\n";
    print OUT "\\hline\n";
    print OUT "\\hline\n";
    print OUT "\\endhead\n";

    foreach $id (@order) {
	$task = $tasks[$id];
	$room = room($task);
	$name = $task->{name};
	$score = $task->{score};
	$roomname = defined($room) ? $room->{name} : "";

	undef@details;
	push(@details, "Scores $score points.") if $score;
	push(@details, "Finishes the game.") if $task->{finish};
	push(@details, texstr(join(". ", @{$task->{note}})) . ".")
	    if $task->{note};

	printf OUT "%s &\n", texstr($room != $oldroom ? $roomname : "");
	printf OUT "%s &\n", texstr($name);
	printf OUT "\\texttt{%s} &\n", uc texstr(join(". ", @{$task->{cmd}}));
	printf OUT "%s \\\\\n", texstr(join(" ", @details));
	print OUT "\\hline\n";

	$oldroom = $room;
    }

    print OUT "\\end{longtable}\n";
    print OUT "}\n";

    close(OUT);
}

# Finish document.
print "\\end{document}\n";

# Er... that's it.
close(STDOUT);
exit 0;

# Read raw IFM data.
sub ifmraw {
    my %listtag = ('lpos' => 1, 'cmd' => 1, 'after' => 1, 'needed' => 1,
		   'enter' => 1, 'move' => 1, 'note' => 1);

    my $cmd = "$ifm -nowarn -map -items -tasks -format raw";
    my ($data, $list, $obj, $section);
    my $file = shift;

    open(IFM, "$cmd $file 2>&1 |") or moan("can't run IFM: $!");

    while (<IFM>) {
	chop;

	if      (/error:/) {
	    die "$_\n";
	} elsif (/^$/) {
	    undef $obj;
	} elsif (/^(\w+):\s+(.+)/) {
	    my $tag = $1;
	    my $val = $2;

	    if      (defined $obj) {
		if ($listtag{$tag}) {
		    push(@{$obj->{$tag}}, $val);
		} else {
		    $obj->{$tag} = $val;
		}
	    } elsif ($tag eq "title") {
		$data->{$tag} = $val;
	    } elsif ($tag eq "section") {
		$obj->{name} = $val;
		push(@{$data->{$tag}}, $obj);
		$section++;
	    } elsif ($tag eq "room" ||
		     $tag eq "item" ||
		     $tag eq "task") {
		$obj->{id} = $val;
		$obj->{section} = $section - 1 if $tag eq "room";
		push(@{$data->{taskorder}}, $val) if $tag eq "task";
		${$data->{$tag}}[$val] = $obj;
	    } elsif ($tag eq "link" ||
		     $tag eq "join") {
		$obj->{id} = $val;
		push(@{$data->{$tag}}, $obj);
	    } else {
		moan("line $.: invalid input: $_");
	    }
	}
    }

    close IFM;

    return $data;
}

# Get the room of an object, if any.
sub room {
    my $obj = shift;
    my $id = $obj->{room};
    return $rooms[$id] if defined $id;
    return undef;
}

# Convert a string for feeding to LaTeX.
sub texstr {
    my $text = shift;
    $text =~ s/([\\\#\$%&~_^{}])/\\$1/g;
    return $text;
}

# Print an error message and die.
sub moan {
    die "$0: error: ", join(' ', @_), "\n";
}

# Print a usage message and exit.
sub usage {
    print "Usage: $0 [options] [-- ifm-options] file\n";
    print "   -m       print maps\n";
    print "   -i       print item table\n";
    print "   -t       print task table\n";
    print "   -h       this help message\n";

    exit 0;
}
