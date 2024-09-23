string reset_game_name
= reset_game_name "blip.boo"
include "slideshow_start.inc"

number ticks
= ticks 0

number cfg
cfg_load cfg "com.illnorth.clock"
cfg_get_number cfg ticks "ticks"

function run
{
	+ ticks 1
	include "slideshow_logic.inc"
}

function draw
{
	clear 255 255 255

	number ninety
	= ninety 3.14159
	/ ninety 2

	number angle
	number angle2
	number angle3

	= angle ticks
	/ angle 60
	/ angle 60
	= angle2 angle
	* angle 3.14159
	* angle 2
	- angle ninety

	/ angle2 60
	= angle3 angle2
	* angle2 3.14159
	* angle2 2
	- angle2 ninety

	/ angle3 12
	* angle3 2
	* angle3 3.14159
	- angle3 ninety

	number x
	number y

	= x angle2
	cos x
	* x 85
	+ x 320
	= y angle2
	sin y
	* y 85
	+ y 180

	line 0 0 0 255 x y 320 180 5 ; minute hand
	
	number a1
	number a2
	number x1
	number y1
	number x2
	number y2
	number x3
	number y3

	= x1 angle2
	cos x1
	* x1 5
	+ x1 x
	= y1 angle2
	sin y1
	* y1 5
	+ y1 y
	= a1 angle2
	+ a1 ninety
	= a2 angle2
	- a2 ninety
	= x2 a1
	cos x2
	= y2 a1
	sin y2
	* x2 2.5
	* y2 2.5
	+ x2 x
	+ y2 y
	= x3 a2
	cos x3
	= y3 a2
	sin y3
	* x3 2.5
	* y3 2.5
	+ x3 x
	+ y3 y

	filled_triangle 0 0 0 255 0 0 0 255 0 0 0 255 x1 y1 x2 y2 x3 y3 ; minute hand point

	number a2
	= a2 angle2
	+ a2 3.14159
	= x a2
	cos x
	* x 10
	+ x 320
	= y a2
	sin y
	* y 10
	+ y 180

	line 0 0 0 255 x y 320 180 5 ; extend minute hand back

	= x angle3
	cos x
	* x 60
	+ x 320
	= y angle3
	sin y
	* y 60
	+ y 180

	line 0 0 0 255 x y 320 180 10 ; hour hand

	= x1 angle3	
	cos x1
	* x1 10
	+ x1 x
	= y1 angle3
	sin y1
	* y1 10
	+ y1 y
	= a1 angle3
	+ a1 ninety
	= a2 angle3
	- a2 ninety
	= x2 a1
	cos x2
	= y2 a1
	sin y2
	* x2 5
	* y2 5
	+ x2 x
	+ y2 y
	= x3 a2
	cos x3
	= y3 a2
	sin y3
	* x3 5
	* y3 5
	+ x3 x
	+ y3 y

	filled_triangle 0 0 0 255 0 0 0 255 0 0 0 255 x1 y1 x2 y2 x3 y3 ; hour hand point
	
	= a2 angle3
	+ a2 3.14159
	= x a2
	cos x
	* x 10
	+ x 320
	= y a2
	sin y
	* y 10
	+ y 180

	line 0 0 0 255 x y 320 180 10 ; extend hour hand back

	= x angle
	cos x
	* x 100
	+ x 320
	= y angle
	sin y
	* y 100
	+ y 180

	line 255 0 0 255 x y 320 180 2 ; second hand

	number a2
	= a2 angle
	+ a2 3.14159
	= x a2
	cos x
	* x 15
	+ x 320
	= y a2
	sin y
	* y 15
	+ y 180

	line 255 0 0 255 x y 320 180 2 ; extend second hand back
}

function end
{
	cfg_set_number cfg "ticks" ticks
	number save_result
	cfg_save cfg save_result "com.illnorth.clock"
}
