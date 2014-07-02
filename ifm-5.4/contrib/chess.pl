#! /usr/bin/env perl

# Write a chess board in IFM format.

@pieces = ("Rook", "Knight", "Bishop", "Queen",
	   "King", "Bishop", "Knight", "Rook");

# Print header.
print <<END;
title "Chessboard";

show_map_border = 1;

room_width = 0.9;
room_height = 0.9;

room_text_fontsize = 14;
item_text_fontsize = 10;

room_colour = "gray100" in style Light;
room_colour = "gray60" in style Dark;
END

for $row (1 .. 8) {
    # Toggle direction of room placement.
    $forward = !$forward;

    for $num (1 .. 8) {
	# Toggle dark/light squares.
	$dark = !$dark;

	# Get column number.
	$col = $forward ? $num : 9 - $num;

	# Get room direction.
	if ($num == 1) {
	    $dir = ($row == 1 ? undef : "n");
	} else {
	    $dir = ($row % 2 ? "e" : "w");
	}

	# Print room.
	$tag = substr("ABCDEFGH", $col - 1, 1) . $row;
	print "\nroom \"$tag\" tag $tag style ";
	print $dark ? "Dark" : "Light";
	print " dir $dir" if $dir;
	print ";\n";

	# Print piece, if any.
	if ($row <= 2) {
	    $piece = "White ";
	    $piece .= ($row == 1 ? $pieces[$col - 1] : "Pawn");
	} elsif ($row >= 7) {
	    $piece = "Black ";
	    $piece .= ($row == 8 ? $pieces[$col - 1] : "Pawn");
	} else {
	    undef $piece;
	}

	print "  item \"$piece\";\n" if $piece;

	# Link room with others below it.
	$rooms[$row][$col] = $tag;

	for ($i = -1; $i <= 1; $i++) {
	    $other = $rooms[$row - 1][$col + $i];
	    print "  link last to $other;\n" if $other;
	}
    }
}
