number model

model_load model "zeus.x"
model_translate model 0 -2.5 -5

number font
font_load font "font.ttf" 48 0

function draw
{
	clear 100 100 255

	set_3d
	model_draw model 255 255 255 255

	set_2d
	font_draw font 255 255 0 255 "Testing..." 50 100
}
