string buf last lastop
= buf "0"
= last ""
= lastop "="
number peq
= peq 1

number button_sfx
mml_create button_sfx "@PO0 = { 0 100 }\nA @PO0 g @PO0"

number font
font_load font "vga.ttf" 48 1

resize 1280 720

map mc mc1 mc2 m9 m8 m7 m6 m5 m4 m3 m2 m1 m0 mdec mneg madd msub mmul mdiv meq
map_set mc "draw" draw_window
map_set mc "event" null_event
map_set mc1 "draw" draw_window
map_set mc1 "event" null_event
map_set mc2 "draw" draw_window
map_set mc2 "event" null_event
call_result m9 mkbutton "9"
call_result m8 mkbutton "8"
call_result m7 mkbutton "7"
call_result m6 mkbutton "6"
call_result m5 mkbutton "5"
call_result m4 mkbutton "4"
call_result m3 mkbutton "3"
call_result m2 mkbutton "2"
call_result m1 mkbutton "1"
call_result m0 mkbutton "0"
call_result mdec mkbutton "."
call_result mneg mkbutton "+-"
call_result madd mkbutton "+"
call_result msub mkbutton "-"
call_result mmul mkbutton "*"
call_result mdiv mkbutton "/"
call_result meq mkbutton "="

number container c1 c2 _9 _8 _7 _6 _5 _4 _3 _2 _1 _0 _dec _neg _add _sub _mul _div _eq
widget_create container 300 500 mc
widget_set_accepts_focus container FALSE
widget_set_padding_top container 50
widget_create c1 180 500 mc1
widget_set_accepts_focus c1 FALSE
widget_set_parent c1 container
widget_create c2 120 500 mc2
widget_set_accepts_focus c2 FALSE
widget_set_parent c2 container

widget_create _7 60 0.25 m7
widget_set_parent _7 c1
widget_create _8 60 0.25 m8
widget_set_parent _8 c1
widget_create _9 60 0.25 m9
widget_set_parent _9 c1
widget_create _4 60 0.25 m4
widget_set_parent _4 c1
widget_create _5 60 0.25 m5
widget_set_parent _5 c1
widget_create _6 60 0.25 m6
widget_set_parent _6 c1
widget_create _1 60 0.25 m1
widget_set_parent _1 c1
widget_create _2 60 0.25 m2
widget_set_parent _2 c1
widget_create _3 60 0.25 m3
widget_set_parent _3 c1
widget_create _0 60 0.25 m0
widget_set_parent _0 c1
widget_create _dec 60 0.25 mdec
widget_set_parent _dec c1
widget_create _neg 60 0.25 mneg
widget_set_parent _neg c1

widget_create _mul 0.5 0.25 mmul
widget_set_parent _mul c2
widget_create _div 0.5 0.25 mdiv
widget_set_parent _div c2
widget_create _add 0.5 0.25 madd
widget_set_parent _add c2
widget_create _sub 0.5 0.25 msub
widget_set_parent _sub c2
widget_create _eq 1.0 0.5 meq
widget_set_parent _eq c2

map ops
map_set ops "9" i_num
map_set ops "8" i_num
map_set ops "7" i_num
map_set ops "6" i_num
map_set ops "5" i_num
map_set ops "4" i_num
map_set ops "3" i_num
map_set ops "2" i_num
map_set ops "1" i_num
map_set ops "0" i_num
map_set ops "." i_num
map_set ops "+" i_op
map_set ops "-" i_op
map_set ops "*" i_op
map_set ops "/" i_op
map_set ops "+-" i_neg
map_set ops "=" i_eq

gui_start container

gui_set_focus _eq

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
		= ox 2
		= oy 2
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

function button_event type a b c d x y w h focussed ~data
{
	number pressed
	= pressed 0

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
			= pressed 1
		:really_up
		map_set data "down" FALSE
	:up

	if (&& (== TRUE focussed) (== FALSE b) (|| (&& (== type EVENT_KEY_DOWN) (== KEY_RETURN a)) (&& (== type EVENT_JOY_DOWN) (== a JOY_A)))) down2
		map_set data "down" TRUE
	:down2
	if (&& (== [data "down"] TRUE) (== TRUE focussed) (|| (&& (== type EVENT_KEY_UP) (== KEY_RETURN a)) (&& (== type EVENT_JOY_UP) (== a JOY_A)))) play_it
		= pressed 1
		map_set data "down" FALSE
	:play_it

	if (== pressed 1) press
		call [ops [data "text"]] data
	:press
}

function mkbutton text
{
	map m
	map_set m "text" text
	map_set m "draw" draw_button
	map_set m "event" button_event
	map_set m "down" FALSE
	return m
}

function owned wx wy ww wh x y
{
	if (|| (< x wx) (< y wy) (>= x (+ wx ww)) (>= y (+ wy wh))) nope
		return FALSE
	:nope
	return TRUE
}

function null_event type a b c d x y w h focussed ~data
{
}

function gui_event id type a b c d x y w h focussed ~data
{
	call [data "event"] type a b c d x y w h focussed data
}

function gui_draw id x y w h focussed data
{
	call [data "draw"] x y w h focussed data
}

function draw
{
	number w
	font_width font w buf
	font_draw font 255 255 255 255 buf (- 790 w) 50
}

function i_num data
{
	if (== peq 1) set
		= buf [data "text"]
		= peq 0
		return
	:set

	number found
	string_matches found buf "[\\.]"

	if (&& (== found TRUE) (== [data "text"] ".")) nodup
		return
	:nodup

	+ buf [data "text"]
}

function i_op data
{
	= peq 0
	call i_eq data
	= lastop [data "text"]
	= last buf
	= peq 1
}

function i_neg data
{
	number found
	string_matches found buf "^-"

	if (== found TRUE) pos negative
		string_substr buf 1
	:pos
		string_format buf "-%" buf
	:negative
}

function i_eq data
{
	if (== peq 1) clear_it calc
		= buf "0"
		= last ""
		= lastop "="
	:clear_it
		number a b
		= a buf
		= b last
		if (== lastop "+") __add (== lastop "-") __sub (== lastop "*") __mul (== lastop "/") __div
			+ a b
		:__add
			- b a
			= a b
		:__sub
			* a b
		:__mul
			/ b a
			= a b
		:__div
		string_format buf "%" a
		= last ""
		= lastop "="
		= peq 1
	:calc
}
