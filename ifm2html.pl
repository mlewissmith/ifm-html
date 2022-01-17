#! /usr/bin/perl

=head1 NAME

ifm2html - ...

=head1 SYNOPSIS

Lorem ipsum dolor sit amet, consectetuer adipiscing elit.

=cut

use strict;
use warnings;
use feature qw(say);
use experimental qw(switch);
#no if $] >= 5.018, warnings => qw( experimental::smartmatch );

use Cwd qw(getcwd realpath);
use Data::Dumper; $Data::Dumper::Sortkeys = 1;
use File::Basename;
use Getopt::Long;
use HTML::Entities;
use Pod::Usage;


my %opt = ( "debug"       => 0,
            "quiet"       => 0,
            "format"      => "html",
            "ifmexe"      => "ifm",
            "transfigexe" => "fig2dev",
            "dotexe"      => "dot");

################################################################################

sub dprint {
    ## debug print
    ## usage:    dprint @list
    return unless $opt{debug};
    my ($_package, $_file, $_line) = caller;
    my @_c = caller(1);
    my $_subroutine = (defined $_c[3])?$_c[3]:$_package;
    @_ = ('#' x 80 ) unless @_;
    foreach my $arg (@_) {
        my @lines = split "\n", $arg;
        foreach my $l (@lines) {
            warn "${_file}($_line)(${_subroutine})\t$l\n";
        }
    }
}

sub qsay {
    return if $opt{quiet};
    say @_;
}

sub qprint {
    return if $opt{quiet};
    print @_;
}

sub stripwhite {
    my $arg = shift;
    $arg =~ s/^\s+//;
    $arg =~ s/\s+$//;
    return $arg;
}

sub heredoc {
    # my $text = heredoc(<<"TARGET");
    #   or
    # print heredoc(<<"TARGET");
    local $_ = shift;
    my ($white, $leader);  # common whitespace and common leading string
    if (/^\s*(?:([^\w\s]+)(\s*).*\n)(?:\s*\1\2?.*\n)+$/) {
        ($white, $leader) = ($2, quotemeta($1));
    } else {
        ($white, $leader) = (/^(\s+)/, '');
    }
    s/^\s*?$leader(?:$white)?//gm;
    return $_;
}

################################################################################


=head1 OPTIONS

=over 4

=item -options...

Lorem ipsum dolor sit amet, consectetuer adipiscing elit.

=cut

Getopt::Long::Configure(qw(no_ignore_case));
GetOptions(\%opt,
           'ifmexe=s',
           'basename=s',
           'format=s',
           'debug|D!',
           'quiet|q!',
           'help|h'          => sub { pod2usage(-verbose => 1); },
           'man'             => sub { pod2usage(-verbose => 2, -noperldoc => !$>); },
           ) or pod2usage(-verbose => 0);

pod2usage(-verbose => 0) unless ($ARGV[0]);
unless (-e $ARGV[0]) {
    die "$ARGV[0]: $!\n";
}

my $inputfile = $ARGV[0];
my $ifmexe = $opt{ifmexe};

$opt{basename} = basename($inputfile, ".ifm") unless $opt{basename};

my %ifmdata;

########################################
# read the map sections; populate $ifmdata{maps}
{
    my $ifmcmd = qq($ifmexe --show maps $inputfile);
    dprint $ifmcmd;
    if (open IFM, "$ifmcmd |") {
        # discard header line
        <IFM>;
        while (<IFM>) {
            my $line = $_;
            chomp $line;
            my @data = split(/\s+/, $line, 5);
            $ifmdata{maps}{$data[0]}{count} = $data[1];
            $ifmdata{maps}{$data[0]}{width} = $data[2];
            $ifmdata{maps}{$data[0]}{height} = $data[3];
            $ifmdata{maps}{$data[0]}{name} = $data[4];
        }
        close IFM;
    } else {
        die "$ifmcmd: $!";
    }
}

