number x
= x 0

number i
= i 0
:next_loop
call draw
call run
+ i 1
? i 100000
jl next_loop

function draw_at x
{
	number f
	= f x
	/ f 640
	* f 3.14159
	* f 2
	;sin f f
	* f 90
	+ f 180
}

function draw
{
	number xx
	= xx 0

:loop
	call draw_at xx
	+ xx 2
	? xx 640
	jl loop

	number tmp
	= tmp x
	% tmp 640

	call draw_at tmp
}

function run
{
	+ x 5
}
