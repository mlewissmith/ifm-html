# This file is part of IFM (Interactive Fiction Mapper), copyright (C)
# Glenn Hutchings 1997-2008.

# IFM comes with ABSOLUTELY NO WARRANTY.  This is free software, and you
# are welcome to redistribute it under certain conditions; see the file
# COPYING for details.

# tkifm -- Tcl/Tk interface to IFM

# Whether to allow editing.
set ifm(edit) 1

# Edit window dimensions.
set ifm(editwidth)  80
set ifm(editheight) 24

# Editing font.
set ifm(editfont) {Courier 12 bold}

# Edit window colours.
set ifm(editforeground) black
set ifm(editbackground) wheat

# Item list window dimensions.
set ifm(itemwidth)  50
set ifm(itemheight) 30

# Task list (brief) window dimensions.
set ifm(taskwidth)  50
set ifm(taskheight) 30

# Task list (verbose) window dimensions.
set ifm(verbosewidth)  80
set ifm(verboseheight) 30

# Variable window dimensions.
set ifm(varswidth)  50
set ifm(varsheight) 30

# Text window font.
set ifm(textfont) {Times 12 bold}

# Text window colours.
set ifm(textforeground) black
set ifm(textbackground) wheat

# Whether to allow tearoff menus.
set ifm(tearoff) 1

# IFM exec variables.
set ifm(mapcmd)     {ifm -map -format tk}
set ifm(itemcmd)    {ifm -nowarn -items -format tk}
set ifm(taskcmd)    {ifm -nowarn -tasks -format tk}
set ifm(verbosecmd) {ifm -nowarn -style verbose -format tk}
set ifm(pscmd)      {ifm -nowarn -map -format ps}
set ifm(figcmd)     {ifm -nowarn -map -format fig}
set ifm(varscmd)    {ifm -nowarn -show vars}
set ifm(pathcmd)    {ifm -nowarn -show path}
set ifm(aboutcmd)   {ifm -nowarn -version}

# Syntax highlighting variables.
set ifm(syntaxcomments)     firebrick
set ifm(syntaxstrings)      grey40
set ifm(syntaxstructure)    blue
set ifm(syntaxdirections)   darkgoldenrod
set ifm(syntaxspecial)      cadetblue
set ifm(syntaxbuiltin)      forestgreen
set ifm(syntaxkeyword)      royalblue
set ifm(syntaxpreprocessor) purple

# Internal variables.
set ifm(untitled)      "untitled.ifm"
set ifm(ifmfiles)      {{"IFM files" {.ifm}} {"All files" *}}
set ifm(psfiles)       {{"PostScript files" {.ps}} {"All files" *}}
set ifm(figfiles)      {{"Fig files" {.fig}} {"All files" *}}
set ifm(roomitemratio) 0.5
set ifm(busycursor)    watch

# Room style variable names.
set ifm(roomvars) {
    room_text_fontdef room_colour room_text_colour room_border_colour
    room_border_width room_shadow_xoff room_shadow_yoff room_shadow_colour
    room_exit_colour room_exit_width show_items item_text_fontdef
    item_text_colour
}

# Link style variable names.
set ifm(linkvars) {
    link_colour link_arrow_size link_text_colour link_text_fontdef
    link_line_width link_updown_string link_inout_string link_spline
}

