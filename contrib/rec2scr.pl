#! /usr/bin/env perl

=head1 NAME

rec2scr -- convert IF recording output to game transcript

=head1 SYNOPSIS

rec2scr [options] zcode-file

=head1 DESCRIPTION

B<rec2scr> takes a Z-code IF game file and a list of recorded game commands
on stdin and creates a game transcript from them on stdout, by feeding the
commands to the game.  It does this non-interactively, using the
B<dumb-frotz> interpreter (available from C<ftp.gmd.de>).

The Z-code file specified on the command line can exist in the current
directory or in a directory where Z-code files are kept (as specified by
the B<-z> option or the C<REC_ZCODE_DIR> environment variable).  If the
file is not found as-is, then the standard Z-code suffixes (.z3, .z4, etc)
are appended to try and find it.

=head1 OPTIONS

=over 4

=item B<-r> F<file>

Read recording commands from the specified file, instead of stdin.

=item B<-s> seed

Set the random number seed.  This is useful for eliminating various random
factors when tailoring your recording file to solve the game.

=item B<-t>

Make a transcript file, instead of capturing B<dumb-frotz> standard output.

=item B<-c> F<file>

Specify a command file.  The commands in this file are fed to B<dumb-frotz>
before playing back the recorded commands.  This is useful for entering
various game-specific commands before starting the game proper.  See
L<"COMMANDS">, below.

=item B<-z> F<dir>

Specify directory where Z-code files are kept.

=item B<-g> F<file>

Save game to the specified file after playing back the recording.

=item B<-o> F<file>

Write output to the specified file, instead of stdout.

=item B<-h>

Give a usage message.

=back

=head1 COMMANDS

A command file can contain comments (starting with a hash) and blank lines,
which are ignored.  It can also contain the following statements:

=over 4

=item cmd "string"

Specify a command to send to the game, followed by RETURN.  The
double-quotes are required.

=item print /regexp/

Print the output line containing the specified regexp, and the 4 lines
following, to stderr.  This is useful for quickly looking at the random
elements of a game to see if anything has changed.

=back

=head1 RESTRICTIONS

Things are very dependent on the nuances of the current version of
B<dumb-frotz>.

B<rec2scr> tries to clean up the transcript after B<dumb-frotz> has printed
it, but does it very simplistically.

=head1 ENVIRONMENT

    REC_ZCODE_DIR                       Directory of Z-code files.
    REC_DF_OPTIONS                      Extra dumb-frotz options.

=head1 AUTHOR

Glen Hutchings

=cut

require 5.000;
use Getopt::Std;

# Set defaults.
$options = "-Rmp -Rch1 -Rlt";
$options .= " " . $ENV{REC_DF_OPTIONS} if $ENV{REC_DF_OPTIONS};

@suffixes = (qw(z3 z4 z5 z6 z8));

# Parse arguments.
$0 =~ s-.*/--;
&getopts('c:g:ho:r:s:tz:') or die "Type `$0 -h' for help\n";
&usage if defined $opt_h;

@dirs = ('.');
push(@dirs, $opt_z) if $opt_z;
push(@dirs, $ENV{REC_ZCODE_DIR}) if $ENV{REC_ZCODE_DIR};

die "$0: no Z-code file specified\n" unless @ARGV > 0;

$zfile = shift;

if (-f $zfile) {
    $zpath = $zfile;
} else {
    FIND: foreach $dir (@dirs) {
	$zpath = $dir . "/" . $zfile;
	last FIND if -f $zpath;
	foreach $suf (@suffixes) {
	    $zpath = $dir . "/" . $zfile . "." . $suf;
	    last FIND if -f $zpath;
	}
    }
}

die "$0: Z-code file $zfile not found\n" unless -f $zpath;

if ($opt_r) {
    open(STDIN, $opt_r) or die "$0: can't open $opt_r: $!\n";
}

if ($opt_o) {
    open(STDOUT, "> $opt_o") or die "$0: can't open $opt_o: $!\n";
}

if ($opt_c) {
    open(CMD, $opt_c) or die "$0: can't open $opt_c: $!\n";

    while (<CMD>) {
	if      (/^cmd "(.*)"/) {
	    push(@cmds, $1);
	} elsif (/^print \/(.+)\//) {
	    push(@print, $1);
	} elsif (/^\s*$/ || /^\s*\#/) {
	    next;
	} else {
	    die "$0: $opt_c, line $.: invalid command: $_";
	}
    }

    close CMD;
}

$options .= " -s$opt_s" if $opt_s;

$tmpdir = $ENV{TMPDIR} || "/tmp";
$in = $tmpdir . "/r2s$$.in";
$rec = $tmpdir . "/r2s$$.rec";
$out = $tmpdir . "/r2s$$.out";
$sav = $tmpdir . "/r2s$$.sav";

# Create input.
open(REC, "> $rec") or die "$0: can't create recording file\n";
print REC while (<STDIN>);
close REC;

open(IN, "> $in") or die "$0: can't create input file\n";

print join("\n", @cmds), "\n" if @cmds > 0;

if ($opt_t) {
    print IN "SCRIPT\n";	# Request transcript.
    print IN "$out\n";		# Transcript file.
    print IN "LOOK\n";		# Show current location.
}

print IN "\\P\n";		# Request playback.
print IN "$rec\n";		# Recording file.
print IN "Y\n";			# Yes to more-prompts question.

if ($opt_g) {
    print IN "SAVE\n";		# Save game.
    print IN "$sav\n";		# Save filename.
}

if ($opt_t) {
    print IN "UNSCRIPT\n";	# End transcript.
}

print IN "Q\n";			# Quit at end.
print IN "Y\n";			# Yes, really.

close IN;

# Invoke dumb-frotz.
$cmd = "dumb-frotz $options $zpath < $in > ";
$cmd .= ($opt_t ? "/dev/null" : $out);
system $cmd;

# Create save file if required.
if ($opt_g) {
    system "mv $sav $opt_g 2> /dev/null";
    die "$0: can't create save file\n" unless -f $opt_g;
}

# Munge output.
unless (open(OUT, $out)) {
    &cleanup;
    die "$0: no output created\n";
}

while (<OUT>) {
    # Dumb-frotz status messages.
    next if /^More prompts/;
    next if /^Compression mode/;
    next if /^Line-type display/;

    # Garbage near start of playback.
    if (/Hot key -- Playback on/) {
	for (1 .. 3) {
	    $_ = <OUT>;
	}

	print ">";
	next;
    }

    # Print command-file regexps if required.
    if (@print) {
	foreach $re (@print) {
	    if (/$re/) {
		print STDERR "\nLine $.:\n\n";
		$print_count += 5;
		last;
	    }
	}
    }

    if ($print_count) {
	print STDERR "   ", $_;
	$print_count--;
    }

    # Pad out missing blank lines before prompts.
    print "\n" if !$blank && /^>/;
    $blank = (/^$/ ? 1 : 0);

    # Be sure final newline is present.
    chomp;
    print;
    print "\n";
}

close OUT;

# Er... that's it.
&cleanup;

# Clean up.
sub cleanup {
    unlink $in, $rec, $out, $sav;
}

# Give a usage message and exit.
sub usage {
    print "Usage: $0 [options] zcode-file\n";
    print "  -r file      specify recording file\n";
    print "  -s seed      set random number seed\n";
    print "  -c file      read commands from file\n";
    print "  -g file      save game to specified file\n";
    print "  -o file      write output to specified file\n";
    print "  -t           make transcript using 'script'\n";
    print "  -h           this help message\n";
    exit 0;
}
