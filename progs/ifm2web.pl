# This file is part of IFM (Interactive Fiction Mapper), copyright (C)
# Glenn Hutchings 1997-2008.

# IFM comes with ABSOLUTELY NO WARRANTY.  This is free software, and you
# are welcome to redistribute it under certain conditions; see the file
# COPYING for details.

# ifm2web -- convert IFM map to image suitable for the Interweb

require 5.000;
use Getopt::Std;

# Parse arguments.
$0 =~ s-.*/--;
getopts('a:ghm:no:rs:twz:S:', \%opts) || die "Type '$0 -h' for help\n";
usage() if $opts{h};

$maps = $opts{m};
$dotprog = $opts{n} ? "neato" : "dot";
$title = $opts{t} ? "true" : "false";
$rooms = $opts{r} ? "true" : "false";
$output = $opts{o};
$alpha = $opts{a} || "white";
$zoom = $opts{z} || 1;
$scale = $opts{s} || 100;
$style = $opts{S};
$whatif = $opts{w};

# Get IFM file.
$file = shift(@ARGV) or moan("no IFM file specified");

$prefix = $file;
$prefix =~ s/\.ifm$//;
$output = $prefix . ".png" unless $output;

# Build IFM command.
$ifm  = "ifm -nowarn";
$ifm .= " -S $style" if $style;

if ($opts{g}) {
    $ifm .= " -tasks -format dot";
    $ifm .= " -s task_graph_rooms=$rooms";
} else {
    $ifm .= " -map";
    $ifm .= "=$maps" if $maps;
    $ifm .= " -format fig";
    $ifm .= " -s show_map_title=$title";
    $ifm .= " -s show_map_border=false";
    $ifm .= " -s map_border_colour=$alpha";
    $ifm .= " -s map_background_colour=$alpha";
}

foreach $opt (keys %ifmopts) {
    $ifm .= " -$opt";
    $ifm .= " $ifmopts{$opt}" if $ifmopts =~ /$opt/;
}

# Build conversion command.
$cmd = "$ifm $file";
$cnv = "convert -transparent $alpha -scale $scale%";

if ($opts{g}) {
    $cmd .= " | $dotprog -Tpng";
    $cmd .= " | $cnv -rotate 90 - $output";
} else {
    $cmd .= " | fig2dev -L png -m $zoom";
    $cmd .= " | $cnv - $output";
}

if ($whatif) {
    print $cmd, "\n";
} else {
    system $cmd;
}

# Print an error message and die.
sub moan {
    die "$0: error: ", join(' ', @_), "\n";
}

# Give a usage message and exit.
sub usage {
    print "Usage: $0 [options] file\n";

    print "\nMap output:\n";
    print "  -m sections  print the specified map sections\n";
    print "  -z zoom      set the fig2dev(1) zoom factor\n";
    print "  -t           show section titles\n";

    print "\nTask output:\n";
    print "  -g           write task graph instead of map\n";
    print "  -r           group tasks by room\n";
    print "  -n           use neato(1) instead of dot(1)\n";

    print "\nOther options:\n";
    print "  -s scale     set convert(1) scale factor\n";
    print "  -S style     use the specified style\n";
    print "  -o file      write to specified file\n";
    print "  -n           just print what would be done\n";
    print "  -h           this help message\n";

    exit 0;
}
