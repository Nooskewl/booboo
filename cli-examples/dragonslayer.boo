print "After a long, arduous journey to the\n"
print "core of the Earth, you come upon the Dragon's layer...\n"
print "\n"
print "Do you slay the beast? ("
text_fore GREEN 1
print "y"
text_reset
print "/"
text_fore RED 0
print "N"
text_reset
print "): "
var response
= response (getch)
var s
= s (string_from_number response)
print "\n\n"
if (== s "y") slay dead
	text_fore GREEN 1
	text_back GREEN 0
	print "Rejoice! The beast is slain!"
:slay
	text_fore RED 1
	text_back RED 0
	print "Alas, in your hesitation, thou hast been consumed by flames!"
:dead
text_reset
print "\n"
