number font
font_load font "vga.ttf" 16 1

number mx my
number b1 b2 b3 wheel
= mx 0
= my 0
= b1 0
= b2 0
= b3 0
= wheel 0

function draw
{
	string s

	string_format s "Mouse position: %,%" mx my
	font_draw font 64 64 64 255 s 32 32

	string_format s "Mouse buttons: % % % %" b1 b2 b3 wheel
	font_draw font 64 64 64 255 s 32 64
}

function run
{
	mouse_get_position mx my
	mouse_get_buttons b1 b2 b3 wheel
}
