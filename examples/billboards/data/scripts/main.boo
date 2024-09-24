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
	billboard_create b tree x 0 z 0.4 0.8
	vector_add billboards b
:loop
	
set_3d

number angle
= angle 0

number x y z
= x 0
= y 0
= z 0

function draw
{
	clear 100 100 255

	number i
	for i 0 (< i num_billboards) 1 loop
		billboard_draw [billboards i] 255 255 255 255
	:loop
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

	if (< (eabs joy_x1) 0.1) clip_x1
		= joy_x1 0
	:clip_x1
	if (< (eabs joy_y1) 0.1) clip_y1
		= joy_y1 0
	:clip_y1
	if (< (eabs joy_y2) 0.1) clip_y2
		= joy_y2 0
	:clip_y2

	neg joy_y1

	/ joy_x1 100
	/ joy_y1 25
	/ joy_y2 25

	+ angle joy_x1

	number xi yi zi
	= xi (+ angle (/ PI 2))
	cos xi
	= xi (* xi joy_y1)
	= yi joy_y2
	= zi (+ angle (/ PI 2))
	sin zi
	= zi (* zi joy_y1)

	+ x xi
	+ y yi
	+ z zi

	identity_3d
	rotate_3d 0 angle 0
	translate_3d x y z
}
