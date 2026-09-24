include "bloom.inc"

var music
= music (mml_load "music/menu.mml")
= VOID (mml_play music 0.5 1)

var logo
= logo (image_load "misc/coinhunt.png")

var small_font
= small_font (font_load "c:/windows/fonts/arial.ttf" 32 1 512 TRUE)

var sfx
= sfx (mml_create "@PO0 = { 0 -100 }\nA @PO0 c32 @PO0")

var selected
= selected 0

include "poll_joystick.inc"
var old_joy_a
= old_joy_a joy_a
var old_joy_u
= old_joy_u joy_u
var old_joy_d
= old_joy_d joy_d

function draw
{
	set_target orig_buf
	clear 0 0 0 0	

	var logo_w logo_h
	explode (image_size logo) logo_w logo_h
	image_draw logo 255 255 255 255 (- 320 (/ logo_w 2)) 50

	var w
	var h
	= w 256
	= h 32
	var dx
	var dy
	= dx (- 320 128)
	= dy (+ 50 logo_h 20)
	? selected 0
	je draw_bar
	= dy (+ dy h)
:draw_bar
	filled_rectangle #00 #d8 #ff 255 #00 #d8 #ff 255 #00 #d8 #ff 255 #00 #d8 #ff 255 dx dy w h

	var r1 g1 b1 r2 g2 b2

	if (== selected 0) a b
		= r1 0
		= g1 0
		= b1 0
		= r2 #00
		= g2 #d8
		= b2 #ff
	:a
		= r1 #00
		= g1 #d8
		= b1 #ff
		= r2 0
		= g2 0
		= b2 0
	:b

	var text
	= text "PLAY"
	var tw2
	var th2
	= tw2 (/ (font_width small_font text) 2)
	= th2 (/ (font_height small_font) 2)
	= dx (- 320 tw2)
	= dy (+ 50 logo_h 20)
	var half
	= half (/ h 2)
	= dy (- (+ dy half) th2)
	font_draw small_font r1 g1 b1 255 text dx dy
	= text "HIGH SCORES"
	= tw2 (font_width small_font text)
	= tw2 (/ tw2 2)
	= dx (- 320 tw2)
	= dy (+ dy h)
	font_draw small_font r2 g2 b2 255 text dx dy
	
	call draw_bloom orig_buf BLOOMS

	set_target_backbuffer

	image_draw [bloombufs bbidx] 255 255 255 255 0 0
	image_draw orig_buf 255 255 255 255 0 0
}

function run
{
	include "poll_joystick.inc"

	? joy_u old_joy_u
	je test_joy_d
	= old_joy_u joy_u
	? joy_u 1
	je move_bar
:test_joy_d
	? joy_d old_joy_d
	je no_move_bar
	= old_joy_d joy_d
	? joy_d 1
	je move_bar
	goto no_move_bar
:move_bar
	= VOID (mml_play sfx 1 0)
	? selected 0
	je make_1
	= selected 0
	goto no_move_bar
:make_1
	= selected 1
:no_move_bar

	? joy_a old_joy_a
	je no_go
	= old_joy_a joy_a
	? joy_a 1
	jne no_go
	? selected 0
	je start_game
	reset "highscores.boo"
	goto no_go
:start_game
	reset "game.boo"
:no_go
}
