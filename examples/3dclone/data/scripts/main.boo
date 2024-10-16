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

model_set_animation model "ArmatureAction" cb

number clone1 clone2

model_clone model clone1
model_clone model clone2

model_translate clone1 1 0 0
model_translate clone2 -1 0 0

model_scale model scale scale scale
model_scale clone1 scale scale scale
model_scale clone2 scale scale scale

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
	model_draw clone1 255 255 255 255
	model_draw clone2 255 255 255 255
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
