number font
font_load font "vga.ttf" 16 0

number glyph1 glyph2
image_load glyph1 "misc/glyph1.png"
image_load glyph2 "misc/glyph2.png"

font_add_extra_glyph font #00 glyph1
font_add_extra_glyph font #01 glyph2

resize 240 135

function draw
{
	font_draw font 255 255 255 255 "|00F|01i|02r|03e|01!|02!|03!@00@00@01" (- 240 25) 100 1
}
