number random
= random 12309481233

; this is a really crappy random number generator since the random number generator is currently in the game library
function genrand
{
	number tmp
	= tmp 75
	* tmp random
	+ tmp 74
	? tmp 0
	jge ok
	neg tmp
:ok
	= random tmp
	return random
}

function circle
{
	print "Circle\n"
}

function square
{
	print "Square\n"
}

function triangle
{
	print "Triangle\n"
}

vector v
vector_add v circle
vector_add v square
vector_add v triangle

vector shapes

map m

number i
= i 0
:next_loop
number r
call_result r genrand
fmod r 3
floor r
map_set m "draw" [v r]
vector_add shapes m
+ i 1
? i 10
jl next_loop

number i
= i 0
:next_loop2
map m
vector_get shapes m i
call [m "draw"]
+ i 1
? i 10
jl next_loop2
