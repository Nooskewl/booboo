number exited
= exited FALSE

vector groups
vector_add groups 0
vector_add groups 1

number font
font_load font "vga.ttf" 12 1

map c c_both c_left c_right ll lr l1 l2 l3 l4 l5 r1 r2 r3 r4 r5 bcycle ; widget userdata
number wc wc_both wc_left wc_right wll wlr wl1 wl2 wl3 wl4 wl5 wr1 wr2 wr3 wr4 wr5 wbcycle ; widgets

function start_gui
{
	map_set c "draw" draw_window
	map_set c "event" null_event
	map_set c_both "draw" draw_nothing
	map_set c_both "event" null_event
	map_set c_left "draw" draw_nothing
	map_set c_left "event" null_event
	map_set c_right "draw" draw_nothing
	map_set c_right "event" null_event
	call_result ll mklabel "IN"
	call_result lr mklabel "OUT"
	pointer g1 g2
	address g1 [groups 0]
	address g2 [groups 1]
	call_result l1 mkradio g1 0 "Enlarge"
	call_result l2 mkradio g1 1 "Shrink"
	call_result l3 mkradio g1 2 "Appear"
	call_result l4 mkradio g1 3 "Slide"
	call_result l5 mkradio g1 4 "V. Slide"
	call_result r1 mkradio g2 0 "Enlarge"
	call_result r2 mkradio g2 1 "Shrink"
	call_result r3 mkradio g2 2 "Appear"
	call_result r4 mkradio g2 3 "Slide"
	call_result r5 mkradio g2 4 "V. Slide"
	call_result bcycle mkbutton "Cycle GUI" cycle_gui

	widget_create wll 1 30 ll
	widget_set_accepts_focus wll FALSE
	widget_create wl1 200 30 l1
	widget_create wl2 200 30 l2
	widget_create wl3 200 30 l3
	widget_create wl4 200 30 l4
	widget_create wl5 200 30 l5
	widget_create wc_left 200 100 c_left
	widget_set_accepts_focus wc_left FALSE
	widget_set_parent wll wc_left
	widget_set_parent wl1 wc_left
	widget_set_parent wl2 wc_left
	widget_set_parent wl3 wc_left
	widget_set_parent wl4 wc_left
	widget_set_parent wl5 wc_left

	widget_create wlr 1 30 lr
	widget_set_accepts_focus wlr FALSE
	widget_create wr1 200 30 r1
	widget_create wr2 200 30 r2
	widget_create wr3 200 30 r3
	widget_create wr4 200 30 r4
	widget_create wr5 200 30 r5
	widget_create wc_right 200 150 c_right
	widget_set_accepts_focus wc_right FALSE
	widget_set_parent wlr wc_right
	widget_set_parent wr1 wc_right
	widget_set_parent wr2 wc_right
	widget_set_parent wr3 wc_right
	widget_set_parent wr4 wc_right
	widget_set_parent wr5 wc_right

	widget_create wc_both 400 180 c_both
	widget_set_accepts_focus wc_both FALSE
	widget_set_parent wc_left wc_both
	widget_set_parent wc_right wc_both

	widget_create wc 400 210 c
	widget_set_accepts_focus wc FALSE
	widget_set_parent wc_both wc
	widget_create wbcycle 400 30 bcycle
	widget_set_break_line wbcycle TRUE
	widget_set_parent wbcycle wc

	gui_start wc
}

call start_gui

gui_set_focus wl1

function draw_nothing x y w h focussed data
{
}

function draw_window x y w h focussed data
{
	filled_rectangle 0 0 255 255 0 0 255 255 0 0 255 255 0 0 255 255 x y w h
}

function draw_button x y w h focussed data
{
	number r g b
	if (== focussed TRUE) yellow white
		= r 255
		= g 255
		= b 0
	:yellow
		= r 255
		= g 255
		= b 255
	:white
	number tw th
	font_width font tw [data "text"]
	font_height font th

	number xx yy
	= xx (+ x (/ w 2))
	- xx (/ tw 2)
	= yy (+ y (/ h 2))
	- yy (/ th 2)

	number ox oy
	if (== [data "down"] TRUE) offset no_offset
		= ox 5
		= oy 5
	:offset
		= ox 0
		= oy 0
	:no_offset

	+ x ox
	+ y oy
	+ xx ox
	+ yy oy

	filled_rectangle 0 0 255 255 0 0 255 255 0 255 255 255 0 255 255 255 x y w h
	rectangle r g b 255 x y w h 2
	font_draw font r g b 255 [data "text"] xx yy
}