########################################
# read the raw maps data; populate $ifmdata{rooms}
# ifm -f raw -m1
{
    my $sortorder = 0;
    for my $map (sort keys %{$ifmdata{maps}}) {
        my $ifmcmd = qq($ifmexe -f raw -m${map} $inputfile);
        dprint $ifmcmd;
        if (open IFM, "$ifmcmd |") {
            my $current_room;
            my $current_link;
            while (<IFM>) {
                chomp;
                my $line = $_;
                next unless $line;
                my ($field, $value) = split(/\s*:\s*/, $line, 2);

                given($field) {
                    when("title") {
                        $ifmdata{title} = $value;
                    }

                    when("room") {
                        $current_room = $value;
                        $ifmdata{rooms}{$current_room}{map} = $map;
                        $ifmdata{rooms}{$current_room}{sortorder} = $sortorder++;
                    }
                    when("name") {
                        $ifmdata{rooms}{$current_room}{name} = $value;
                    }

                    when("note") {
                        push @{$ifmdata{rooms}{$current_room}{note}}, $value;
                    }

                    when(/(link)|(join)/) {
                        $current_link = $value;
                        my ($this_room, $that_room) = split(/\s+/,$current_link);
                        $ifmdata{rooms}{$this_room}{link}{$that_room}++;
                    }
                    when("oneway") {
                        my ($this_room, $that_room) = split(/\s+/,$current_link);
                        $ifmdata{rooms}{$this_room}{link}{$that_room}--;
                    }

                    when(/^(exit)|(go)|(height)|(lpos)|(rpos)|(section)|(width)$/) {
                        # do nothing
                    }

                    default {
                        dprint "Unknown map field $field";
                    }
                }
            }
            close IFM;
            # reverse links and joins
            for my $this_room (sort keys %{$ifmdata{rooms}}) {
                for my $that_room (sort keys %{$ifmdata{rooms}{$this_room}{link}}) {
                    if ($ifmdata{rooms}{$this_room}{link}{$that_room}) {
                        $ifmdata{rooms}{$that_room}{link}{$this_room}++;
                    }
                }
            }
        } else {
            die "$ifmcmd: $!";
        }
    }
}

########################################
# read the raw tasks data; populate $ifmdata{tasks}
# ifm -f raw -t
{
    my $sortorder = 0;
    my $ifmcmd = qq($ifmexe -f raw -t $inputfile);
    dprint $ifmcmd;
    if (open IFM, "$ifmcmd |") {
        my $current_task;
        while (<IFM>) {
            chomp;
            my $line = $_;
            next unless $line;
            my ($field, $value) = split(/\s*:\s*/, $line, 2);

            given($field) {
                when("task") {
                    $current_task = $value;
                    $ifmdata{tasks}{$current_task}{sortorder} = $sortorder++;
                }

                when(/^(type)|(name)|(score)$/) {
                    $ifmdata{tasks}{$current_task}{$field} = $value;
                }

                when("room") {
                    $ifmdata{tasks}{$current_task}{$field} = $value;
                    push @{$ifmdata{rooms}{$value}{tasks}}, $current_task;
                }

                when("note") {
                    push @{$ifmdata{tasks}{$current_task}{note}}, $value;
                }

                when("cmd") {
                    push @{$ifmdata{tasks}{$current_task}{cmd}}, $value;
                }

                when("get") {
                    $ifmdata{tasks}{$current_task}{get} = $value;
                }

                when("give") {
                    push @{$ifmdata{tasks}{$current_task}{give}}, $value;
                }

                when("tag") {
                    # do nothing
                }

                default {
                    dprint "Unknown task field $field";
                }
            }
        }
        close IFM;
    } else {
        die "$ifmcmd: $!";
    }
}

