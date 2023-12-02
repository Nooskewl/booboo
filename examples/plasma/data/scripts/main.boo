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

number W
= W 640
/ W TSZ
floor W
+ W 1

number H
= H 360
/ H TSZ
floor H
+ H 1

; generate the initial randomly-coloured TSZxTSZ blocks
vector frame
number y
for y 0 (< y H) 1 next_y
vector row
number x
for x 0 (< x W) 1 next_x
number r
rand r 0 255
vector_add row r
:next_x
vector_add frame row
:next_y

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
	for i 0 (< i new_w) 1 next_fill
	vector_add empty_row 0
:next_fill

	for i 0 (< i new_h) 1 next_row_fill
	vector_add v empty_row
:next_row_fill

	number y
	number x
	for y 0 (< y new_h) 1 next_y
	for x 0 (< x new_w) 1 next_x
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
	for yy (- y 1) (< yy (+ y 3)) 1 next_pixel_y
	number tmpy
	= tmpy (/ yy 2)
	floor tmpy
	for xx (- x 1) (< xx (+ x 3)) 1 next_pixel_x
	+ valid 1
	number tmpx
	= tmpx (/ xx 2)
	floor tmpx
	? tmpx 0
	jl skip_it
	? tmpx w
	jge skip_it
	? tmpy 0
	jl skip_it
	? tmpy h
	jge skip_it
	number pal_index
	vector_get frame pal_index tmpy tmpx
	number rr
	number gg
	number bb
	vector_get palette rr pal_index 0
	vector_get palette gg pal_index 1
	vector_get palette bb pal_index 2
	+ r rr
	+ g gg
	+ b bb
:skip_it
:next_pixel_x
:next_pixel_y
	/ r valid
	/ g valid
	/ b valid
	floor r
	floor g
	floor b
	number best_fit
	call_result best_fit find_closest r g b
	vector_set v y x best_fit
:next_x
:next_y
	
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
	number rr
	number gg
	number bb
	vector_get palette rr i 0
	vector_get palette gg i 1
	vector_get palette bb i 2
	- rr r
	- gg g
	- bb b
	abs rr
	abs gg
	abs bb
	* rr rr
	* gg gg
	* bb bb
	* rr 0.2126
	* gg 0.7152
	* bb 0.0722
	+ rr gg
	+ rr bb
	sqrt rr
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
	number x
	number r
	number g
	number b
	number pal_index
	for y 0 (< y h) 1 next_y
		for x 0 (< x w) 1 next_x
			vector_get frame pal_index y x
			+ pal_index ticks
			% pal_index 256
			vector_get palette r pal_index 0
			vector_get palette g pal_index 1
			vector_get palette b pal_index 2
			filled_rectangle r g b 255 r g b 255 r g b 255 r g b 255 (* x xsz) (* y ysz) xsz ysz
:next_x
:next_y

	end_primitives
}

function run
{
	+ ticks 1
}