function draw_label x y w h focussed data
{
	number tw th
	font_width font tw [data "text"]
	font_height font th

	number xx yy
	= xx (+ x (/ w 2))
	- xx (/ tw 2)
	= yy (+ y (/ h 2))
	- yy (/ th 2)

	filled_rectangle 0 0 255 255 0 0 255 255 0 255 255 255 0 255 255 255 x y w h
	font_draw font 255 255 255 255 [data "text"] xx yy
}

function draw_radio x y w h focussed data
{
	number tw th cy
	font_width font tw [data "text"]
	font_height font th

	number xx yy
	= xx (+ x 20)
	= yy (+ y (/ h 2))
	= cy yy
	- yy (/ th 2)

	filled_rectangle 0 0 255 255 0 0 255 255 0 0 255 255 0 0 255 255 x y w h
	font_draw font 255 255 255 255 [data "text"] (+ xx 20) yy

	circle 255 255 255 255 (+ xx 10) cy 7 1 -1

	if (== `[data "group"] [data "index"]) check
		filled_circle 255 255 255 255 (+ xx 10) cy 5 -1
	:check
	
	if (== focussed 1) draw_focus
		rectangle 255 255 0 255 x y w h 2
	:draw_focus
}

function null_event type a b c d x y w h focussed ~data
{
}

function button_event type a b c d x y w h focussed ~data
{
	if (&& (== type EVENT_MOUSE_DOWN) (== a 1) (== b FALSE)) down
		number on_button
		call_result on_button owned x y w h c d
		if (== on_button TRUE) really_down
			map_set data "down" TRUE
		:really_down
	:down

	if (&& (== [data "down"] TRUE) (== type EVENT_MOUSE_UP)) up
		number on_button
		call_result on_button owned x y w h c d
		if (== on_button TRUE) really_up
			call [data "callback"]
		:really_up
		map_set data "down" FALSE
	:up

	if (&& (== TRUE focussed) (== FALSE b) (|| (&& (== type EVENT_KEY_DOWN) (== KEY_RETURN a)) (&& (== type EVENT_JOY_DOWN) (== b JOY_A)))) down2
		map_set data "down" TRUE
	:down2
	if (&& (== [data "down"] TRUE) (== TRUE focussed) (|| (&& (== type EVENT_KEY_UP) (== KEY_RETURN a)) (&& (== type EVENT_JOY_UP) (== b JOY_A)))) play_it
		call [data "callback"]
		map_set data "down" FALSE
	:play_it
}

function radio_event type a b c d x y w h focussed ~data
{
	if (&& (== type EVENT_MOUSE_DOWN) (== a 1) (== b FALSE)) down
		number on_button
		call_result on_button owned x y w h c d
		if (== on_button TRUE) really_down
			= `[data "group"] [data "index"]
		:really_down
	:down
	if (&& (== TRUE focussed) (== FALSE b) (|| (&& (== type EVENT_KEY_DOWN) (== KEY_RETURN a)) (&& (== type EVENT_JOY_DOWN) (== b JOY_A)))) down2
		= `[data "group"] [data "index"]
	:down2
}

function mkbutton text callback
{
	map m
	map_set m "text" text
	map_set m "draw" draw_button
	map_set m "event" button_event
	map_set m "callback" callback
	map_set m "down" FALSE
	return m
}

function mklabel text
{
	map m
	map_set m "text" text
	map_set m "draw" draw_label
	map_set m "event" null_event
	return m
}

function mkradio group index text
{
	map m
	map_set m "text" text
	map_set m "draw" draw_radio
	map_set m "event" radio_event
	map_set m "group" group
	map_set m "index" index
	return m
}

function owned wx wy ww wh x y
{
	if (|| (< x wx) (< y wy) (>= x (+ wx ww)) (>= y (+ wy wh))) nope
		return FALSE
	:nope
	return TRUE
}

function gui_event id type a b c d x y w h focussed ~data
{
	call [data "event"] type a b c d x y w h focussed data
}

function gui_draw id x y w h focussed data
{
	call [data "draw"] x y w h focussed data
}

function cycle_gui
{
	gui_set_transition_types [groups 0] [groups 1]
	gui_exit
	= exited TRUE
}

function event type a b c d
{
	if (== exited FALSE) no
		return
	:no
	if (&& (== type EVENT_KEY_DOWN) (== a KEY_SPACE)) again
		= exited FALSE
		call start_gui
	:again
}

function draw
{
	if (== exited FALSE) back
		return
	:back

	font_draw font 255 255 255 255 "Press SPACE to restart GUI" 10 10
}
