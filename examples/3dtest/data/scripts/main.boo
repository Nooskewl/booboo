number model

model_load model "zeus.x"

model_translate model 0 0 -2.5

number sx sy sz

model_size model sx sy sz
number scale
number max_
= max_ sx
if (> sy max_) change_y
	= max_ sy
:change_y
if (> sz max_) change_z
	= max_ sz
:change_z
= scale (/ 1.0 max_)
model_scale model scale scale scale

model_set_animation model "ArmatureAction" cb

number font
font_load font "font.ttf" 48 0

function cb
{
;	model_stop model
;	model_reset model
}

function draw
{
	clear 100 100 255

	set_3d
	model_draw model 255 255 255 255

	set_2d
	font_draw font 255 255 0 255 "Testing..." 50 100
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

	neg joy_y1

	if (< (eabs joy_x1) 0.1) zero_x
		= joy_x1 0
	:zero_x
	if (< (eabs joy_y1) 0.1) zero_y
		= joy_y1 0
	:zero_y
	if (< (eabs joy_x2) 0.1) zero_z
		= joy_x2 0
	:zero_z

	model_translate model joy_x1 joy_y1 joy_x2
}
