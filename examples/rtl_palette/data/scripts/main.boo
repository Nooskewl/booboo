number font
font_load font "vga.ttf" 16 0

resize 240 135

function draw
{
	font_draw font 255 255 255 255 "|00F|01i|02r|03e|01!|02!|03!" (- 240 25) 100 1
}
