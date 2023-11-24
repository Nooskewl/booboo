number font
font_load font "vga.ttf" 14 0

function draw
{
	number mx my
	number b1 b2 b3
	mouse_get_position mx my
	mouse_get_buttons b1 b2 b3

	string s

	string_format s "Mouse position: %,%" mx my
	font_draw font 64 64 64 255 s 32 32

	string_format s "Mouse buttons: % % %" b1 b2 b3
	font_draw font 64 64 64 255 s 32 64
}
