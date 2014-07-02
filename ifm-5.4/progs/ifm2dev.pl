# This file is part of IFM (Interactive Fiction Mapper), copyright (C)
# Glenn Hutchings 1997-2008.

# IFM comes with ABSOLUTELY NO WARRANTY.  This is free software, and you
# are welcome to redistribute it under certain conditions; see the file
# COPYING for details.

# ifm2dev -- convert IFM map to various formats

require 5.002;
use Getopt::Std;

# Parse arguments.
$0 =~ s-.*/--;
$figopts = 'bdfglmnopqstxyzACLSXY';
getopt($figopts, \%opts);
usage() if $opts{h};

# Set default output format.
$opts{L} = "eps" unless $opts{L};

# Set output template.
if ($opts{o}) {
    $template = $opts{o};
    die "$0: error: output template must contain %d\n"
	unless $template =~ /%[0-9]*d/;
    delete $opts{o};
}

# Set verbose flag.
$verbose = $opts{v};
delete $opts{v};

# Build fig2dev command.
$fig2dev = "fig2dev";
foreach $opt (keys %opts) {
    $fig2dev .= " -$opt";
    $fig2dev .= " $opts{$opt}" if $figopts =~ /$opt/;
}

# Parse any extra IFM arguments.
$ifmopts = 'sSI';
getopt($ifmopts, \%ifmopts);

# Build IFM command.
$ifm = "ifm";
foreach $opt (keys %ifmopts) {
    $ifm .= " -$opt";
    $ifm .= " $ifmopts{$opt}" if $ifmopts =~ /$opt/;
}

# Get IFM file.
$file = shift(@ARGV) or die "$0: error: no IFM file specified\n";

# Get no. of map sections to output.
@data = `$ifm -show maps $file`;
shift(@data);
$count = @data;
die "$0: error: can't find any map sections\n" if $count <= 0;

# Set output template if not done yet.
unless ($template) {
    $template = $file;
    $template =~ s/\.ifm$//;
    $template .= "-%02d.$opts{L}";
}

# Convert each map section.
for ($num = 1; $num <= $count; $num++) {
    $output = sprintf($template, $num);
    $cmd = "$ifm -nowarn -format fig -map=$num $file | $fig2dev > $output";
    print $cmd, "\n" if $verbose;
    system $cmd;
}

# Er... that's it.
exit 0;

# Print a usage message and exit.
sub usage {
    print "Usage: $0 [options] [fig2dev-options] [-- ifm-options] file\n";
    print "   -o file  set output file template\n";
    print "   -h       this help message\n";

    exit 0;
}
