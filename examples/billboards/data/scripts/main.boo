var tree
= tree (image_load "tree.png")

var billboards

var num_billboards
= num_billboards 100

var i
for i 0 (< i num_billboards) 1 loop
	var b
	var x
	= x (* (/ (- (rand 0 1000) 500) 500) 10)
	var z
	= z (* (/ (- (rand 0 1000) 500) 500) 10)
	= b (billboard_create tree x 0 z 0.55 0.8)
	vector_add billboards b
:loop
	
set_3d

var angle
= angle 0
var anglex
= anglex 0

var x y z
= x 0
= y 0
= z 0

mouse_set_relative TRUE

function draw
{
	clear 100 100 255

	var i
	for i 0 (< i num_billboards) 1 loop
		billboard_draw [billboards i] 255 255 255 255
	:loop
}

function eabs x
{
	if (< x 0) switch
		= x (neg x)
	:switch
	return x
}

function run
{
	include "poll_joystick.inc"

	var dx dy
	explode (mouse_get_delta) dx dy
	if (!= 0 dx) set_x2
		= joy_x2 (/ dx 10)
	:set_x2
	if (!= 0 dy) set_y2
		= joy_y2 (/ dy 10)
	:set_y2

	var w a s d
	= w (key_get KEY_W)
	= a (key_get KEY_A)
	= s (key_get KEY_S)
	= d (key_get KEY_D)
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

	= joy_x1 (neg joy_x1)
	= joy_y1 (neg joy_y1)

	= joy_x1 (/ joy_x1 10)
	= joy_x2 (/ joy_x2 10)
	= joy_y1 (/ joy_y1 10)
	= joy_y2 (/ joy_y2 10)

	= angle (+ angle joy_x2)
	= anglex (+ anglex joy_y2)

	if (< anglex (* -1 (/ PI 2))) cliplow (> anglex (/ PI 2)) cliphigh
		= anglex (* -1 (/ PI 2))
	:cliplow
		= anglex (/ PI 2)
	:cliphigh

	var xi yi zi
	= xi (+ angle (/ PI 2))
	= xi (cos xi)
	= xi (* xi joy_y1)
	= yi 0
	= zi (+ angle (/ PI 2))
	= zi (sin zi)
	= zi (* zi joy_y1)

	var xi2 yi2 zi2
	= xi2 angle
	= xi2 (cos xi2)
	= xi2 (* xi2 joy_x1)
	= yi2 0
	= zi2 angle
	= zi2 (sin zi2)
	= zi2 (* zi2 joy_x1)

	= x (+ x xi xi2)
	= y (+ y yi yi2)
	= z (+ z zi zi2)

	identity_3d
	rotate_3d anglex 1 0 0
	rotate_3d angle 0 1 0
	translate_3d x y z
}