# Set up the main window.
proc MainWindow {} {
    global ifm

    wm protocol . WM_DELETE_WINDOW Quit

    set m .menu
    menu $m

    # File options.
    set c $m.file
    menu $c -tearoff $ifm(tearoff)
    $m add cascade -label "File" -menu $c -underline 0

    if $ifm(edit) {
	$c add command -label "New" -command New -underline 0
    }

    $c add command -label "Open..." -command Open -underline 0

    if $ifm(edit) {
	$c add command -label "Save"       -command Save   -underline 0
	$c add command -label "Save As..." -command SaveAs -underline 5
    }

    $c add separator
    $c add command -label "Export PostScript..." -command ExportPS -underline 7
    $c add command -label "Export Fig..." -command ExportFIG -underline 0

    $c add separator
    $c add command -label "Redraw"     -command Redraw -underline 0

    $c add separator
    $c add command -label "Quit"       -command Quit   -underline 0

    # Map options.
    set c $m.sect
    menu $c -tearoff $ifm(tearoff)
    $m add cascade -label "Map" -menu $c -underline 0
    set ifm(mapmenu) $c

    # Item options.
    set c $m.items
    menu $c -tearoff $ifm(tearoff)
    $m add cascade -label "Items" -menu $c -underline 0
    $c add command -label "Item list" -command ShowItems -underline 0

    # Task options.
    set c $m.tasks
    menu $c -tearoff $ifm(tearoff)
    $m add cascade -label "Tasks" -menu $c -underline 0
    $c add command -label "Task list (brief)" -command ShowTasks -underline 0
    $c add command -label "Task list (verbose)" -command ShowVerbose -underline 0

    # Show options.
    set c $m.show
    menu $c -tearoff $ifm(tearoff)
    $m add cascade -label "Show" -menu $c -underline 0
    $c add command -label "Variables" -command ShowVars -underline 0
    $c add command -label "Search path" -command ShowPath -underline 0

    # Help options.
    set c $m.help
    menu $c -tearoff $ifm(tearoff)
    $m add cascade -label "Help" -menu $c -underline 0
    $c add command -label "About" -command About -underline 0

    # Configure menu.
    . configure -menu $m

    # Set up editing box.
    frame .edit
    set t .edit.text
    set s .edit.scroll
    set ifm(text) $t

    ctext $t -yscrollcommand "$s set" -setgrid true \
	-width $ifm(editwidth) -height $ifm(editheight) \
	-wrap word -font $ifm(editfont) -fg $ifm(editforeground) \
	-bg $ifm(editbackground) -insertofftime 0 \
	-insertbackground red

    scrollbar $s -command "$t yview"

    pack .edit -expand yes -fill both
    pack $s -side right -fill y
    pack $t -expand yes -fill both

    # Set up highlighting.
    ctext::addHighlightClass .edit.text preprocessor \
        $ifm(syntaxpreprocessor) {%include %define %defeval %if %ifdef %undef \
				      %ifndef %ifeq %ifneq %else %endif %eval \
				      %1 %2 %3 %4 %5 %6 %7 %8 %9 %exec defined}

    ctext::addHighlightClass .edit.text structure $ifm(syntaxstructure) \
	{room link}
    ctext::addHighlightClass .edit.text directions  $ifm(syntaxdirections) \
	{n north ne northeast e east se southeast s south sw \
	     southwest w west nw northwest}
    ctext::addHighlightClass .edit.text special  $ifm(syntaxspecial) \
        {endstyle style title map start finish safe require}
    ctext::addHighlightClass .edit.text builtin  $ifm(syntaxbuiltin) \
	{all any it last none undef}
    ctext::addHighlightClass .edit.text keyword  $ifm(syntaxkeyword) \
        {after before cmd d do down dir drop except exit follow \
	     from get give go goto hidden ignore in item join keep \
	     leave length lose lost need nodrop nolink nopath note \
	     oneway out score tag task to u up until with}

    # This is where the syntax highlighting is defined
    ctext::addHighlightClassForRegexp .edit.text strings \
	$ifm(syntaxstrings) {\"([^\\\"]|(\\.))*\"}
    ctext::addHighlightClassForRegexp .edit.text comments \
	$ifm(syntaxcomments) {#[^\n]*}

    if $ifm(edit) {
	focus $ifm(text)
    }
}

# Draw a map section.
proc DrawMap {num} {
    global ifm rooms links exits
    global sectnum

    if [NeedSave] return

    if {$num > $sectnum} {
	Message "Map section no longer exists!"
	return
    }

    Busy

    # Get attributes.
    set sect sect$num
    set title [Get $sect title]
    set xlen [Get $sect xlen]
    set ylen [Get $sect ylen]

    # Calculate dimensions.
    set width [expr $ifm(room_size) * $xlen]
    set height [expr $ifm(room_size) * $ylen]
    set xgap [expr ($ifm(room_size) * (1 - $ifm(room_width))) / 2]
    set ygap [expr ($ifm(room_size) * (1 - $ifm(room_height))) / 2]

    set map_canvas_width [expr $ifm(map_canvas_width) * $ifm(room_size)]
    if {$map_canvas_width > $width} {set map_canvas_width $width}
    set map_canvas_height [expr $ifm(map_canvas_height) * $ifm(room_size)]
    if {$map_canvas_height > $height} {set map_canvas_height $height}

    # Initialise window.
    set w .$sect
    catch {destroy $w}
    toplevel $w
    wm title $w $title

    # Set up canvas.
    set f $w.frame
    frame $f
    pack $f -fill both -expand yes

    set c $f.map
    canvas $c -width ${map_canvas_width}c -height ${map_canvas_height}c \
	    -scrollregion "0 0 ${width}c ${height}c" \
	    -xscrollcommand "$f.xscroll set" \
	    -yscrollcommand "$f.yscroll set" \
	    -relief sunken -bd 2 \
	    -bg $ifm(map_background_colour)
    scrollbar $f.xscroll -command "$c xview" -orient horiz
    scrollbar $f.yscroll -command "$c yview"

    bind $c <1> "$c scan mark %x %y"
    bind $c <B1-Motion> "$c scan dragto %x %y"

    grid $c -in $f -row 0 -column 0 \
	    -rowspan 1 -columnspan 1 -sticky nsew
    grid $f.xscroll -in $f -row 1 -column 0 \
	    -rowspan 1 -columnspan 1 -sticky nsew
    grid $f.yscroll -in $f -row 0 -column 1 \
	    -rowspan 1 -columnspan 1 -sticky nsew

    grid rowconfig    $f 0 -weight 1 -minsize 0
    grid columnconfig $f 0 -weight 1 -minsize 0

    # Draw rooms.
    foreach room $rooms {
	if {[Get $room sect] == $sect} {
	    set desc [Get $room desc]
	    set xoff [Get $room xpos]
	    set yoff [Get $room ypos]
	    set yoff [expr $ylen - 1 - $yoff]
	    set items [Get $room items]
	    set sxoff [Get $room room_shadow_xoff]
	    set syoff [Get $room room_shadow_yoff]

	    if {$sxoff != 0 || $syoff != 0} {
		set xsoff [expr $ifm(room_size) * $sxoff]
		set ysoff [expr $ifm(room_size) * $syoff]
		set xmin [expr $xoff * $ifm(room_size) + $xgap - $xsoff]
		set ymin [expr $yoff * $ifm(room_size) + $ygap + $ysoff]
		set xmax [expr $xmin + $ifm(room_size) * $ifm(room_width)]
		set ymax [expr $ymin + $ifm(room_size) * $ifm(room_height)]

		$c create rectangle ${xmin}c ${ymin}c ${xmax}c ${ymax}c \
			-width $ifm(room_border_width) \
			-outline $ifm(room_shadow_colour) \
			-fill $ifm(room_shadow_colour)
	    }

	    set xmin [expr $xoff * $ifm(room_size) + $xgap]
	    set ymin [expr $yoff * $ifm(room_size) + $ygap]
	    set xmax [expr $xmin + $ifm(room_size) * $ifm(room_width)]
	    set ymax [expr $ymin + $ifm(room_size) * $ifm(room_height)]

	    $c create rectangle ${xmin}c ${ymin}c ${xmax}c ${ymax}c \
		    -width [Get $room room_border_width] \
		    -outline [Get $room room_border_colour] \
		    -fill [Get $room room_colour]

	    set xc [expr ($xmin + $xmax) / 2]
	    set yc [expr ($ymin + $ymax) / 2]
	    set wid [expr ($xmax - $xmin) * 0.9]

	    if {[Get $room show_items] && $items != ""} {
		set fac $ifm(roomitemratio)

		set yo [expr $ifm(room_size) * $ifm(room_height) * $fac / 2]
		$c create text ${xc}c [expr $yc - $yo]c -text $desc \
			-width ${wid}c -justify center \
			-font [Get $room room_text_fontdef] \
			-fill [Get $room room_text_colour]

		set yo [expr $ifm(room_size) * $ifm(room_height) * (1 - $fac) / 2]
		$c create text ${xc}c [expr $yc + $yo]c -text $items \
			-width ${wid}c -justify center \
			-font [Get $room item_text_fontdef] \
			-fill [Get $room item_text_colour]
	    } else {
		$c create text ${xc}c ${yc}c -text $desc \
			-width ${wid}c -justify center \
			-font [Get $room room_text_fontdef] \
			-fill [Get $room room_text_colour]
	    }
	}
    }

    # Draw room exits.
    foreach exit $exits {
	if {[Get $exit sect] == $sect} {
	    set coords [Truncate $exit $ifm(room_width) $ifm(room_height)]
	    set xlist [lindex $coords 0]
	    set ylist [lindex $coords 1]

	    set x1 [lindex $xlist 0]
	    set y1 [lindex $ylist 0]
	    set y1 [expr $ylen - 1 - $y1]

	    set x3 [lindex $xlist 1]
	    set y3 [lindex $ylist 1]
	    set y3 [expr $ylen - 1 - $y3]

	    set fac 0.35
	    set x2 [expr $x1 + ($x3 - $x1) * $fac]
	    set y2 [expr $y1 + ($y3 - $y1) * $fac]

	    set cmd "$c create line"

	    set x1 [expr ($x1 + 0.5) * $ifm(room_size)]
	    set y1 [expr ($y1 + 0.5) * $ifm(room_size)]
	    lappend cmd ${x1}c ${y1}c

	    set x2 [expr ($x2 + 0.5) * $ifm(room_size)]
	    set y2 [expr ($y2 + 0.5) * $ifm(room_size)]
	    lappend cmd ${x2}c ${y2}c

	    lappend cmd -width [Get $room room_exit_width] \
		    -fill [Get $room room_exit_colour]
	    eval $cmd
	}
    }

    # Draw links.
    foreach link $links {
	if {[Get $link sect] == $sect} {
	    set coords [Truncate $link $ifm(room_width) $ifm(room_height)]
	    set xlist [lindex $coords 0]
	    set ylist [lindex $coords 1]

	    set cmd "$c create line"
	    for {set i 0} {$i < [llength $xlist]} {incr i} {
		if {$i > 0} {
		    set xlast $x
		    set ylast $y
		}

		set xoff [lindex $xlist $i]
		set yoff [lindex $ylist $i]
		set yoff [expr $ylen - 1 - $yoff]
		set x [expr ($xoff + 0.5) * $ifm(room_size)]
		set y [expr ($yoff + 0.5) * $ifm(room_size)]
		lappend cmd ${x}c ${y}c
	    }

	    if [Get $link oneway] {
		set a1 [expr [Get $link link_arrow_size] * $ifm(room_size)]
		set a2 $a1
		set a3 [expr $a2 / 3.24]
		lappend cmd -arrow last -arrowshape "${a1}c ${a2}c ${a3}c"
	    }

	    lappend cmd -fill [Get $link link_colour]
	    lappend cmd -width [Get $link link_line_width] \
		    -smooth [Get $link link_spline]
	    eval $cmd

	    set updown [Get $link updown]
	    set inout [Get $link inout]
	    if {$updown || $inout} {
		set xmid [expr ($x + $xlast) / 2]
		set ymid [expr ($y + $ylast) / 2]

		if {$updown} {
		    set text [Get $link link_updown_string]
		} else {
		    set text [Get $link link_inout_string]
		}

		$c create text ${xmid}c ${ymid}c -text $text \
			-font [Get $link link_text_fontdef] \
			-fill [Get $link link_text_colour]
	    }
	}
    }

    # Add 'dismiss' button.
    set b $w.bye
    button $b -text "Dismiss" -command "destroy $w"
    pack $b -fill x

    Unbusy
}

# Show item list.
proc ShowItems {} {
    global ifm

    if [NeedSave] return

    # Get item data.
    Busy
    set result [RunProgram $ifm(itemcmd) $ifm(path)]
    if [lindex $result 0] {
	set data [lindex $result 1]
    } else {
	Unbusy
	Error [lindex $result 2]
	return
    }

    # Display results.
    eval $data
    if {$itemlist != ""} {
	TextWindow "Items" .items $itemlist $ifm(itemwidth) $ifm(itemheight)
    } else {
	Message "No items found"
    }

    Unbusy
}

# Show task list.
proc ShowTasks {} {
    global ifm

    if [NeedSave] return

    # Get task data.
    Busy

    set result [RunProgram $ifm(taskcmd) $ifm(path)]
    if [lindex $result 0] {
	set data [lindex $result 1]
    } else {
	Unbusy
	Error [lindex $result 2]
	return
    }

    # Display results.
    eval $data
    if {$tasklist != ""} {
	TextWindow "Tasks" .tasks $tasklist $ifm(taskwidth) $ifm(taskheight)
    } else {
	Message "No tasks found"
    }

    Unbusy
}

# Show verbose task output.
proc ShowVerbose {} {
    global ifm

    if [NeedSave] return

    # Get task data.
    Busy

    set result [RunProgram $ifm(verbosecmd) $ifm(path)]
    if [lindex $result 0] {
	set data [lindex $result 1]
    } else {
	Unbusy
	Error [lindex $result 2]
	return
    }

    # Display results.
    TextWindow "Verbose Task Output" .verbose $data \
	    $ifm(verbosewidth) $ifm(verboseheight)

    Unbusy
}

# Show defined variables.
proc ShowVars {} {
    global ifm

    if [NeedSave] return

    # Get variables.
    Busy

    set result [RunProgram $ifm(varscmd) $ifm(path)]
    if [lindex $result 0] {
	set data [lindex $result 1]
    } else {
	Unbusy
	Error [lindex $result 2]
	return
    }

    # Display results.
    TextWindow "Variables" .vars $data \
	    $ifm(varswidth) $ifm(varsheight)

    Unbusy
}

# Show search path.
proc ShowPath {} {
    global ifm

    # Get path.
    set result [RunProgram $ifm(pathcmd)]
    if [lindex $result 0] {
	set data [lindex $result 1]
    } else {
	Error [lindex $result 2]
	return
    }

    # Display results.
    Message "Search path: $data"
}

# Display a text window.
proc TextWindow {title w text width height} {
    global ifm

    # Initialise window.
    catch {destroy $w}
    toplevel $w
    wm title $w $title

    # Set up text.
    set f $w.frame
    set t $f.text
    set s $f.scroll

    frame $f

    text $t -yscrollcommand "$s set" -setgrid true \
	    -width $width -height $height \
	    -wrap word -font $ifm(textfont) -fg $ifm(textforeground) \
	    -bg $ifm(textbackground)
    scrollbar $s -command "$t yview"

    pack $f
    pack $s -side right -fill y
    pack $t -expand yes -fill both

    $t insert end $text
    $t configure -state disabled

    # Add 'dismiss' button.
    set b $w.bye
    button $b -text "Dismiss" -command "destroy $w"
    pack $b -fill x
}

# Build the map.
proc BuildMap {} {
    global sectnum roomnum linknum exitnum
    global sects rooms links exits
    global ifm

    # Get map data.
    set result [RunProgram $ifm(mapcmd) $ifm(path)]
    if [lindex $result 0] {
	set data [lindex $result 1]
    } else {
	Error [lindex $result 2]
	return
    }

    # Remove old windows.
    if [info exists sects] {
	foreach sect $sects {
	    if [winfo exists .$sect] {
		destroy .$sect
		set num [Get $sect num]
		set redraw($num) 1
	    }
	}
    }

    catch {destroy .items}
    catch {destroy .tasks}
    catch {destroy .verbose}
    catch {destroy .vars}

    # Set up new maps.
    set sects {}
    set rooms {}
    set links {}
    set exits {}

    set sectnum 0
    set roomnum 0
    set linknum 0
    set exitnum 0

    eval $data

    # Reconfigure map menu.
    set c $ifm(mapmenu)
    $c delete 0 end

    for {set i 1} {$i <= $sectnum} {incr i} {
	$c add command -label [Get sect$i title] -command "DrawMap $i"
	if [info exists redraw($i)] {DrawMap $i}
    }
}

# Read IFM source.
proc ReadFile {file} {
    global ifm
	
    Busy
    SetFile $file

    if [file exists $file] {
	if [catch {set fd [open $file r]}] {
	    Unbusy
	    Error "Can't open $file"
	    return
	}

	set ifm(data) [read -nonewline $fd]
	close $fd

	$ifm(text) configure -state normal
	$ifm(text) delete 0.0 end
	$ifm(text) insert end $ifm(data)
	$ifm(text) highlight 1.0 end

	if !$ifm(edit) {
	    $ifm(text) configure -state disabled
	}

	append ifm(data) "\n"
	set ifm(modtime) [file mtime $file]

	GotoLine 1
	BuildMap
    }

    Unbusy
}

# Set the current input pathname.
proc SetFile {path} {
    global ifm

    regsub {^\./} $path {} path

    set ifm(dir)  [file dirname $path]
    set ifm(file) [file tail $path]
    set ifm(path) $path
    set ifm(data) "\n"

    wm iconname . $ifm(file)

    if $ifm(edit) {
	wm title . $ifm(file)
    } else {
	wm title . "$ifm(file) (read-only)"
    }
}

# Add a section.
proc AddSect {title xlen ylen} {
    global sects sectnum
    incr sectnum
    set var sect$sectnum
    lappend sects $var

    Set $var num $sectnum
    Set $var title $title
    Set $var xlen $xlen
    Set $var ylen $ylen
}

# Add a room.
proc AddRoom {desc items xpos ypos} {
    global ifm rooms roomnum sectnum
    incr roomnum
    set var room$roomnum
    lappend rooms $var

    Set $var num $roomnum
    Set $var desc $desc
    Set $var items $items
    Set $var xpos $xpos
    Set $var ypos $ypos

    Set $var sect sect$sectnum

    foreach attr $ifm(roomvars) {
	Set $var $attr $ifm($attr)
    }
}

# Add a link.
proc AddLink {xlist ylist updown inout oneway} {
    global ifm links linknum sectnum
    incr linknum
    set var link$linknum
    global $var
    lappend links $var

    Set $var num $linknum
    Set $var xlist $xlist
    Set $var ylist $ylist
    Set $var updown $updown
    Set $var inout $inout
    Set $var oneway $oneway

    Set $var sect sect$sectnum

    foreach attr $ifm(linkvars) {
	Set $var $attr $ifm($attr)
    }
}

# Add a room exit.
proc AddExit {xlist ylist} {
    global exits exitnum sectnum
    incr exitnum
    set var exit$exitnum
    global $var
    lappend exits $var

    Set $var num $exitnum
    Set $var xlist $xlist
    Set $var ylist $ylist

    Set $var sect sect$sectnum
}

# Start a new file.
proc New {} {
    global ifm

    if {[MaybeSave] == 0} return
    SetFile [file join $ifm(dir) $ifm(untitled)]
    $ifm(text) delete 0.0 end
    set ifm(data) ""

    GotoLine 1
}

# Open a file and parse it.
proc Open {} {
    global ifm

    if {[MaybeSave] == 0} return
    set file [tk_getOpenFile -filetypes $ifm(ifmfiles) -initialdir $ifm(dir)]
    if [string length $file] {ReadFile $file}
}

# Save the current file.
proc Save {} {
    global ifm

    if [catch {set fd [open $ifm(path) w]}] {
	Error "Can't save $file"
	return
    }

    Busy
    set ifm(data) [$ifm(text) get 0.0 end]
    puts -nonewline $fd $ifm(data)
    close $fd

    set ifm(modtime) [file mtime $ifm(path)]

    BuildMap
    Unbusy
}

# Save current file under another name.
proc SaveAs {} {
    global ifm

    set file [tk_getSaveFile -initialfile $ifm(path) -initialdir $ifm(dir) \
	    -filetypes $ifm(ifmfiles)]

    if [string length $file] {
	SetFile $file
	Save
    }
}

# Export current file to PostScript.
proc ExportPS {} {
    global ifm

    if [file exists $ifm(path)] {
	if {[MaybeSave] == 0} return
	if {[MaybeRead] == 0} return
    } else {
	Message "You must save the current file first."
	return
    }

    # Get save filename.
    set root [file rootname $ifm(file)]
    set file [tk_getSaveFile -initialfile ${root}.ps -initialdir $ifm(dir) \
	    -filetypes $ifm(psfiles) -title "Export PostScript"]
    if {$file == ""} return

    # Get PostScript data.
    Busy

    set result [RunProgram $ifm(pscmd) $ifm(path)]
    if [lindex $result 0] {
	set data [lindex $result 1]
    } else {
	Unbusy
	Error [lindex $result 2]
	return
    }

    # Write file.
    if [catch {set fd [open $file w]}] {
	Unbusy
	Error "Can't save $file"
	return
    }

    puts $fd $data
    close $fd

    Unbusy
}

# Export current file to Fig.
proc ExportFIG {} {
    global ifm

    if [file exists $ifm(path)] {
	if {[MaybeSave] == 0} return
	if {[MaybeRead] == 0} return
    } else {
	Message "You must save the current file first."
	return
    }

    # Get save filename.
    set root [file rootname $ifm(file)]
    set file [tk_getSaveFile -initialfile ${root}.fig -initialdir $ifm(dir) \
	    -filetypes $ifm(figfiles) -title "Export Fig"]
    if {$file == ""} return

    # Get Fig data.
    Busy

    set result [RunProgram $ifm(figcmd) $ifm(path)]
    if [lindex $result 0] {
	set data [lindex $result 1]
    } else {
	Unbusy
	Error [lindex $result 2]
	return
    }

    # Write file.
    if [catch {set fd [open $file w]}] {
	Unbusy
	Error "Can't save $file"
	return
    }

    puts $fd $data
    close $fd

    Unbusy
}

# Save current file if required.
proc MaybeSave {} {
    global ifm

    if [Modified] {
	set reply [Yesno "$ifm(path) has been modified.  Save it?" "yes"]
	if {$reply == "cancel"} {return 0}
	if {$reply == "yes"} Save
    }

    return 1
}

# Reread current file if required.
proc MaybeRead {} {
    global ifm

    if [file exists $ifm(path)] {
	if {[file mtime $ifm(path)] > $ifm(modtime)} {
	    ReadFile $ifm(path)
	}
    }

    return 1
}

# Redraw the map.
proc Redraw {} {
    global ifm

    if [file exists $ifm(path)] {
	if {[MaybeSave] == 0} return
	Busy
	BuildMap
	Unbusy
    } else {
	Message "You must save the current file first."
    }
}

# Quit.
proc Quit {} {
    if [MaybeSave] {destroy .}
}

# Display info about program.
proc About {} {
    global ifm

    set result [RunProgram $ifm(aboutcmd)]
    if [lindex $result 0] {
	set text [lindex $result 1]
    } else {
	Error [lindex $result 2]
	return
    }

    Message $text
}

# Run a program and return its results.
proc RunProgram {prog args} {
    global ifm

    set result {}
    set errmsg {}
    set err [catch {set result [eval exec $prog $args]} errmsg]

    return [list [expr !$err] $result $errmsg]
}

# Return whether source has been modified.
proc Modified {} {
    global ifm
    set curdata [$ifm(text) get 0.0 end]
    if {$ifm(data) != $curdata} {return 1}
    return 0
}

# Go to a text line.
proc GotoLine {num} {
    global ifm
    $ifm(text) mark set insert "$num.0"
    $ifm(text) see insert
}

# Return whether save is required.
proc NeedSave {} {
    global ifm

    if [file exists $ifm(path)] {
	if {[MaybeSave] == 0} {return 1}
	if {[MaybeRead] == 0} {return 1}
    } else {
	Message "You must save the current file first."
	return 1
    }

    return 0
}

# Truncate links to join on to boxes properly.
proc Truncate {link wid ht} {
    set x [Get $link xlist]
    set y [Get $link ylist]
    set xfac [expr 1 - $wid / 2]
    set yfac [expr 1 - $ht / 2]
    set np [llength $x]

    # Truncate first point.
    set x1 [lindex $x 1]
    set y1 [lindex $y 1]
    set x2 [lindex $x 0]
    set y2 [lindex $y 0]
    set xf [expr $x1 + $xfac * ($x2 - $x1)]
    set yf [expr $y1 + $yfac * ($y2 - $y1)]

    # Truncate last point.
    set x1 [lindex $x [expr $np - 2]]
    set y1 [lindex $y [expr $np - 2]]
    set x2 [lindex $x [expr $np - 1]]
    set y2 [lindex $y [expr $np - 1]]
    set xl [expr $x1 + $xfac * ($x2 - $x1)]
    set yl [expr $y1 + $yfac * ($y2 - $y1)]

    # Replace points.
    set x [lreplace $x 0 0 $xf]
    set x [lreplace $x end end $xl]
    set y [lreplace $y 0 0 $yf]
    set y [lreplace $y end end $yl]

    return [list $x $y]
}

# Set busy state.
proc Busy {} {
    global ifm

    set cursor [$ifm(text) cget -cursor]

    if {$cursor != $ifm(busycursor)} {
	set ifm(oldcursor) $cursor
	$ifm(text) configure -cursor $ifm(busycursor)
	update idletasks
    }
}

# Set unbusy state.
proc Unbusy {} {
    global ifm

    if [info exists ifm(oldcursor)] {
	$ifm(text) configure -cursor $ifm(oldcursor)
	update idletasks
    }
}

# Ask a yes/no question.
proc Yesno {question default} {
    tk_messageBox -message $question -type yesnocancel \
	    -default $default -icon question
}

# Print a message.
proc Message {message} {
    tk_messageBox -message $message -type ok \
	    -default ok -icon info
}

# Print a warning.
proc Warning {message} {
    tk_messageBox -message $message -type ok \
	    -default ok -title "Scary!" -icon warning
}

# Print an error.
proc Error {message} {
    bell
    tk_messageBox -message $message -type ok \
	    -default ok -title "Oops!" -icon error
}

# Set a variable reference.
proc Set {var attr val} {
    global $var
    set ${var}($attr) $val
}

# Return value of a variable reference.
proc Get {var attr} {
    global $var
    return [set ${var}($attr)]
}

# Allow customizations.
if {$tcl_platform(platform) == "unix"} {
    set rcname ".tkifmrc"
} else {
    set rcname "tkifm.ini"
}

set rcfile [file join $env(HOME) $rcname]
if [file readable $rcfile] {source $rcfile}

if {$tcl_platform(platform) == "unix"} {
    option add *Dialog.msg.wrapLength 10i
}

# Boot up.
MainWindow

# Parse arguments.
if {$argc == 1} {
    ReadFile [lindex $argv 0]
} else {
    SetFile $ifm(untitled)
}
