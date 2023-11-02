number TSZ
= TSZ 16 ; should be a power of 2

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
int p
vector_add v p
= p i
/ p 85
* p 255
int p
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
int p
vector_add v p
= p i
/ p 85
* p 255
int p
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
int p
vector_add v p
vector_add v 0
= p i
/ p 86
number inv
= inv 1
- inv p
= p inv
* p 255
int p
vector_add v p
vector_add palette v
+ i 1
? i 86
jl next_colour3

number W
= W 640
/ W TSZ
int W
+ W 1

number H
= H 360
/ H TSZ
int H
+ H 1

; generate the initial randomly-coloured TSZxTSZ blocks
vector frame
number y
= y 0
:next_y
vector row
number x
= x 0
:next_x
number r
rand r 0 255
vector_add row r
+ x 1
? x W
jl next_x
vector_add frame row
+ y 1
? y H
jl next_y

; You can subdivide this down to 1 pixel, but even two subdivisions is kinda slow...
call_result frame divide frame W H
* W 2
* H 2
call_result frame divide frame W H
* W 2
* H 2
;call_result frame divide frame W H
;* W 2
;* H 2
;call_result frame divide frame W H
;* W 2
;* H 2

number ticks
= ticks 0

function divide frame w h
{
	number new_w
	number new_h
	= new_w w
	* new_w 2
	= new_h h
	* new_h 2
	vector v
	vector empty_row
	number i
	= i 0
:next_fill
	vector_add empty_row 0
	+ i 1
	? i new_w
	jl next_fill

	= i 0
:next_row_fill
	vector_add v empty_row
	+ i 1
	? i new_h
	jl next_row_fill

	number y
	number x
	= y 0
:next_y
	= x 0
:next_x
	number r
	= r 0
	number g
	= g 0
	number b
	= b 0
	number valid
	= valid 0
	number xx
	number yy
	number xx2
	number yy2
	= yy y
	- yy 1
	= yy2 yy
	+ yy2 3
:next_pixel_y
	= xx x
	- xx 1
	= xx2 xx
	+ xx2 3
	number tmpy
	= tmpy yy
	/ tmpy 2
	int tmpy
:next_pixel_x
	+ valid 1
	number tmpx
	= tmpx xx
	/ tmpx 2
	int tmpx
	? tmpx 0
	jl skip_it
	? tmpx w
	jge skip_it
	? tmpy 0
	jl skip_it
	? tmpy h
	jge skip_it
	vector row
	vector_get frame row tmpy
	number pal_index
	vector_get row pal_index tmpx
	number rr
	number gg
	number bb
	vector colour
	vector_get palette colour pal_index
	vector_get colour rr 0
	vector_get colour gg 1
	vector_get colour bb 2
	+ r rr
	+ g gg
	+ b bb
:skip_it
	+ xx 1
	? xx xx2
	jl next_pixel_x
	+ yy 1
	? yy yy2
	jl next_pixel_y
	/ r valid
	/ g valid
	/ b valid
	int r
	int g
	int b
	number best_fit
	call_result best_fit find_closest r g b
	vector row
	vector_get v row y
	vector_set row x best_fit
	vector_set v y row
	+ x 1
	? x new_w
	jl next_x
	+ y 1
	? y new_h
	jl next_y
	
	return v
}

function find_closest r g b
{
	number best_i
	number smallest_diff
	= best_i 0
	= smallest_diff 255
	number i
	= i 0
:next_loop
	vector colour
	vector_get palette colour i
	number rr
	number gg
	number bb
	vector_get colour rr 0
	vector_get colour gg 1
	vector_get colour bb 2
	- rr r
	- gg g
	- bb b
	abs rr rr
	abs gg gg
	abs bb bb
	* rr rr
	* gg gg
	* bb bb
	* rr 0.2126
	* gg 0.7152
	* bb 0.0722
	+ rr gg
	+ rr bb
	sqrt rr rr
	? rr smallest_diff
	jge not_better
	= smallest_diff rr
	= best_i i
:not_better
	+ i 1
	? i 256
	jl next_loop

	return best_i
}

function max a b
{
	? a b
	jle b_max
	return a
:b_max
	return b
}

function draw
{
	number xsz
	= xsz 640
	/ xsz W
	number ysz
	= ysz 360
	/ ysz H

	start_primitives

	number w
	number h

	vector_size frame h
	vector row
	vector_get frame row 0
	vector_size row w

	number y
	= y 0
:next_y
	number x
	= x 0
:next_x
	vector row
	vector_get frame row y
	number pal_index
	vector_get row pal_index x
	+ pal_index ticks
	% pal_index 256
	vector colour
	vector_get palette colour pal_index
	number r
	number g
	number b
	vector_get colour r 0
	vector_get colour g 1
	vector_get colour b 2
	number xx
	number yy
	= yy y
	* yy ysz
	= xx x
	* xx xsz
	filled_rectangle r g b 255 r g b 255 r g b 255 r g b 255 xx yy xsz ysz
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
	+ ticks 1
}
