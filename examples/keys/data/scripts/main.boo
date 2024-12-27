number font
font_load font "vga.ttf" 16 1

number sfx
mml_create sfx "@PO0 = { 0 100 }\n@PO1 = { 0 200 }\n@PO2 = { 0 300 }\nA l16 @PO0a@PO0 @PO1a@PO1 @PO2a@PO2"

number pressed
= pressed 0

function draw
{
	font_draw font 64 64 64 255 "Press F!" 32 32
}

function run
{
	number f
	key_get f KEY_f

	if (&& (== 1 f) (== pressed 0)) on (&& (== 0 f) (== pressed 1)) off
	= pressed 1
	mml_play sfx 1 0
:on
	= pressed 0
:off
}
