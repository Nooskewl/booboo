var exited
= exited FALSE

var groups
vector_add groups TRANSITION_ENLARGE
vector_add groups TRANSITION_SHRINK

var font
= font (font_load "vga.ttf" 12 1)

var c c_both c_left c_right ll lr l1 l2 l3 l4 l5 l6 r1 r2 r3 r4 r5 r6 bcycle ; widget userdata
number wc wc_both wc_left wc_right wll wlr wl1 wl2 wl3 wl4 wl5 wl6 wr1 wr2 wr3 wr4 wr5 wr6 wbcycle ; widgets

function start_gui
{
	= [c "draw"] draw_window
	= [c "event"] null_event
	= [c_both "draw"] draw_nothing
	= [c_both "event"] null_event
	= [c_left "draw"] draw_nothing
	= [c_left "event"] null_event
	= [c_right "draw"] draw_nothing
	= [c_right "event"] null_event
	call_result ll mklabel "IN"
	call_result lr mklabel "OUT"
	var g1 g2
	= g1 (@ [groups 0])
	= g2 (@ [groups 1])
	call_result l1 mkradio g1 TRANSITION_NONE "None"
	call_result l2 mkradio g1 TRANSITION_ENLARGE "Enlarge"
	call_result l3 mkradio g1 TRANSITION_SHRINK "Shrink"
	call_result l4 mkradio g1 TRANSITION_SLIDE "Slide"
	call_result l5 mkradio g1 TRANSITION_SLIDE_VERTICAL "V. Slide"
	call_result l6 mkradio g1 TRANSITION_FADE "Fade"
	call_result r1 mkradio g2 TRANSITION_NONE "None"
	call_result r2 mkradio g2 TRANSITION_ENLARGE "Enlarge"
	call_result r3 mkradio g2 TRANSITION_SHRINK "Shrink"
	call_result r4 mkradio g2 TRANSITION_SLIDE "Slide"
	call_result r5 mkradio g2 TRANSITION_SLIDE_VERTICAL "V. Slide"
	call_result r6 mkradio g2 TRANSITION_FADE "Fade"
	call_result bcycle mkbutton "Cycle GUI" cycle_gui

	= wll (widget_create 1 30 ll)
	widget_set_accepts_focus wll FALSE
	= wl1 (widget_create 200 30 l1)
	= wl2 (widget_create 200 30 l2)
	= wl3 (widget_create 200 30 l3)
	= wl4 (widget_create 200 30 l4)
	= wl5 (widget_create 200 30 l5)
	= wl6 (widget_create 200 30 l6)
	= wc_left (widget_create 200 210 c_left)
	widget_set_accepts_focus wc_left FALSE
	widget_set_parent wll wc_left
	widget_set_parent wl1 wc_left
	widget_set_parent wl2 wc_left
	widget_set_parent wl3 wc_left
	widget_set_parent wl4 wc_left
	widget_set_parent wl5 wc_left
	widget_set_parent wl6 wc_left

	= wlr (widget_create 1 30 lr)
	widget_set_accepts_focus wlr FALSE
	= wr1 (widget_create 200 30 r1)
	= wr2 (widget_create 200 30 r2)
	= wr3 (widget_create 200 30 r3)
	= wr4 (widget_create 200 30 r4)
	= wr5 (widget_create 200 30 r5)
	= wr6 (widget_create 200 30 r6)
	= wc_right (widget_create 200 210 c_right)
	widget_set_accepts_focus wc_right FALSE
	widget_set_parent wlr wc_right
	widget_set_parent wr1 wc_right
	widget_set_parent wr2 wc_right
	widget_set_parent wr3 wc_right
	widget_set_parent wr4 wc_right
	widget_set_parent wr5 wc_right
	widget_set_parent wr6 wc_right

	= wc_both (widget_create 400 210 c_both)
	widget_set_accepts_focus wc_both FALSE
	widget_set_parent wc_left wc_both
	widget_set_parent wc_right wc_both

	= wc (widget_create 400 240 c)
	widget_set_accepts_focus wc FALSE
	widget_set_parent wc_both wc
	= wbcycle (widget_create 400 30 bcycle)
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
	var r g b
	if (== focussed TRUE) yellow white
		= r 255
		= g 255
		= b 0
	:yellow
		= r 255
		= g 255
		= b 255
	:white
	var tw th
	= tw (font_width font [data "text"])
	= th (font_height font)

	var xx yy
	= xx (- (+ x (/ w 2)) (/ tw 2))
	= yy (- (+ y (/ h 2)) (/ th 2))

	var ox oy
	if (== [data "down"] TRUE) offset no_offset
		= ox 5
		= oy 5
	:offset
		= ox 0
		= oy 0
	:no_offset

	= x (+ x ox)
	= y (+ y oy)
	= xx (+ xx ox)
	= yy (+ yy oy)

	filled_rectangle 0 0 255 255 0 0 255 255 0 255 255 255 0 255 255 255 x y w h
	rectangle r g b 255 x y w h 2
	font_draw font r g b 255 [data "text"] xx yy
}

function draw_label x y w h focussed data
{
	var tw th
	= tw (font_width font [data "text"])
	= th (font_height font)

	var xx yy
	= xx (- (+ x (/ w 2)) (/ tw 2))
	= yy (- (+ y (/ h 2)) (/ th 2))

	filled_rectangle 0 0 255 255 0 0 255 255 0 255 255 255 0 255 255 255 x y w h
	font_draw font 255 255 255 255 [data "text"] xx yy
}

function draw_radio x y w h focussed data
{
	var tw th cy
	= tw (font_width font [data "text"])
	= th (font_height font)

	var xx yy
	= xx (+ x 20)
	= yy (+ y (/ h 2))
	= cy yy
	= yy (- yy (/ th 2))

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
		var on_button
		call_result on_button owned x y w h c d
		if (== on_button TRUE) really_down
			= [data "down"] TRUE
		:really_down
	:down

	if (&& (== [data "down"] TRUE) (== type EVENT_MOUSE_UP)) up
		var on_button
		call_result on_button owned x y w h c d
		if (== on_button TRUE) really_up
			call [data "callback"]
		:really_up
		= [data "down"] FALSE
	:up

	if (&& (== TRUE focussed) (== FALSE b) (|| (&& (== type EVENT_KEY_DOWN) (== KEY_RETURN a)) (&& (== type EVENT_JOY_DOWN) (== b JOY_A)))) down2
		= [data "down"] TRUE
	:down2
	if (&& (== [data "down"] TRUE) (== TRUE focussed) (|| (&& (== type EVENT_KEY_UP) (== KEY_RETURN a)) (&& (== type EVENT_JOY_UP) (== b JOY_A)))) play_it
		call [data "callback"]
		= [data "down"] FALSE
	:play_it
}

function radio_event type a b c d x y w h focussed ~data
{
	if (&& (== type EVENT_MOUSE_DOWN) (== a 1) (== b FALSE)) down
		var on_button
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
	var m
	= [m "text"] text
	= [m "draw"] draw_button
	= [m "event"] button_event
	= [m "callback"] callback
	= [m "down"] FALSE
	return m
}

function mklabel text
{
	var m
	= [m "text"] text
	= [m "draw"] draw_label
	= [m "event"] null_event
	return m
}

function mkradio group index text
{
	var m
	= [m "text"] text
	= [m "draw"] draw_radio
	= [m "event"] radio_event
	= [m "group"] group
	= [m "index"] index
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
