var strings
vector_add strings "A lot of burgers"
vector_add strings "My favourite is #00D8FF!"
vector_add strings "I like #FFD800 and #FF00D8"

var colour_regex colour_capture
= colour_regex "#[A-Fa-f0-9]{6}"
= colour_capture ".*(#[A-Fa-f0-9]{6}).*(#[A-Fa-f0-9]{6}).*"

var i

for i 0 (< i 3) 1 search
	var found
	= found (string_matches [strings i] colour_regex)
	print "%: %\n" [strings i] found
:search

= [strings 0] (string_replace [strings 0] "A lot of (.*)" "I don't like $1")

print "%\n" [strings 0]
	
var v
= v (string_match [strings 2] colour_capture)

var j sz
= sz (vector_size v)
for j 0 (< j sz) 1 print_colours
	print "	%\n" [v j]
:print_colours
