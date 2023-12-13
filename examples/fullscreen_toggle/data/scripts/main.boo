number font
font_load font "vga.ttf" 16 1

number old
= old 0

function draw
{
	clear 128 128 255

	font_draw font 255 255 255 255 "Press F!" 25 25
}

function run
{
	number f
	key_get f KEY_f

	if (&& (== f 1) (== old 0)) toggle
		toggle_fullscreen
		add_notification "Fullscreen toggled..."
:toggle

	= old f
}
