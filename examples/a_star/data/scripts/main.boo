number tilemap
tilemap_load tilemap "map.wm2"

number w h layers
tilemap_size tilemap w h
tilemap_num_layers tilemap layers

number dx dy
= dx 0
= dy 0

number font
font_load font "vga.ttf" 16 0

number is_clicked
= is_clicked 0

number num_clicks
= num_clicks 0

vector clicks
vector c
vector_add c 0
vector_add c 0
vector_add clicks c
vector_add clicks c

vector path

function draw
{
	tilemap_draw tilemap 0 (- layers 1) dx dy

	font_draw font 0 0 0 255 "Click twice!" 16 (- 360 40)

	number sz
	vector_size path sz
	number i
	for i 0 (< i (- sz 1)) 1 draw_path
		number x1 y1 x2 y2
		= x1 [path i 0]
		= y1 [path i 1]
		= x2 [path (+ i 1) 0]
		= y2 [path (+ i 1) 1]
		+ x1 0.5
		+ y1 0.5
		+ x2 0.5
		+ y2 0.5
		* x1 16 ; tile size
		* y1 16 ; tile size
		* x2 16 ; tile size
		* y2 16 ; tile size
		line 0 255 0 255 x1 y1 x2 y2 3
	:draw_path
}

function run
{
	number b1 b2 b3
	mouse_get_buttons b1 b2 b3

	if (&& (== b1 1) (== is_clicked 0)) click (&& (== b1 0) (== is_clicked 1)) unclick
	= is_clicked 1
	number mx my
	mouse_get_position mx my
	number m
	= m (% num_clicks 2)
	= [clicks m 0] (/ mx 16) ; tile size
	= [clicks m 1] (/ my 16)
	+ num_clicks 1
	= m (% num_clicks 2)
	vector empty ; can be used to add extra solids like for entities
	if (== m 0) get_path
		tilemap_find_path tilemap path empty [clicks 0 0] [clicks 0 1] [clicks 1 0] [clicks 1 1]
	:get_path
:click
	= is_clicked 0
:unclick
}
