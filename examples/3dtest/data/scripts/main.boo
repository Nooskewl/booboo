number model

model_load model "zeus.x"
model_translate model 0 -2 -10
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
