number font
font_load font "vga.ttf" 32 0

number sample
sample_load sample "blip.wav"

number a b playing
= a 0
= b 0
= playing 0

function draw
{
	font_draw font 64 64 64 255 "Z: Play once" 10 10
	font_draw font 64 64 64 255 "X: Start/stop loop" 10 40
}

function run
{
	include "poll_joystick.inc"

	if (&& (== joy_a 1) (== a 0)) play (&& (== joy_b 1) (== b 0)) loop
		sample_play sample 1 0
		= playing 0
:play
		if (== playing 1) stop start
			sample_stop sample
			= playing 0
:stop
			sample_play sample 1 1
			= playing 1
:start
:loop

	= a joy_a
	= b joy_b
}
