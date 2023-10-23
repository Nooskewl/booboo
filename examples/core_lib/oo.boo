number random
= random 12309481233

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
pointer p
address p circle
vector_add v p
address p square
vector_add v p
address p triangle
vector_add v p

vector shapes

map m

number i
= i 0
:next_loop
number r
call_result r genrand
fmod r 3
int r
pointer p
vector_get v p r
map_set m "draw" p
vector_add shapes m
+ i 1
? i 10
jl next_loop

number i
= i 0
:next_loop2
map m
vector_get shapes m i
pointer p
map_get m p "draw"
call p
+ i 1
? i 10
jl next_loop2
