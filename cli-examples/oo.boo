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
map_set m "draw" [v (rand 0 2)]
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
