srand(time)

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
for i 0 (< i 10) 1 next_loop
	= [m "draw"] [v (rand 0 2)]
	vector_add shapes m
:next_loop

for i 0 (< i 10) 1 next_loop2
	map m
	= m [shapes i]
	call [m "draw"]
:next_loop2