########################################
# read the raw items data; populate $ifmdata{items}
# *AFTER* parsing tasks.  Some of the tasks get optimised out...
{
    my $sortorder = 0;
    my $ifmcmd = qq($ifmexe -f raw -i $inputfile);
    dprint $ifmcmd;
    if (open IFM, "$ifmcmd |") {
        my $current_item;
        while (<IFM>) {
            chomp;
            my $line = $_;
            next unless $line;
            my ($field, $value) = split(/\s*:\s*/, $line, 2);

            given($field) {
                when("item") {
                    $current_item = $value;
                    $ifmdata{items}{$current_item}{sortorder} = $sortorder++;
                }
                when(/^(name)|(score)|(hidden)$/) {
                    $ifmdata{items}{$current_item}{$field} = $value;
                }
                when("room") {
                    $ifmdata{items}{$current_item}{$field} = $value;
                    push @{$ifmdata{rooms}{$value}{items}}, $current_item;
                }

                when("note") {
                    push @{$ifmdata{items}{$current_item}{$field}}, $value;
                }
                when("after") {
                    push @{$ifmdata{items}{$current_item}{$field}}, $value;
                    if (defined $ifmdata{tasks}{$value}) {
                        push @{$ifmdata{tasks}{$value}{toget}}, $current_item;
                    }
                }
                when("needed") {
                    push @{$ifmdata{items}{$current_item}{$field}}, $value;
                    if (defined $ifmdata{tasks}{$value}) {
                        push @{$ifmdata{tasks}{$value}{needs}}, $current_item;
                    }
                }

                when("move") {
                    my ($move_from, $move_to) = split(/\s+/,$value);
                    push @{$ifmdata{items}{$current_item}{move}{$move_from}}, $move_to;
                    push @{$ifmdata{rooms}{$move_from}{need}{$move_to}}, $current_item;
                }

                when(/^(tag)|(leave)|(enter)$/) {
                    # do nothing
                }

                default {
                    dprint "Unknown item field $field";
                }
            }
        }
        close IFM;
    } else {
        die "$ifmcmd: $!";
    }
}



#dprint Dumper \%ifmdata;

################################################################################
### now display output

my $maps = \%{$ifmdata{maps}};
my $rooms = \%{$ifmdata{rooms}};
my $tasks = \%{$ifmdata{tasks}};
my $items = \%{$ifmdata{items}};

