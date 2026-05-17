var json
= json (json_load "colours.json")

var font
= font (font_load "NotoSansArabic.ttf" 32 1)

function draw
{
	var r1 g1 b1 r2 g2 b2 r3 g3 b3 r4 g4 b4
	= r1 (json_get_number json "topleft>r")
	= g1 (json_get_number json "topleft>g")
	= b1 (json_get_number json "topleft>b")
	= r2 (json_get_number json "topright>r")
	= g2 (json_get_number json "topright>g")
	= b2 (json_get_number json "topright>b")
	= r3 (json_get_number json "bottomright>r")
	= g3 (json_get_number json "bottomright>g")
	= b3 (json_get_number json "bottomright>b")
	= r4 (json_get_number json "bottomleft>[0]")
	= g4 (json_get_number json "bottomleft>[1]")
	= b4 (json_get_number json "bottomleft>[2]")

	filled_rectangle r1 g1 b1 255 r2 g2 b2 255 r3 g3 b3 255 r4 g4 b4 255 0 0 640 360

	var text
	= text (json_get_string json "message")
	font_draw font 255 255 255 255 text (- 640 10) (- 360 10) FALSE TRUE

	font_draw font 255 255 255 255 "Hello, world!" 10 10
}
