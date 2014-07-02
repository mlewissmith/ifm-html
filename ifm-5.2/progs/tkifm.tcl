# This file is part of IFM, the Interactive Fiction Mapper, copyright (C)
# Glenn Hutchings 1997-2006.

# IFM comes with ABSOLUTELY NO WARRANTY.  This is free software, and you
# are welcome to redistribute it under certain conditions; see the file
# COPYING for details.

# ctext -- Tcl/Tk syntax highlighter
# Based on code by George Peter Staplin

# Parts of this file are also covered by the TCL license;
# see the file COPYING.TCL for copyright details.

namespace eval ctext {}

#win is used as a unique token to create arrays for each ctext instance
proc ctext::getAr {win suffix name} {
    set arName __ctext[set win][set suffix]
    uplevel [list upvar #0 $arName $name]
    return $arName
}

proc ctext {win args} { 
    if {[llength $args] & 1} {
        return -code error "invalid number of arguments given to ctext (uneven number after window) : $args"
    }
    
    frame $win -class Ctext

    set tmp [text .__ctextTemp]
    
    ctext::getAr $win config ar

    set ar(-fg) [$tmp cget -foreground]
    set ar(-bg) [$tmp cget -background]
    set ar(-font) [$tmp cget -font]
    set ar(-relief) [$tmp cget -relief]
    destroy $tmp
    set ar(-yscrollcommand) ""
    set ar(-linemap) 1
    set ar(-linemapfg) $ar(-fg)
    set ar(-linemapbg) $ar(-bg)
    set ar(-linemap_mark_command) {}
    set ar(-linemap_markable) 1
    set ar(-linemap_select_fg) black
    set ar(-linemap_select_bg) yellow
    set ar(-highlight) 1
    set ar(win) $win
    set ar(modified) 0
    
    set ar(ctextFlags) [list -yscrollcommand -linemap -linemapfg -linemapbg \
-font -linemap_mark_command -highlight -linemap_markable -linemap_select_fg \
-linemap_select_bg]
    
    array set ar $args
    
    foreach flag {foreground background} short {fg bg} {
        if {[info exists ar(-$flag)] == 1} {
            set ar(-$short) $ar(-$flag)
            unset ar(-$flag)
        }
    }
    
    #Now remove flags that will confuse text and those that need modification:
    foreach arg $ar(ctextFlags) {
        if {[set loc [lsearch $args $arg]] >= 0} {
            set args [lreplace $args $loc [expr {$loc + 1}]]
        }
    }
    
    text $win.l -font $ar(-font) -width 1 -height 1 \
        -relief $ar(-relief) -fg $ar(-linemapfg) \
        -bg $ar(-linemapbg) -takefocus 0

    set topWin [winfo toplevel $win]
    bindtags $win.l [list $win.l $topWin all]

    if {$ar(-linemap) == 1} {
        grid $win.l -sticky ns -row 0 -column 0
    }
    
    set args [concat $args [list -yscrollcommand [list ctext::event:yscroll $win $ar(-yscrollcommand)]]]

    #escape $win, because it could have a space
    eval text \$win.t -font \$ar(-font) $args
    
    grid $win.t -row 0 -column 1 -sticky news
    grid rowconfigure $win 0 -weight 100
    grid columnconfigure $win 1 -weight 100

    bind $win.t <Configure> [list ctext::linemapUpdate $win]
    bind $win.l <ButtonPress-1> [list ctext::linemapToggleMark $win %y]
    bind $win.t <KeyRelease-Return> [list ctext::linemapUpdate $win]
    rename $win __ctextJunk$win
    rename $win.t $win._t

    bind $win <Destroy> [list ctext::event:Destroy $win %W]
    bindtags $win.t [linsert [bindtags $win.t] 0 $win]

    interp alias {} $win {} ctext::instanceCmd $win
    interp alias {} $win.t {} $win
    
    #If the user wants C comments they should call ctext::enableComments
    ctext::disableComments $win
    ctext::modified $win 0
    ctext::buildArgParseTable $win

    return $win
}

proc ctext::event:yscroll {win clientData args} {
    ctext::linemapUpdate $win

    if {$clientData == ""} {
        return
    }
    uplevel #0 $clientData $args
}

proc ctext::event:Destroy {win dWin} {
    if {![string equal $win $dWin]} {
        return
    }
    catch {rename $win {}}
    interp alias {} $win.t {}
    ctext::clearHighlightClasses $win
    array unset [ctext::getAr $win config ar]
}

#This stores the arg table within the config array for each instance.
#It's used by the configure instance command.
proc ctext::buildArgParseTable win {
    set argTable [list]

    lappend argTable any -linemap_mark_command {
        set configAr(-linemap_mark_command) $value
        break
    }

    lappend argTable {1 true yes} -linemap {
        grid $self.l -sticky ns -row 0 -column 0
        grid columnconfigure $self 0 \
            -minsize [winfo reqwidth $self.l]
        set configAr(-linemap) 1
        break
    }

    lappend argTable {0 false no} -linemap {
        grid forget $self.l
        grid columnconfigure $self 0 -minsize 0
        set configAr(-linemap) 0
        break
    }

    lappend argTable any -yscrollcommand {
        set cmd [list $self._t config -yscrollcommand [list ctext::event:yscroll $self $value]]

        if {[catch $cmd res]} {
            return $res
        }
        set configAr(-yscrollcommand) $value
        break
    }

    lappend argTable any -linemapfg {
        if {[catch {winfo rgb $self $value} res]} {
            return -code error $res
        }
        $self.l config -fg $value
        set configAr(-linemapfg) $value
        break
    }

    lappend argTable any -linemapbg {
        if {[catch {winfo rgb $self $value} res]} {
            return -code error $res
        }
        $self.l config -bg $value
        set configAr(-linemapbg) $value
        break
    }

    lappend argTable any -font {
        if {[catch {$self.l config -font $value} res]} {
            return -code error $res
        }
        $self._t config -font $value
        set configAr(-font) $value
        break
    }

    lappend argTable {0 false no} -highlight {
        set configAr(-highlight) 0
        break
    }

    lappend argTable {1 true yes} -highlight {
        set configAr(-highlight) 1
        break
    }

    lappend argTable {0 false no} -linemap_markable {
        set configAr(-linemap_markable) 0
        break
    }

    lappend argTable {1 true yes} -linemap_markable {
        set configAr(-linemap_markable) 1
        break
    }

    lappend argTable any -linemap_select_fg {
        if {[catch {winfo rgb $self $value} res]} {
            return -code error $res
        }
        set configAr(-linemap_select_fg) $value
        $self.l tag configure lmark -foreground $value
        break
    }

    lappend argTable any -linemap_select_bg {
        if {[catch {winfo rgb $self $value} res]} {
            return -code error $res
        }
        set configAr(-linemap_select_bg) $value
        $self.l tag configure lmark -background $value
        break
    }

    ctext::getAr $win config ar
    set ar(argTable) $argTable
}

proc ctext::instanceCmd {self cmd args} {
    #slightly different than the RE used in ctext::comments
    set commentRE {\"|\\|'|/|\*}

    switch -glob -- $cmd {
        append {
            if {[catch {$self._t get sel.first sel.last} data] == 0} {
                clipboard append -displayof $self $data
            }
        }

        cget {
            set arg [lindex $args 0]
            ctext::getAr $self config configAr

            foreach flag $configAr(ctextFlags) {
                if {[string match ${arg}* $flag]} {
                    return [set configAr($flag)]
                }
            }
            return [$self._t cget $arg]
        }

        conf* {
            ctext::getAr $self config configAr

            if {0 == [llength $args]} {
                set res [$self._t configure]
                set del [lsearch -glob $res -yscrollcommand*]        
                set res [lreplace $res $del $del]
            
                foreach flag $configAr(ctextFlags) {
                    lappend res [list $flag [set configAr($flag)]]
                }                
                return $res
            }
            
            array set flags {}
            foreach flag $configAr(ctextFlags) {
                set loc [lsearch $args $flag]
                if {$loc < 0} {
                    continue
                }
                
                if {[llength $args] <= ($loc + 1)} {
                    #.t config -flag
                    return [set configAr($flag)]
                }

                set flagArg [lindex $args [expr {$loc + 1}]]
                set args [lreplace $args $loc [expr {$loc + 1}]]
                set flags($flag) $flagArg
            }

            foreach {valueList flag cmd} $configAr(argTable) {
                if {[info exists flags($flag)]} {
                    foreach valueToCheckFor $valueList {
                        set value [set flags($flag)]
                        if {[string equal "any" $valueToCheckFor]} $cmd \
                        elseif {[string equal $valueToCheckFor [set flags($flag)]]} $cmd 
                    }
                }
            }
            
            if {[llength $args]} {
                #we take care of configure without args at the top of this branch
                uplevel 1 [linsert $args 0 $self._t configure]
            }
        }

        copy {
            tk_textCopy $self
        } 

        cut {
            if {[catch {$self.t get sel.first sel.last} data] == 0} {
                clipboard clear -displayof $self.t
                clipboard append -displayof $self.t $data
                $self delete [$self.t index sel.first] [$self.t index sel.last]
                ctext::modified $self 1
            }
        }

        delete {
            #delete n.n ?n.n
            
            #first deal with delete n.n
            set argsLength [llength $args]
            
            if {$argsLength == 1} {
                set deletePos [lindex $args 0]
                set prevChar [$self._t get $deletePos]
                
                $self._t delete $deletePos
                set char [$self._t get $deletePos]
                
                set prevSpace [ctext::findPreviousSpace $self._t $deletePos]
                set nextSpace [ctext::findNextSpace $self._t $deletePos]
                
                set lineStart [$self._t index "$deletePos linestart"]
                set lineEnd [$self._t index "$deletePos + 1 chars lineend"]
                
                if {[string equal $prevChar "#"] || [string equal $char "#"]} {
                    set removeStart $lineStart
                    set removeEnd $lineEnd
                } else {
                    set removeStart $prevSpace
                    set removeEnd $nextSpace
                }
                
                foreach tag [$self._t tag names] {
                    if {[string equal $tag "_cComment"] != 1} {
                        $self._t tag remove $tag $removeStart $removeEnd
                    }
                }
                
                set checkStr "$prevChar[set char]"
                
                if {[regexp $commentRE $checkStr]} {
                    after idle [list ctext::comments $self]
                }
                ctext::highlight $self $lineStart $lineEnd
                ctext::linemapUpdate $self
            } elseif {$argsLength == 2} {
                #now deal with delete n.n ?n.n?
                set deleteStartPos [lindex $args 0]
                set deleteEndPos [lindex $args 1]
                
                set data [$self._t get $deleteStartPos $deleteEndPos]
                
                set lineStart [$self._t index "$deleteStartPos linestart"]
                set lineEnd [$self._t index "$deleteEndPos + 1 chars lineend"]
                eval \$self._t delete $args
                
                foreach tag [$self._t tag names] {
                    if {[string equal $tag "_cComment"] != 1} {
                        $self._t tag remove $tag $lineStart $lineEnd
                    }
                }
                
                if {[regexp $commentRE $data]} {
                    after idle [list ctext::comments $self]
                }
                
                ctext::highlight $self $lineStart $lineEnd
                if {[string first "\n" $data] >= 0} {
                    ctext::linemapUpdate $self
                }
            } else {
                return -code error "invalid argument(s) sent to $self delete: $args"
            }
            ctext::modified $self 1
        }

        fastdelete {
            eval \$self._t delete $args
            ctext::modified $self 1
            ctext::linemapUpdate $self
        }
        
        fastinsert {
            eval \$self._t insert $args
            ctext::modified $self 1
            ctext::linemapUpdate $self
        }
        
        highlight {
            ctext::highlight $self [lindex $args 0] [lindex $args 1]
            ctext::comments $self
        }

        insert {
            if {[llength $args] < 2} {
                return -code error "please use at least 2 arguments to $self insert"
            }
            set insertPos [lindex $args 0]  
            set prevChar [$self._t get "$insertPos - 1 chars"]
            set nextChar [$self._t get $insertPos]
            set lineStart [$self._t index "$insertPos linestart"]
            set prevSpace [ctext::findPreviousSpace $self._t ${insertPos}-1c]
            set data [lindex $args 1]   
            eval \$self._t insert $args 

            set nextSpace [ctext::findNextSpace $self._t insert]
            set lineEnd [$self._t index "insert lineend"] 
             
            if {[$self._t compare $prevSpace < $lineStart]} {
                set prevSpace $lineStart
            }

            if {[$self._t compare $nextSpace > $lineEnd]} {
                set nextSpace $lineEnd
            }
            
            foreach tag [$self._t tag names] { 
                if {[string equal $tag "_cComment"] != 1} {
                    $self._t tag remove $tag $prevSpace $nextSpace 
                }
            } 

            set REData $prevChar
            append REData $data
            append REData $nextChar
            if {[regexp $commentRE $REData]} {
                after idle [list ctext::comments $self]
            }
            
        
            ctext::modified $self 1
            ctext::linemapUpdate $self
        }

        paste {
            tk_textPaste $self 
            ctext::modified $self 1
        }

        edit {
            set subCmd [lindex $args 0]
            set argsLength [llength $args]
            
            ctext::getAr $self config ar

            if {"modified" == $subCmd} {
                if {$argsLength == 1} {
                    return $ar(modified)
                } elseif {$argsLength == 2} {
                    set value [lindex $args 1]
                    set ar(modified) $value
                } else {
                    return -code error "invalid arg(s) to $self edit modified: $args"
                }
            } else {
                #Tk 8.4 has other edit subcommands that I don't want to emulate.
                return [uplevel 1 [linsert $args 0 $self._t $cmd]]
            }
        }
        
        default { 
            return [uplevel 1 [linsert $args 0 $self._t $cmd]]
        }
    }
}

proc ctext::tag:blink {win count} {
    if {$count & 1} {
        $win tag configure __ctext_blink -foreground [$win cget -bg] -background [$win cget -fg]
    } else {
        $win tag configure __ctext_blink -foreground [$win cget -fg] -background [$win cget -bg]
    }

    if {$count == 4} {
        $win tag delete __ctext_blink 1.0 end
        return
    }
    incr count
    after 50 [list ctext::tag:blink $win $count]
}

proc ctext::matchPair {win str1 str2 escape} {
    set prevChar [$win get "insert - 2 chars"]
    
    if {[string equal $prevChar $escape]} {
        #The char that we thought might be the end is actually escaped.
        return
    }

    set searchRE "[set str1]|[set str2]"
    set count 1
    
    set pos [$win index "insert - 1 chars"]
    set endPair $pos
    set lastFound ""
    while 1 {
        set found [$win search -backwards -regexp $searchRE $pos]
        
        if {$found == "" || [$win compare $found > $pos]} {
            return
        }

        if {$lastFound != "" && [$win compare $found == $lastFound]} {
            #The search wrapped and found the previous search
            return
        }
        
        set lastFound $found
        set char [$win get $found]
        set prevChar [$win get "$found - 1 chars"]
        set pos $found

        if {[string equal $prevChar $escape]} {
            continue
        } elseif {[string equal $char [subst $str2]]} {
            incr count
        } elseif {[string equal $char [subst $str1]]} {
            incr count -1
            if {$count == 0} {
                set startPair $found
                break
            } 
        } else {
            #This shouldn't happen.  I may in the future make it return -code error
            puts stderr "ctext seems to have encountered a bug in ctext::matchPair"
            return
        }
    }
    
    $win tag add __ctext_blink $startPair
    $win tag add __ctext_blink $endPair
    ctext::tag:blink $win 0
}

proc ctext::matchQuote {win} {
    set endQuote [$win index insert]
    set start [$win index "insert - 1 chars"]
    
    if {[$win get "$start - 1 chars"] == "\\"} {
        #the quote really isn't the end
        return
    }
    set lastFound ""
    while 1 {
        set startQuote [$win search -backwards \" $start]
        if {$startQuote == "" || [$win compare $startQuote > $start]} {
            #The search found nothing or it wrapped.
            return
        }

        if {$lastFound != "" && [$win compare $lastFound == $startQuote]} {
            #We found the character we found before, so it wrapped.
            return
        }
        set lastFound $startQuote
        set start [$win index "$startQuote - 1 chars"]
        set prevChar [$win get $start]

        if {$prevChar == "\\"} {
            continue
        }
        break
    }
    
    if {[$win compare $endQuote == $startQuote]} {
        #probably just \"
        return
    }
    
    $win tag add __ctext_blink $startQuote $endQuote
    ctext::tag:blink $win 0
}

proc ctext::enableComments {win} {
    $win tag configure _cComment -foreground khaki
}
proc ctext::disableComments {win} {
    catch {$win tag delete _cComment}
}

proc ctext::comments {win} {
    if {[catch {$win tag cget _cComment -foreground}]} {
        #C comments are disabled
        return
    }

    set startIndex 1.0
    set commentRE {\\\\|\"|\\\"|\\'|'|/\*|\*/}
    set commentStart 0
    set isQuote 0
    set isSingleQuote 0
    set isComment 0
    $win tag remove _cComment 1.0 end
    while 1 {
        set index [$win search -count length -regexp $commentRE $startIndex end]
        
        if {$index == ""} {
            break
        }
        
        set endIndex [$win index "$index + $length chars"]
        set str [$win get $index $endIndex]
        set startIndex $endIndex

        if {$str == "\\\\"} {
            continue
        } elseif {$str == "\\\""} {
            continue
        } elseif {$str == "\\'"} {
            continue
        } elseif {$str == "\"" && $isComment == 0 && $isSingleQuote == 0} {
            if {$isQuote} {
                set isQuote 0
            } else {
                set isQuote 1
            }
        } elseif {$str == "'" && $isComment == 0 && $isQuote == 0} {
            if {$isSingleQuote} {
                set isSingleQuote 0
            } else {
                set isSingleQuote 1
            }
        } elseif {$str == "/*" && $isQuote == 0 && $isSingleQuote == 0} {
            if {$isComment} {
                #comment in comment
                break
            } else {
                set isComment 1
                set commentStart $index
            }
        } elseif {$str == "*/" && $isQuote == 0 && $isSingleQuote == 0} {
            if {$isComment} {
                set isComment 0
                $win tag add _cComment $commentStart $endIndex
                $win tag raise _cComment
            } else {
                #comment end without beginning
                break
            }
        }
    }
}

proc ctext::addHighlightClass {win class color keywords} { 
    set ref [ctext::getAr $win highlight ar]
    foreach word $keywords {
        set ar($word) [list $class $color]
    }
    $win tag configure $class 

    ctext::getAr $win classes classesAr
    set classesAr($class) [list $ref $keywords]
}

#For [ ] { } # etc.
proc ctext::addHighlightClassForSpecialChars {win class color chars} {  
    set charList [split $chars ""]

    set ref [ctext::getAr $win highlightSpecialChars ar]
    foreach char $charList {
        set ar($char) [list $class $color]
    }
    $win tag configure $class 

    ctext::getAr $win classes classesAr
    set classesAr($class) [list $ref $charList]
}

proc ctext::addHighlightClassForRegexp {win class color re} {  
    set ref [ctext::getAr $win highlightRegexp ar]

    set ar($class) [list $re $color]
    $win tag configure $class 

    ctext::getAr $win classes classesAr
    set classesAr($class) [list $ref $class]
}

#For things like $blah 
proc ctext::addHighlightClassWithOnlyCharStart {win class color char} { 
    set ref [ctext::getAr $win highlightCharStart ar]

    set ar($char) [list $class $color]
    $win tag configure $class 

    ctext::getAr $win classes classesAr
    set classesAr($class) [list $ref $char]
}

proc ctext::deleteHighlightClass {win classToDelete} {
    ctext::getAr $win classes classesAr

    if {![info exists classesAr($classToDelete)]} {
        return -code error "$classToDelete doesn't exist"
    }
    
    foreach {ref keyList} [set classesAr($classToDelete)] {
        upvar #0 $ref refAr
        foreach key $keyList {
            if {![info exists refAr($key)]} {
                continue
            }
            unset refAr($key)
        }
    }
    unset classesAr($classToDelete)
}

proc ctext::getHighlightClasses win {
    ctext::getAr $win classes classesAr

    array names classesAr
}

proc ctext::findNextChar {win index char} {
    set i [$win index "$index + 1 chars"]
    set lineend [$win index "$i lineend"]
    while 1 {
        set ch [$win get $i]
        if {[$win compare $i >= $lineend]} {
            return ""
        }
        if {$ch == $char} {
            return $i
        }
        set i [$win index "$i + 1 chars"]
    }
}

proc ctext::findNextSpace {win index} {
    set i [$win index $index]
    set lineStart [$win index "$i linestart"]
    set lineEnd [$win index "$i lineend"]
    #Sometimes the lineend fails (I don't know why), so add 1 and try again.
    if {[$win compare $lineEnd == $lineStart]} {
        set lineEnd [$win index "$i + 1 chars lineend"]
    }

    while {1} {
        set ch [$win get $i]

        if {[$win compare $i >= $lineEnd]} {
            set i $lineEnd
            break
        }

        if {[string is space $ch]} { 
            break
        }
        set i [$win index "$i + 1 chars"]
    }
    return $i
}

proc ctext::findPreviousSpace {win index} {
    set i [$win index $index]
    set lineStart [$win index "$i linestart"]
    while {1} {
        set ch [$win get $i]

        if {[$win compare $i <= $lineStart]} {
            set i $lineStart
            break
        }

        if {[string is space $ch]} {
            break
        }
        
        set i [$win index "$i - 1 chars"]
    }
    return $i
}

proc ctext::clearHighlightClasses {win} {
    #no need to catch, because array unset doesn't complain
    #puts [array exists ::ctext::highlight$win]

    ctext::getAr $win highlight ar
    array unset ar
    
    ctext::getAr $win highlightSpecialChars ar
    array unset ar
    
    ctext::getAr $win highlightRegexp ar
    array unset ar
    
    ctext::getAr $win highlightCharStart ar
    array unset ar
    
    ctext::getAr $win classes ar
    array unset ar
}

#This is a proc designed to be overwritten by the user.
#It can be used to update a cursor or animation while
#the text is being highlighted.
proc ctext::update {} {

}

proc ctext::highlight {win start end} {
    ctext::getAr $win config configAr

    if {!$configAr(-highlight)} {
        return
    }

    set si $start
    set twin "$win._t"
    
    #The number of times the loop has run.
    set numTimesLooped 0
    set numUntilUpdate 600

    ctext::getAr $win highlight highlightAr
    ctext::getAr $win highlightSpecialChars highlightSpecialCharsAr
    ctext::getAr $win highlightRegexp highlightRegexpAr
    ctext::getAr $win highlightCharStart highlightCharStartAr

    while 1 {
        set res [$twin search -count length -regexp -- {([^\s\(\{\[\}\]\)\.\t\n\r;\"'\|,]+)} $si $end]
        if {$res == ""} { 
            break 
        } 
        
        set wordEnd [$twin index "$res + $length chars"]
        set word [$twin get $res $wordEnd] 
        set firstOfWord [string index $word 0]

        if {[info exists highlightAr($word)] == 1} {
            set wordAttributes [set highlightAr($word)]
            foreach {tagClass color} $wordAttributes break
            
            $twin tag add $tagClass $res $wordEnd
            $twin tag configure $tagClass -foreground $color

        } elseif {[info exists highlightCharStartAr($firstOfWord)] == 1} {
            set wordAttributes [set highlightCharStartAr($firstOfWord)]
            foreach {tagClass color} $wordAttributes break
            
            $twin tag add $tagClass $res $wordEnd 
            $twin tag configure $tagClass -foreground $color
        }
        set si $wordEnd

        incr numTimesLooped
        if {$numTimesLooped >= $numUntilUpdate} {
            ctext::update
            set numTimesLooped 0
        }
    }
    
    foreach {ichar tagInfo} [array get highlightSpecialCharsAr] {
        set si $start
        foreach {tagClass color} $tagInfo break

        while 1 {
            set res [$twin search -- $ichar $si $end] 
            if {"" == $res} { 
                break 
            } 
            set wordEnd [$twin index "$res + 1 chars"]
    
            $twin tag add $tagClass $res $wordEnd
            $twin tag configure $tagClass -foreground $color
            set si $wordEnd

            incr numTimesLooped
            if {$numTimesLooped >= $numUntilUpdate} {
                ctext::update
                set numTimesLooped 0
            }
        }
    }
    
    foreach {tagClass tagInfo} [array get highlightRegexpAr] {
        set si $start
        foreach {re color} $tagInfo break
        while 1 {
            set res [$twin search -count length -regexp -- $re $si $end] 
            if {"" == $res} { 
                break 
            } 
        
            set wordEnd [$twin index "$res + $length chars"]
            $twin tag add $tagClass $res $wordEnd
            $twin tag configure $tagClass -foreground $color
            set si $wordEnd
            
            incr numTimesLooped
            if {$numTimesLooped >= $numUntilUpdate} {
                ctext::update
                set numTimesLooped 0
            }
        }
    }
}

proc ctext::linemapToggleMark {win y} {
    ctext::getAr $win config configAr
    
    if {!$configAr(-linemap_markable)} {
        return
    }
    
    set markChar [$win.l index @0,$y] 
    set lineSelected [lindex [split $markChar .] 0]
    set line [$win.l get $lineSelected.0 $lineSelected.end]

    if {$line == ""} {
        return
    }

    ctext::getAr $win linemap linemapAr
    
    if {[info exists linemapAr($line)] == 1} { 
        #It's already marked, so unmark it.
        array unset linemapAr $line
        ctext::linemapUpdate $win
        set type unmarked
    } else {
        #This means that the line isn't toggled, so toggle it.
        array set linemapAr [list $line {}]
        $win.l tag add lmark $markChar [$win.l index "$markChar lineend"] 
        $win.l tag configure lmark -foreground $configAr(-linemap_select_fg) \
-background $configAr(-linemap_select_bg)
        set type marked
    }

    if {[string length $configAr(-linemap_mark_command)]} {
        uplevel #0 [linsert $configAr(-linemap_mark_command) end $win $type $line]
    }
}

#args is here because -yscrollcommand may call it
proc ctext::linemapUpdate {win args} {
    if {[winfo exists $win.l] != 1} { 
        return
    }

    set pixel 0
    set lastLine {}
    set lineList [list]
    set fontMetrics [font metrics [$win._t cget -font]]
    set incrBy [expr {1 + ([lindex $fontMetrics 5] / 2)}]

    while {$pixel < [winfo height $win.l]} {
        set idx [$win._t index @0,$pixel]

        if {$idx != $lastLine} {
            set line [lindex [split $idx .] 0]
            set lastLine $idx
            $win.l config -width [string length $line]
            lappend lineList $line
        }
        incr pixel $incrBy 
    } 

    ctext::getAr $win linemap linemapAr
    
    $win.l delete 1.0 end
    set lastLine {}
    foreach line $lineList {
        if {$line == $lastLine} {
            $win.l insert end "\n" 
        } else {
            if {[info exists linemapAr($line)]} { 
                $win.l insert end "$line\n" lmark
            } else {
                $win.l insert end "$line\n"
            }
        }
        set lastLine $line
    }
}

proc ctext::modified {win value} {
    ctext::getAr $win config ar
    set ar(modified) $value
    event generate $win <<Modified>>
    return $value
}
# This file is part of IFM, the Interactive Fiction Mapper, copyright (C)
# Glenn Hutchings 1997-2006.

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
