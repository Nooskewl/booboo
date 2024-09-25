number level
model_load level "level.x"
model_rotate level PI 1 0 0

set_3d

number angle
= angle 0
number angle2
= angle2 0

number x y z
= x 0
= y -10000
= z 0

number inited
= inited 0

function draw
{
	clear 100 100 255

	identity_3d
	scale_3d 0.1 0.1 0.1
	rotate_3d angle2 1 0 0
	rotate_3d angle 0 1 0
	translate_3d x y z

	model_draw level 255 255 255 255
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

	* joy_x1 5
	/ joy_x2 10
	* joy_y1 5
	/ joy_y2 10

	+ angle joy_x2
	+ angle2 joy_y2

	number xi zi
	= xi (+ angle (/ PI 2))
	cos xi
	= xi (* xi joy_y1)
	= zi (+ angle (/ PI 2))
	sin zi
	= zi (* zi joy_y1)

	number xi2 zi2
	= xi2 angle
	cos xi2
	= xi2 (* xi2 joy_x1)
	= zi2 angle
	sin zi2
	= zi2 (* zi2 joy_x1)

	number incx incy incz
	= incx (+ xi xi2)
	= incz (+ zi zi2)

	+ x incx
	+ z incz

	if (|| (!= incx 0) (!= incz 0) (== inited 0)) check_coll
		number y1i y2i
		= y1i (- y 1000000)
		= y2i (+ y 1000000);

		number col out_x out_y out_z
		cd_model_line_segment col level x y1i z x y2i z out_x out_y out_z

		if (== col 1) collided
			= y out_y
		:collided

		= inited 1
	:check_coll
}
