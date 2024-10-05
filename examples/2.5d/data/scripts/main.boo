number tree
image_load tree "tree.png"

vector billboards

number num_billboards
= num_billboards 100

number i
for i 0 (< i num_billboards) 1 loop
	number b
	number x
	rand x 0 1000
	- x 500
	/ x 500
	* x 10
	number z
	rand z 0 1000
	- z 500
	/ z 500
	* z 10
	billboard_create b tree x 0.4 z 0.55 0.8
	vector_add billboards b
	:loop

number sprite
sprite_load sprite "pleasant"

number pleasant
billboard_from_sprite pleasant sprite 0 0.25 -1 0 0 32

number texture
image_load texture "texture.png"

vector positions
vector_add positions -1000
vector_add positions -0.01
vector_add positions -1000
vector_add positions -1000
vector_add positions -0.01
vector_add positions 1000
vector_add positions 1000
vector_add positions -0.01
vector_add positions -1000
vector_add positions -1000
vector_add positions -0.01
vector_add positions 1000
vector_add positions 1000
vector_add positions -0.01
vector_add positions -1000
vector_add positions 1000
vector_add positions -0.01
vector_add positions 1000
vector texcoords
vector_add texcoords 0
vector_add texcoords 0
vector_add texcoords 0
vector_add texcoords 10000
vector_add texcoords 10000
vector_add texcoords 0
vector_add texcoords 0
vector_add texcoords 10000
vector_add texcoords 10000
vector_add texcoords 0
vector_add texcoords 10000
vector_add texcoords 10000
vector faces
vector_add faces 0
vector_add faces 1
vector_add faces 2
vector_add faces 3
vector_add faces 4
vector_add faces 5
vector colours
	
set_3d

number angle
= angle 0
number anglex
= anglex 0

number x y z
= x 0
= y 0
= z 0

mouse_set_relative TRUE

function draw
{
	clear 100 100 255
	
	draw_3d_textured texture positions faces colours texcoords 2

	number i
	for i 0 (< i num_billboards) 1 loop
		billboard_draw [billboards i] 255 255 255 255
	:loop

:again_neg
	? angle 0
	jge done_neg
	+ angle (* PI 2)
	goto again_neg
:done_neg
:again_pos
	? angle (* PI 2)
	jle done_pos
	- angle (* PI 2)
	goto again_pos
:done_pos

	if (|| (< angle (* PI 0.25)) (> angle (* PI 1.75))) front (< angle (* PI 0.75)) right (< angle (* PI 1.25)) back left
		sprite_set_animation sprite "stand_s"
	:front
		sprite_set_animation sprite "stand_e"
	:right
		sprite_set_animation sprite "stand_n"
	:back
		sprite_set_animation sprite "stand_w"
	:left

	billboard_draw pleasant 255 255 255 255
}

function eabs x
{
	if (< x 0) switch
		neg x
	:switch
	return x
}

function run
{
	include "poll_joystick.inc"

	number dx dy
	mouse_get_delta dx dy
	if (!= 0 dx) set_x2
		= joy_x2 (/ dx 10)
	:set_x2
	if (!= 0 dy) set_y2
		= joy_y2 (/ dy 10)
	:set_y2

	number w a s d
	key_get w KEY_w
	key_get a KEY_a
	key_get s KEY_s
	key_get d KEY_d
	if (!= 0 w) set_forward
		= joy_y1 -1
	:set_forward
	if (!= 0 s) set_backward
		= joy_y1 1
	:set_backward
	if (!= 0 a) set_strafe_left
		= joy_x1 -1
	:set_strafe_left
	if (!= 0 d) set_strafe_right
		= joy_x1 1
	:set_strafe_right

	if (< (eabs joy_x1) 0.1) clip_x1
		= joy_x1 0
	:clip_x1
	if (< (eabs joy_x2) 0.1) clip_x2
		= joy_x2 0
	:clip_x2
	if (< (eabs joy_y1) 0.1) clip_y1
		= joy_y1 0
	:clip_y1
	if (< (eabs joy_y2) 0.1) clip_y2
		= joy_y2 0
	:clip_y2

	neg joy_x1
	neg joy_y1

	/ joy_x1 10
	/ joy_x2 10
	/ joy_y1 10
	/ joy_y2 10

	+ angle joy_x2
	+ anglex joy_y2

	number xi yi zi
	= xi (+ angle (/ PI 2))
	cos xi
	= xi (* xi joy_y1)
	= yi 0
	= zi (+ angle (/ PI 2))
	sin zi
	= zi (* zi joy_y1)

	number xi2 yi2 zi2
	= xi2 angle
	cos xi2
	= xi2 (* xi2 joy_x1)
	= yi2 0
	= zi2 angle
	sin zi2
	= zi2 (* zi2 joy_x1)

	+ x xi xi2
	+ y yi yi2
	+ z zi zi2

	identity_3d
	rotate_3d anglex 1 0 0
	rotate_3d angle 0 1 0
	translate_3d x (+ y -0.1) z
}
