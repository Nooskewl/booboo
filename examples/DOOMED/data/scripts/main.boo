var font
= font (font_load "font.ttf" 96 1)
var small_font
= small_font (font_load "font.ttf" 48 1)

var drip_sfx
= drip_sfx (mml_create "@PO0 = { 0 1000 0 1000 0 }\nA o3 @TYPE3 @PO0 g32 @PO0")

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
	var text
	= text "DOOMED"
	var tw
	var th
	= tw (font_width font text)
	= th (font_height font)
	= tw (/ tw 2)
	var dx
	var dy
	= dx (- 320 tw)
	= dy 50
	font_draw font 255 255 255 255 text dx dy

	var w
	var h
	= w 360
	= h 36
	var dx
	var dy
	= dx 320
	= dx (- dx (/ w 2))
	= dy (+ 50 th 20)
	? selected 0
	je draw_bar
	= dy (+ dy h)
:draw_bar
	filled_rectangle 255 255 255 255 255 255 255 255 255 255 255 255 255 255 255 255 dx dy w h

	var text
	= text "PLAY"
	var tw2
	var th2
	= tw2 (font_width small_font text)
	= th2 (font_height small_font)
	= dx 320
	= tw2 (/ tw2 2)
	= th2 (/ th2 2)
	= dx (- dx tw2)
	= dy 50
	= dy (+ dy th)
	= dy (+ dy 20)
	var half
	= half (/ h 2)
	= dy (+ dy half)
	= dy (- dy th2)
	var tr tg tb
	if (== selected 0) black1 white1
		= tr 0
		= tg 0
		= tb 0
	:black1
		= tr 255
		= tg 255
		= tb 255
	:white1
	font_draw small_font tr tg tb 255 text dx dy
	= text "HIGH SCORES"
	= tw2 (font_width small_font text)
	= tw2 (/ tw2 2)
	= dx 320
	= dx (- dx tw2)
	= dy (+ dy h)
	if (== selected 1) black2 white2
		= tr 0
		= tg 0
		= tb 0
	:black2
		= tr 255
		= tg 255
		= tb 255
	:white2
	font_draw small_font tr tg tb 255 text dx dy
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
	mml_play drip_sfx 1 0
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
