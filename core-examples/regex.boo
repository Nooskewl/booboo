vector strings
vector_add strings "A lot of burgers"
vector_add strings "My favourite is #00D8FF!"
vector_add strings "I like #FFD800 and #FF00D8"

string colour_regex colour_capture
= colour_regex "#[A-Fa-f0-9]{4}"
= colour_capture ".*(#[A-Fa-f0-9]{4}).*(#[A-Fa-f0-9]{4}).*"

number i

for i 0 (< i 3) 1 search
	number found
	string_matches found [strings i] colour_regex
	print "%: %\n" [strings i] found
:search

string_replace [strings 0] "A lot of (.*)" "I don't like $1"

print "%\n" [strings 0]
	
vector v
string_match v [strings 2] colour_capture

number j
number sz
vector_size v sz
for j 0 (< j sz) 1 print_colours
	print "	%\n" [v j]
:print_colours
