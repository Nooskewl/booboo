number font
font_load font "font.ttf" 96 1
number small_font
font_load small_font "font.ttf" 48 1

number drip_sfx
mml_create drip_sfx "@PO0 = { 0 1000 0 1000 0 }\nA o3 @TYPE3 @PO0 g32 @PO0"

number selected
= selected 0

include "poll_joystick.inc"
number old_joy_a
= old_joy_a joy_a
number old_joy_u
= old_joy_u joy_u
number old_joy_d
= old_joy_d joy_d

function draw
{
	string text
	= text "DOOMED"
	number tw
	number th
	font_width font tw text
	font_height font th
	= tw (/ tw 2)
	number dx
	number dy
	= dx (- 320 tw)
	= dy 50
	font_draw font 255 255 255 255 text dx dy

	number w
	number h
	= w 360
	= h 36
	number dx
	number dy
	= dx 320
	- dx (/ w 2)
	= dy 50
	+ dy th
	+ dy 20
	? selected 0
	je draw_bar
	+ dy h
:draw_bar
	filled_rectangle 255 255 255 255 255 255 255 255 255 255 255 255 255 255 255 255 dx dy w h

	string text
	= text "PLAY"
	number tw2
	number th2
	font_width small_font tw2 text
	font_height small_font th2
	= dx 320
	/ tw2 2
	/ th2 2
	- dx tw2
	= dy 50
	+ dy th
	+ dy 20
	number half
	= half h
	/ half 2
	+ dy half
	- dy th2
	number tr tg tb
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
	font_width small_font tw2 text
	/ tw2 2
	= dx 320
	- dx tw2
	+ dy h
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