if ($opt{format} eq "html") {
# PAGES:
# - index
# - rooms : rooms ; items + tasks
# - items : list of items; link to room, required/dependent items + tasks
# - tasks : list of tasks (no moves) ; link to room, required/dependent items + tasks
#           graphviz/dot taskdeps

    my $index_html = "$opt{basename}-index.html";
    my $rooms_html = "$opt{basename}-rooms.html";
    my $tasks_html = "$opt{basename}-tasks.html";
    my $items_html = "$opt{basename}-items.html";
    my $taskdeps_svg = "$opt{basename}-taskdeps.svg";

    my $html_css = heredoc(<<"##__HTML__##");
# <style type="text/css">
# body { text-align: left; }
# dt { margin-top: 1em; font-weight: bold; }
# dd { margin-left: 0em; text-indent: 2em; }
# .room { font-style: normal; @{[ $opt{debug}?"font-variant: small-caps;":"" ]} }
# .task { font-style: normal; font-family: monospace; }
# .item { font-style: italic; }
# .debug { font-size: xx-small; font-weight: normal; }
# </style>
##__HTML__##

########################################
## SVG MAPS
    my $transfigexe = $opt{transfigexe};
    for my $m (sort { $a <=> $b } keys %{$maps}) {
        my $map_svg = "$opt{basename}-map${m}.svg";
        dprint "$map_svg";
        my $ifmcmd = qq($ifmexe -map=$m -format fig -s show_map_title=false $inputfile | $transfigexe -L svg);
        #dprint "$ifmcmd";
        system("$ifmcmd > $map_svg") == 0 or die "$ifmcmd: $!";
    }

########################################
## SVG TASKDEPS
    dprint "$taskdeps_svg";
    {
        my $dot = $opt{dotexe};
        my $ifmcmd = qq($ifmexe -f dot -t $inputfile);
        my $dotcmd = qq($dot -Tsvg -o $taskdeps_svg);
        if ( open(IFM, "$ifmcmd |") && open(DOT, "| $dotcmd") ) {
            dprint "$ifmcmd";
            dprint "$dotcmd";
            while (<IFM>) {
                my $line = $_;
                if ( ($line =~ m/^\s*graph/) or
                     ($line =~ m/^\s*rankdir/) or
                     ($line =~ m/^\s*rotate/) or
                     ($line =~ m/^\s*concentrate/) ) {
                    $line = "//$line";
                }
                dprint $line;
                print DOT $line;
            }
            close IFM or die "$!";
            close DOT or die "$!";
        } else {
            die "$!";
        }
    }

########################################
### INDEX PAGE
    dprint "$index_html";
    if (open HTML, ">$index_html") {
        print HTML heredoc(<<"##__HTML__##");
# <!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01//EN" "http://www.w3.org/TR/html4/strict.dtd">
# <html>
# <head>
# <meta http-equiv="Content-type" content="text/html;charset=ISO-8859-1">
# <title>@{[encode_entities($ifmdata{title})]}</title>
# @{[split(/\n/,$html_css)]}
# </head>
# <body>
# <h1>@{[encode_entities($ifmdata{title})]}</h1>
# <ul>
# <li><a href="$rooms_html">Rooms</a>
# <ul>
##__HTML__##
        for my $m (sort { $a <=> $b } keys %{$maps}) {
            say HTML qq(<li><a href="${rooms_html}#map${m}">) . encode_entities($maps->{$m}{name}) . qq(</a>);
        }
        print HTML heredoc(<<"##__HTML__##");
# </ul>
# <li><a href="$items_html">Items</a>
# <li><a href="$tasks_html">Tasks</a>
# </ul>
# </body>
# </html>
##__HTML__##
        close HTML;
    } else {
        die "$index_html: $!";
    }

########################################
### ROOMS PAGE
    dprint "$rooms_html";
    if (open HTML, ">$rooms_html") {
        print HTML heredoc(<<"##__HTML__##");
# <!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01//EN" "http://www.w3.org/TR/html4/strict.dtd">
# <html>
# <head>
# <meta http-equiv="Content-type" content="text/html;charset=ISO-8859-1">
# <title>@{[encode_entities($ifmdata{title})]} : ROOMS</title>
# @{[split(/\n/,$html_css)]}
# </head>
# <body>
# <h1><a href="$index_html">Rooms</a></h1>
##__HTML__##


            print HTML heredoc(<<"##__HTML__##");
# <!-- toc -->
# <ul>
##__HTML__##
        for my $m (sort { $a <=> $b } keys %{$maps}) {
            print HTML heredoc(<<"##__HTML__##");
# <li>
# <a href="#map${m}">@{[ encode_entities($maps->{$m}{name}) ]}</a>
# </li>
##__HTML__##
        }
            print HTML heredoc(<<"##__HTML__##");
# </ul>
# <!-- toc -->
##__HTML__##



        for my $m (sort { $a <=> $b } keys %{$maps}) {
            print HTML heredoc(<<"##__HTML__##");
# <!-- BEGIN map $m -->
# <h2><a name="map${m}" id="map${m}" href="$rooms_html">@{[encode_entities($maps->{$m}{name})]}</a></h2>
# <div><object type="image/svg+xml" data="$opt{basename}-map${m}.svg">[$opt{basename}-map${m}.svg]</object></div>
# <dl>
##__HTML__##

            for my $r (sort { $rooms->{$a}{name} cmp $rooms->{$b}{name} } keys %{$rooms}) {
                next unless ($rooms->{$r}{map} == $m);
                print HTML heredoc(<<"##__HTML__##");
# <!-- BEGIN room $m:$r -->
# <dt class="room">
# <a name="room${r}" id="room${r}" href="#map${m}">@{[encode_entities($rooms->{$r}{name})]}</a>@{[$opt{debug}?qq(<span class="debug">[$r]</span>):qq()]}
# </dt>
##__HTML__##

                ## exits (links)
                my @links = (sort { $a <=> $b } keys %{$rooms->{$r}{link}});
                if (@links) {
                    my @htmlary;
                    for my $l (@links) {
                        my $htmlstr = "";
                        if ($rooms->{$l}{map} != $m) {
                            $htmlstr = qq(<a href="#room${l}">)
                                . qq(<span class="room">)
                                . encode_entities($maps->{$rooms->{$l}{map}}{name})
                                . qq(:)
                                . encode_entities($rooms->{$l}{name})
                                . qq(</span>)
                                . qq(</a>);
                        } else {
                            $htmlstr = qq(<a href="#room${l}">)
                                . qq(<span class="room">)
                                . encode_entities($rooms->{$l}{name})
                                . qq(</span>)
                                . qq(</a>);
                        }
                        $htmlstr .= $opt{debug}?qq(<span class="debug">[$l]</span>):qq();
                        if (defined $rooms->{$r}{need}{$l}) {
                            my @itemary;
                            $htmlstr .= " (requires ";
                            for my $i (@{$rooms->{$r}{need}{$l}}) {
                                push @itemary, qq(<a href="$items_html#item${i}">)
                                    . qq(<span class="item">)
                                    . encode_entities($items->{$i}{name})
                                    . qq(</span>)
                                    . qq(</a>)
                                    . ($opt{debug}?qq(<span class="debug">[$i]</span>):qq());
                            }
                            $htmlstr .= join(",", @itemary);
                            $htmlstr .= ")";

                        }
                        push @htmlary, $htmlstr;
                    }
                    print HTML heredoc(<<"##__HTML__##");
# <!-- BEGIN exit $m:$r -->
# <dd>Exits: @{[join(", ", @htmlary)]}.</dd>
# <!-- END exit $m:$r -->
##__HTML__##
                }

                ## items
                if ($rooms->{$r}{items}) {
                    my @items = (@{$rooms->{$r}{items}});
                    my @htmlary;
                    for my $i (@items) {
                        my $htmlstr = qq(<a href="$items_html#item${i}">)
                            . qq(<span class="item">)
                            . (encode_entities($items->{$i}{name}))
                            . qq(</span>)
                            . qq(</a>);
                        $htmlstr .= $opt{debug}?qq(<span class="debug">[$i]</span>):qq();
                        if ($items->{$i}{hidden}) {
                            $htmlstr .= qq( (hidden));
                        }
                        push @htmlary, $htmlstr;
                    }
                    if (@htmlary) {
                        print HTML heredoc(<<"##__HTML__##");
# <!-- BEGIN items $m:$r -->
# <dd>Items: @{[join(", ", @htmlary)]}.</dd>
# <!-- END items $m:$r -->
##__HTML__##
                    }
                }

                ## tasks
                if ($rooms->{$r}{tasks}) {
                    my @tasks = (@{$rooms->{$r}{tasks}});
                    my @htmlary;
                    for my $t (@tasks) {
                        #next if ($tasks->{$t}{type} eq "MOVE");
                        if ($tasks->{$t}{type} eq "USER") {
                            push @htmlary, qq(<a href="$tasks_html#task${t}">)
                                . qq(<span class="task">)
                                . uc(encode_entities($tasks->{$t}{name}))
                                . qq(</span>)
                                . qq(</a>)
                                . ($opt{debug}?qq(<span class="debug">[$t]</span>):qq());
                        }
                    }
                    if (@htmlary) {
                        print HTML heredoc(<<"##__HTML__##");
# <!-- BEGIN tasks $m:$r -->
# <dd>Tasks: @{[join(", ", @htmlary)]}.</dd>
# <!-- END tasks $m:$r -->
##__HTML__##
                    }
                }

                ## notes
                if (defined $rooms->{$r}{note}) {
                    my @htmlary;
                    for my $n (@{$rooms->{$r}{note}}) {
                        my $n_html = ucfirst(encode_entities($n));
                        $n_html =~ s/#IMG\{(.+?)\}/<img src="$1">/g;
                        push @htmlary, $n_html;
                    }
                    print HTML heredoc(<<"##__HTML__##");
# <!-- BEGIN NOTES $m:$r -->
# <dd>@{[join("</dd><dd>", @htmlary)]}</dd>
# <!-- END NOTES $m:$r -->
##__HTML__##
                }

                print HTML heredoc(<<"##__HTML__##");
# <!-- END room $m:$r -->
#
##__HTML__##
            }
            print HTML heredoc(<<"##__HTML__##");
# </dl>
# <!-- END map $m -->
#
##__HTML__##
        }

        print HTML heredoc(<<"##__HTML__##");
# </body>
# </html>
##__HTML__##
        close HTML;
    } else {
        die "$rooms_html: $!";
    }


########################################
### TASKS page
    dprint "$tasks_html";
    if (open HTML, ">$tasks_html") {
        print HTML heredoc(<<"##__HTML__##");
# <!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01//EN" "http://www.w3.org/TR/html4/strict.dtd">
# <html>
# <head>
# <meta http-equiv="Content-type" content="text/html;charset=ISO-8859-1">
# <title>@{[encode_entities($ifmdata{title})]} : TASKS</title>
# @{[split(/\n/,$html_css)]}
# </head>
# <body>
# <h1><a href="$index_html">Tasks</a></h1>
# <div><object type="image/svg+xml" data="${taskdeps_svg}" width="100%">[${taskdeps_svg}]</object></div>
# <dl>
##__HTML__##

        for my $t (sort { $tasks->{$a}{sortorder} <=> $tasks->{$b}{sortorder} } keys %{$tasks}) {
            next if ($tasks->{$t}{type} eq "MOVE");

            my $itemhref = "";
            if (defined $tasks->{$t}{get}) {
                my $itemid = $tasks->{$t}{get};
                $itemhref = qq(href="$items_html#item$itemid");
            }

            print HTML heredoc(<<"##__HTML__##");
# <!-- BEGIN task $t -->
# <dt class="task">
# <a $itemhref name="task${t}" id="task${t}">@{[uc(encode_entities($tasks->{$t}{name}))]}</a>@{[$opt{debug}?qq(<span class="debug">[$t]</span>):qq()]}
# </dt>
# <dd>
##__HTML__##

            ## room
            if (defined $tasks->{$t}{room}) {
                print HTML heredoc(<<"##__HTML__##");
# <!-- BEGIN room $t -->
# In
# <a href="$rooms_html#room$tasks->{$t}{room}"><span class="room">@{[ encode_entities($rooms->{$tasks->{$t}{room}}{name}) ]}</span></a>@{[ $opt{debug}?qq(<span class="debug">[$tasks->{$t}{room}]</span>):qq() ]}.&nbsp;
# <!-- END room $t -->
##__HTML__##
            }

            ## commands
            if (defined $tasks->{$t}{cmd}) {
                my @htmlary;
                for my $n (@{$tasks->{$t}{cmd}}) {
                    push @htmlary, qq(<span class="task">)
                        . uc(encode_entities($n))
                        . qq(</span>);
                }
                print HTML heredoc(<<"##__HTML__##");
# <!-- BEGIN COMMANDS $t -->
# @{[join("; ", @htmlary)]}.&nbsp;
# <!-- END COMMANDS $t -->
##__HTML__##
            }

            ## needed items
            if (defined $tasks->{$t}{needs}) {
                my @htmlary;
                my %done; # wierdness...
                for my $n (@{$tasks->{$t}{needs}}) {
                    if (not $done{$n}) {
                        $done{$n}++;
                        push @htmlary, qq(<span class="item">)
                            . qq(<a href="$items_html#item${n}">)
                            . encode_entities($items->{$n}{name})
                            . qq(</a>)
                            . qq(</span>)
                            . ( $opt{debug}?qq(<span class="debug">[$n]</span>):qq() );
                    }
                }
                print HTML heredoc(<<"##__HTML__##");
# <!-- BEGIN NEEDS $t -->
# Requires
# @{[join(", ", @htmlary)]}.&nbsp;
# <!-- END NEEDS $t -->
##__HTML__##
            }

            ## to get items
            if (defined $tasks->{$t}{toget}) {
                my @htmlary;
                my %done; # wierdness...
                for my $n (@{$tasks->{$t}{toget}}) {
                    if (not $done{$n}) {
                        $done{$n}++;
                        push @htmlary, qq(<span class="item">)
                            . qq(<a href="$items_html#item${n}">)
                            . encode_entities($items->{$n}{name})
                            . qq(</a>)
                            . qq(</span>)
                            . ( $opt{debug}?qq(<span class="debug">[$n]</span>):qq() );
                    }
                }
                print HTML heredoc(<<"##__HTML__##");
# <!-- BEGIN TO-GET $t -->
# Allows you to get
# @{[join(", ", @htmlary)]}.&nbsp;
# <!-- END TO-GET $t -->
##__HTML__##
            }

            ## score
            if (defined $tasks->{$t}{score}) {
                print HTML heredoc(<<"##__HTML__##");
# <!-- BEGIN SCORE $t -->
# Scores @{[encode_entities($tasks->{$t}{score})]}.&nbsp;
# <!-- END SCORE $t -->
##__HTML__##
            }

            ## notes
            if (defined $tasks->{$t}{note}) {
                my @htmlary;
                for my $n (@{$tasks->{$t}{note}}) {
                    my $n_html = ucfirst(encode_entities($n));
                    $n_html =~ s/#IMG\{(.+?)\}/<img src="$1">/g;
                    push @htmlary, $n_html;
                }
                print HTML heredoc(<<"##__HTML__##");
# <!-- BEGIN NOTES $t -->
# </dd><dd>
# @{[join("</dd><dd>", @htmlary)]}
# <!-- END NOTES $t -->
##__HTML__##
            }

            print HTML heredoc(<<"##__HTML__##");
# </dd>
# <!-- END task $t -->
#
##__HTML__##

        }

        print HTML heredoc(<<"##__HTML__##");
# </dl>
# <div><object type="image/svg+xml" data="${taskdeps_svg}" width="100%">[${taskdeps_svg}]</object></div>
# </body>
# </html>
##__HTML__##
        close HTML;
    } else {
        die "$tasks_html: $!";
    }


########################################
### ITEMS page
    dprint "$items_html";
    if (open HTML, ">$items_html") {
        print HTML heredoc(<<"##__HTML__##");
# <!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01//EN" "http://www.w3.org/TR/html4/strict.dtd">
# <html>
# <head>
# <meta http-equiv="Content-type" content="text/html;charset=ISO-8859-1">
# <title>@{[encode_entities($ifmdata{title})]} : ITEMS</title>
# @{[split(/\n/,$html_css)]}
# </head>
# <body>
# <h1><a href="$index_html">Items</a></h1>
# <dl>
##__HTML__##

        for my $i (sort { $items->{$a}{name} cmp $items->{$b}{name} } keys %{$items}) {
            print HTML heredoc(<<"##__HTML__##");
# <!-- BEGIN item $i -->
# <dt class="item">
# <a name="item${i}" id="item${i}">@{[encode_entities($items->{$i}{name})]}</a>@{[$opt{debug}?qq(<span class="debug">[$i]</span>):qq()]}
# </dt>
# <dd>
##__HTML__##

            ## room
            if (defined $items->{$i}{room}) {
                my $htmlstr = $items->{$i}{hidden}?"Hidden":"Found";
                print HTML heredoc(<<"##__HTML__##");
# <!-- BEGIN ROOM $i -->
# $htmlstr in
# <a href="$rooms_html#room$items->{$i}{room}"><span class="room">@{[ encode_entities($rooms->{$items->{$i}{room}}{name}) ]}</span></a>@{[ $opt{debug}?qq(<span class="debug">[$items->{$i}{room}]</span>):qq() ]}.&nbsp;
# <!-- END ROOM $i -->
##__HTML__##

            } else {
                print HTML heredoc(<<"##__HTML__##");
# <!-- BEGIN ROOM $i -->
# Carried at start of game.&nbsp;
# <!-- END ROOM $i -->
##__HTML__##
            }

            ## obtained after task
            if (defined $items->{$i}{after}) {
                my @htmlary;
                my %done; # wierdness...
                for my $t ( sort { (defined $tasks->{$a}?$tasks->{$a}{sortorder}:0) <=> (defined $tasks->{$b}?$tasks->{$b}{sortorder}:0) } @{$items->{$i}{after}} ) {
                    if (defined $tasks->{$t} and not $done{$t}) {
                        $done{$t}++;
                        # recall, some tasks get optimised out..
                        push(@htmlary, qq(<a href="$tasks_html#task${t}">)
                             . qq(<span class="task">)
                             . uc(encode_entities($tasks->{$t}{name}))
                             . qq(</span>)
                             . qq(</a>)
                             . ($opt{debug}?qq(<span class="debug">[$t]</span>):qq()) );
                    }
                }
                if (@htmlary) {
                    print HTML heredoc(<<"##__HTML__##");
# <!-- BEGIN after-tasks -->
# Obtained after
# @{[ join(", ", @htmlary) ]}.&nbsp;
# <!-- END after-tasks -->
##__HTML__##
                }
            }

            ## needed for task
            if (defined $items->{$i}{needed}) {
                my @htmlary;
                my %done; # wierdness...
                for my $t ( sort { (defined $tasks->{$a}?$tasks->{$a}{sortorder}:0) <=> (defined $tasks->{$b}?$tasks->{$b}{sortorder}:0) } @{$items->{$i}{needed}} ) {
                    if (defined $tasks->{$t} and not $done{$t}) {
                        $done{$t}++;
                        # recall, some tasks get optimised out..
                        push(@htmlary, qq(<a href="$tasks_html#task${t}">)
                             . qq(<span class="task">)
                             . uc(encode_entities($tasks->{$t}{name}))
                             . qq(</span>)
                             . qq(</a>)
                             . ($opt{debug}?qq(<span class="debug">[$t]</span>):qq()) );
                    }
                }
                if (@htmlary) {
                    print HTML heredoc(<<"##__HTML__##");
# <!-- BEGIN after-tasks -->
# Needed for
# @{[ join(", ", @htmlary) ]}.&nbsp;
# <!-- END after-tasks -->
##__HTML__##
                }
            }

            ## needed to move...
            if (defined $items->{$i}{move}) {
                my @htmlary;
                for my $room_from (sort { $rooms->{$a}{name} cmp $rooms->{$b}{name} } keys %{$items->{$i}{move}}) {
                    for my $room_to (@{$items->{$i}{move}{$room_from}}) {
                        push @htmlary, qq(from )
                            . qq(<span class="room">)
                            . qq(<a href="$rooms_html#room${room_from}">)
                            . encode_entities($rooms->{$room_from}{name})
                            . qq(</a>)
                            . qq(</span>)
                            . ($opt{debug}?qq(<span class="debug">[$room_from]</span>):qq())
                            . qq( to )
                            . qq(<span class="room">)
                            . qq(<a href="$rooms_html#room${room_to}">)
                            . encode_entities($rooms->{$room_to}{name})
                            . qq(</a>)
                            . qq(</span>)
                            . ($opt{debug}?qq(<span class="debug">[$room_from]</span>):qq());
                    }
                }
                print HTML heredoc(<<"##__HTML__##");
# <!-- BEGIN move -->
# Needed to move
# @{[ join(", ", @htmlary) ]}.
# <!-- END move -->
##__HTML__##

            }

            ## score
            if (defined $items->{$i}{score}) {
                print HTML heredoc(<<"##__HTML__##");
# <!-- BEGIN SCORE $i -->
# Scores @{[encode_entities($items->{$i}{score})]}.
# <!-- END SCORE $i -->
##__HTML__##
            }

            ## notes
            if (defined $items->{$i}{note}) {
                my @htmlary;
                for my $n (@{$items->{$i}{note}}) {
                    my $n_html = ucfirst(encode_entities($n));
                    $n_html =~ s/#IMG\{(.+?)\}/<img src="$1">/g;
                    push @htmlary, $n_html;
                }
                print HTML heredoc(<<"##__HTML__##");
# <!-- BEGIN NOTES $i -->
# </dd><dd>@{[join("</dd><dd>", @htmlary)]}
# <!-- END NOTES $i -->
##__HTML__##
            }

            print HTML heredoc(<<"##__HTML__##");
# </dd>
# <!-- END item $i -->
#
##__HTML__##

        }

        print HTML heredoc(<<"##__HTML__##");
# </dl>
# </body>
# </html>
##__HTML__##
        close HTML;
    } else {
        die "$items_html: $!";
    }
}


################################################################################
if ($opt{format} eq "debug") {
    print Dumper \%ifmdata
}

if ($opt{format} eq "xml") {
    require XML::Simple;
    XML::Simple->import(":strict");
    print XMLout(\%ifmdata, KeyAttr => { maps => 'id', rooms => 'id', tasks => 'id', items => 'id' });
}

__END__



# ebook-convert infile.html outfile.mobi --output-profile tablet --mobi-keep-original-images
