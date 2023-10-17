number x
= x 0

number i
= i 0
:next_loop
	number xx
	= xx 0

:loop
	number f
	= f x
	/ f 640
	* f 3.14159
	* f 2
	;sin f f
	* f 90
	+ f 180
	+ xx 2
	? xx 640
	jl loop

	number tmp
	= tmp x
	% tmp 640
	number f
	= f x
	/ f 640
	* f 3.14159
	* f 2
	;sin f f
	* f 90
	+ f 180
	+ x 5
+ i 1
? i 100000
jl next_loop
