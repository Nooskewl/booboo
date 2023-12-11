vector frame

number w
= w 320
number h
= h 180

resize w h

vector row

number i
= i 0
:next_row
vector_clear row
number j
= j 0
:next_col
number tmp
= tmp j
/ tmp w
* tmp PI
* tmp 2
sin tmp
number tmp2
= tmp2 i
/ tmp2 h
cos tmp2
number tmp3
= tmp3 i
/ tmp3 h
* tmp3 PI
sin tmp3
+ tmp tmp2
+ tmp tmp3
/ tmp 3
abs tmp
* tmp 255
floor tmp
vector_add row tmp
+ j 1
? j w
jl next_col
vector_add frame row
+ i 1
? i h
jl next_row

vector palette
number i

; red to green
= i 0
:next_colour
vector v
number p
= p i
/ p 85
number inv
= inv 1
- inv p
= p inv
* p 255
floor p
vector_add v p
= p i
/ p 85
* p 255
floor p
vector_add v p
vector_add v 0
vector_add palette v
+ i 1
? i 85
jl next_colour

; green to blue
= i 0
:next_colour2
vector v
vector_add v 0
number p
= p i
/ p 85
number inv
= inv 1
- inv p
= p inv
* p 255
floor p
vector_add v p
= p i
/ p 85
* p 255
floor p
vector_add v p
vector_add palette v
+ i 1
? i 85
jl next_colour2

; blue to red
= i 0
:next_colour3
vector v
number p
= p i
/ p 86
* p 255
floor p
vector_add v p
vector_add v 0
= p i
/ p 86
number inv
= inv 1
- inv p
= p inv
* p 255
floor p
vector_add v p
vector_add palette v
+ i 1
? i 86
jl next_colour3

number t
= t 0

function draw
{
	start_primitives

	number y
	number x

	= y 0
:next_y
	= x 0
:next_x
	number c
	vector_get frame c y x
	+ c t
	% c 256
	number r
	number g
	number b
	vector_get palette r c 0
	vector_get palette g c 1
	vector_get palette b c 2
	filled_rectangle r g b 255 r g b 255 r g b 255 r g b 255 x y 1 1
	+ x 1
	? x w
	jl next_x
	+ y 1
	? y h
	jl next_y

	end_primitives
}

function run
{
	+ t 1
}
