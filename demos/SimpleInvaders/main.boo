vector rocks
vector bullets
vector buildings

number i
for i 0 640 20 add_building
vector_add buildings 1
:add_building

number px next_rock next_fire ticks prev_a bottom
= px 320
= next_rock 60
= next_fire 0
= ticks 0
= prev_a 0
= bottom 300

number sfx_hit sfx_fire
mml_create sfx_hit "A @TYPE1 c8"
mml_create sfx_fire "@PO0 = { 0 100 }\nA @PO0 g32 @PO0"

function draw
{
	filled_triangle 255 255 0 255 255 255 0 255 255 255 0 255 (- px 10) bottom (+ px 10) bottom px (- bottom 20)

	number sz
	vector_size buildings sz
	number i
	for i 0 sz 1 next_building
	? 1 [buildings i]
	jne next_building
	filled_rectangle 0 255 0 255 0 255 0 255 0 255 0 255 0 255 0 255 (* i 20) (+ bottom 20) 20 10
:next_building

	vector_size rocks sz
	for i 0 sz 1 next_rock
	filled_circle 255 0 255 255 (+ 10 (* [rocks i 0] 20)) [rocks i 1] 8 -1
:next_rock

	number sz
	vector_size bullets sz
	for i 0 sz 1 next_bullet
	filled_circle 255 0 0 255 [bullets i 0] [bullets i 1] 4 -1
:next_bullet
}

function run
{
	+ ticks 1

	include "poll_joystick.inc"

	? 1 (&& (!= prev_a joy_a) (== joy_a 1) (>= ticks next_fire))
	jne no_a

	= next_fire (+ ticks 15)

	mml_play sfx_fire 1 0
	vector bullet
	vector_add bullet px
	vector_add bullet (- bottom 20)
	vector_add bullets bullet

:no_a

	? joy_l 1
	jne no_l

	- px 3
	? px 0
	jge no_l
	= px 0

:no_l

	? joy_r 1
	jne no_r

	+ px 3
	? px 639
	jle no_r
	= px 639

:no_r

	= prev_a joy_a

	? 1 (>= ticks next_rock)
	jne no_rock

	number r
	rand r 0 (- (/ 640 20) 1)
	vector rock
	vector_add rock r
	vector_add rock -20
	vector_add rocks rock
	= next_rock (+ ticks 180)

:no_rock

	; Check if rocks hit buildings

	number sz
	vector_size rocks sz
	for i 0 sz 1 check_hit_building
	vector rock
	vector_get rocks rock i
	? 1 (>= [rock 1] (+ bottom 20))
	jne check_hit_building
	? 1 [buildings [rock 0]]
	jne check_hit_building
	mml_play sfx_hit 1 0
	= [buildings [rock 0]] 0
:check_hit_building

	? sz 0
	je no_rocks
	; Update rocks
	= i 0
:update_next_rock
	+ [rocks i 1] 0.5
	? [rocks i 1] 360
	jle next_loop
	vector_erase rocks i
	- i 1
	- sz 1
:next_loop
	+ i 1
	? i sz
	jl update_next_rock
:no_rocks

	; Update bullets

	vector_size bullets sz
	? sz 0
	je no_bullets
	= i 0
:update_next_bullet
	- [bullets i 1] 5
	? [bullets i 1] 0
	jge check_hits
:erase_bullet
	vector_erase bullets i
	- i 1
	- sz 1
	goto next_loop2
:check_hits
	number j
	= j 0
	number sz2
	vector_size rocks sz2
	? sz2 0
	je next_loop2
:next_hit_check
	number dx dy
	= dx (- [bullets i 0] (+ 10 (* 20 [rocks j 0])))
	= dy (- [bullets i 1] [rocks j 1])
	* dx dx
	* dy dy
	+ dx dy
	sqrt dx
	? dx 12
	jge not_a_hit
	mml_play sfx_hit 1 0
	vector_erase rocks j
	goto erase_bullet
:not_a_hit
	+ j 1
	? j sz2
	jl next_hit_check
:next_loop2
	+ i 1
	? i sz
	jl update_next_bullet
:no_bullets
}
