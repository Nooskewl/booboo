number x
= x 0

number i
= i 0
for i 100000 1 next_loop
call draw
call run
:next_loop

function draw_at x
{
	number f
	= f x
	/ f 640
	* f 2 3.14159
	;sin f f
	* f 90
	+ f 180
}

function draw
{
	number xx
	= xx 0

	for xx 640 2 loop
	call draw_at xx
:loop

	number tmp
	= tmp x
	% tmp 640

	call draw_at tmp
}

function run
{
	+ x 5
}
