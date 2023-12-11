number img
image_load img "misc/window.png"

number font
font_load font "vga.ttf" 16 0

resize 240 135

function draw
{
	image_draw_9patch img 255 255 255 255 10 90 220 35

	font_draw font 255 255 255 255 "Whoa! Don't click!" 25 100
}

function run
{
	number b1 b2 b3
	mouse_get_buttons b1 b2 b3

	if (== b1 1) shake
		screen_shake 2 500
		rumble 100
	:shake
}
