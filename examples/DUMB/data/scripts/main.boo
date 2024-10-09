number SPEED
= SPEED 0.01

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

vector enemies enemy

call spawn_enemy

number SPAWN_TIME next_spawn
= SPAWN_TIME 180
= next_spawn 0

number texture
image_load texture "texture.png"

vector positions
vector_add positions -1000 -0.01 -1000
vector_add positions -1000 -0.01 1000
vector_add positions 1000 -0.01 -1000
vector_add positions -1000 -0.01 1000
vector_add positions 1000 -0.01 -1000
vector_add positions 1000 -0.01 1000
vector texcoords
vector_add texcoords 0 0
vector_add texcoords 0 10000
vector_add texcoords 10000 0
vector_add texcoords 0 10000
vector_add texcoords 10000 0
vector_add texcoords 10000 10000
vector faces
vector_add faces 0 1 2
vector_add faces 3 4 5
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

function spawn_enemy
{
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

	number sprite
	sprite_load sprite "pleasant"

	number pleasant
	billboard_from_sprite pleasant sprite x 0.25 z 0 0 32

	vector_init enemy sprite pleasant x 0 z 0 0 0

	vector_add enemies enemy

	number sz
	vector_size enemies sz
	- sz 1

	call set_dest [enemies sz]
}


function set_dest e
{
	number r
	rand r 0 1000
	- r 500
	/ r 500
	* r 10
	= [e 5] r
	rand r 0 1000
	- r 500
	/ r 500
	* r 10
	= [e 7] r
}

function move_enemy e
{
	vector v1 v2 diff tmp
	vector_init v1 [e 2] [e 3] [e 4]
	vector_init v2 [e 5] [e 6] [e 7]
	= diff (vsub v2 v1)
	if (< (vlen diff) 0.1) done
		call set_dest e
		return
	:done
	= tmp (vmul (normalize diff) SPEED)
	+ [e 2] [tmp 0]
	+ [e 4] [tmp 2]
	billboard_translate [e 1] [tmp 0] 0 [tmp 2]
}

function normalize_angle a
{
	number tmp
	? a 0
	jge done_neg
	= tmp a
	neg tmp
	/ tmp (* PI 2)
	floor tmp
	+ tmp 1
	+ a (* tmp (* PI 2))
:done_neg
	? a (* PI 2)
	jl done_pos
	= tmp a
	/ tmp (* PI 2)
	floor tmp
	- a (* tmp (* PI 2))
:done_pos
	return a
}

function draw
{
	clear 100 100 255
	
	draw_3d_textured texture positions faces colours texcoords 2

	number i
	for i 0 (< i num_billboards) 1 loop
		billboard_draw [billboards i] 255 255 255 255
	:loop

	number sz
	vector_size enemies sz

	for i 0 (< i sz) 1 loop2
		vector tmp1 tmp2 tmp3 tmp4
		number angle2

		vector_init tmp1 [enemies i 2] [enemies i 3] [enemies i 4]
		vector_init tmp2 [enemies i 5] [enemies i 6] [enemies i 7]
		vector_init tmp3 0 0 1
		= tmp4 (vsub tmp2 tmp1)
		
		= angle2 (vangle tmp4 tmp3)

		vector cross plane
		= cross (cross tmp4 tmp3)
		vector_init plane 0 1 0
		if (< (dot plane cross) 0) negate
			neg angle2
		:negate

		call_result angle normalize_angle angle

		number a
		= a (- angle2 angle)

		call_result a normalize_angle a

		if (|| (< a (* PI 0.25)) (> a (* PI 1.75))) front (< a (* PI 0.75)) left (< a (* PI 1.25)) back right
			sprite_set_animation_lazy [enemies i 0] "walk_s"
		:front
			sprite_set_animation_lazy [enemies i 0] "walk_w"
		:left
			sprite_set_animation_lazy [enemies i 0] "walk_n"
		:back
			sprite_set_animation_lazy [enemies i 0] "walk_e"
		:right

		billboard_draw [enemies i 1] 255 255 255 255
	:loop2
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

	if (< anglex (* -1 (/ PI 2))) cliplow (> anglex (/ PI 2)) cliphigh
		= anglex (* -1 (/ PI 2))
	:cliplow
		= anglex (/ PI 2)
	:cliphigh

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

	if (< x -10) incx
		= x -10
	:incx
	if (> x 10) decx
		= x 10
	:decx
	if (< z -10) incz
		= z -10
	:incz
	if (> z 10) decz
		= z 10
	:decz

	identity_3d
	rotate_3d anglex 1 0 0
	rotate_3d angle 0 1 0
	translate_3d x (+ y -0.1) z

	number sz i
	vector_size enemies sz
	for i 0 (< i sz) 1 loop
		call move_enemy [enemies i]
	:loop

	+ next_spawn 1
	if (>= next_spawn SPAWN_TIME) spawn
		call spawn_enemy
		= next_spawn 0
	:spawn
}
