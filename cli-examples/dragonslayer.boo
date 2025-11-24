print "After a long, arduous journey to the\n"
print "core of the Earth, you come upon the Dragon's layer...\n"
print "\n"
print "Do you slay the beast? ("
text_colour GREEN 1 BLACK 0
print "y"
text_reset
print "/"
text_colour RED 0 BLACK 0
print "N"
text_reset
print "): "
var response
= response (getch)
var s
= s (string_from_number response)
print "\n\n"
if (== s "y") slay dead
	text_colour GREEN 1 GREEN 0
	print "Rejoice! The beast is slain!"
:slay
	text_colour RED 1 RED 0
	print "Alas, in your hesitation, thou hast been consumed by flames!"
:dead
text_reset
print "\n"
