number sfx1 sfx2 sfx3 sfx4
mml_load sfx1 "sfx/b1.mml"
mml_load sfx2 "sfx/b2.mml"
mml_load sfx3 "sfx/b3.mml"
mml_load sfx4 "sfx/b4.mml"

map c b1 b2 b3 b4
map_set c "type" "window"
map_set b1 "sfx" sfx1
map_set b2 "sfx" sfx2
map_set b3 "sfx" sfx3
map_set b4 "sfx" sfx4
map_set b1 "text" "Elixir"
map_set b2 "text" "Holy Water"
map_set b3 "text" "Potion"
map_set b4 "text" "Potion Omega"
map_set b1 "type" "button"
map_set b2 "type" "button"
map_set b3 "type" "button"
map_set b4 "type" "button"

number font
font_load font "vga.ttf" 12 1

number container w1 w2 w3 w4

widget_create container 200 200 c
widget_create w1 0.5 0.5 b1
widget_set_parent w1 container
widget_create w2 0.5 0.5 b2
widget_set_parent w2 container
widget_create w3 0.5 0.5 b3
widget_set_break_line w3 TRUE
widget_set_parent w3 container
widget_create w4 0.5 0.5 b4
widget_set_parent w4 container

gui_start container

function owned wx wy ww wh x y
{
	if (|| (< x wx) (< y wy) (>= x (+ wx ww)) (>= y (+ wy wh))) nope
		return FALSE
	:nope
	return TRUE
}

function gui_event id type a b c d x y w h focussed data
{
	if (&& (== type EVENT_MOUSE_DOWN) (== [data "type"] "button") (== a 1)) play
		number on_button
		call_result on_button owned x y w h c d
		if (== on_button TRUE) really_play
			mml_play [data "sfx"] 1 0
		:really_play
	:play
}

function gui_draw id x y w h focussed data
{
	if (== [data "type"] "button") draw_button
		number tw th
		font_width font tw [data "text"]
		font_height font th

		number xx yy
		= xx (+ x (/ w 2))
		- xx (/ tw 2)
		= yy (+ y (/ h 2))
		- yy (/ th 2)

		filled_rectangle 0 0 255 255 0 0 255 255 0 255 255 255 0 255 255 255 x y w h
		rectangle 255 255 255 255 x y w h 1
		font_draw font 255 255 255 255 [data "text"] xx yy
	:draw_button
}
