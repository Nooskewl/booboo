number model

model_load model "zeus.x"
model_translate model 0 0 -2

number sx sy sz

model_size model sx sy sz
number scale
number max
= max sx
if (> sy max) change_y
	= max sy
:change_y
if (> sz max) change_z
	= max sz
:change_z
= scale (* -1 (- 1 (/ 1 max)))
+ scale (/ 0.5 max)
model_scale model scale scale scale

;model_rotate model 0 PI 0

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
