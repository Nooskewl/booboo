var ph pw sp am amm buf i
= buf 0

for i 0 (< i 5) 1 wave
	vector_add ph (/ (rand 0 1000) 1000)
	vector_add pw (rand 3 15)
	vector_add sp (+ (* (/ (rand 0 1000) 1000) 0.01) 0.01)
	vector_add am (+ (* (/ (rand 0 1000) 1000) 25) 5)
	vector_add amm (rand 150 300)
	= buf (+ buf 30)
:wave

function draw
{
	clear 0 0 0

	var i
	for i 0 (< i TRAILSZ) 1 tr
		var j
		= j (% (+ i tbi) TRAILSZ)
		var a
		= a (* (/ (+ i 1) TRAILSZ) 255)
		image_draw [trailbufs j] a a a a 0 0
	:tr

	line 0 255 0 255 0 ground 640 ground 2

	; draw life

	var lr
	var lg
	var lb
	var l
	= l life
	? l 0
	jge life_ok
	= l 0
:life_ok
	= lr [life_r l]
	= lg [life_g l]
	= lb [life_b l]

	var box_w
	var box_h
	= box_h (+ 16 6)
	= box_w (- (+ (* (+ 7 2) max_life) 6) 2)

	rectangle lr lg lb 255 10 319 box_w box_h 1

	? life 0
	jle no_life_to_draw

	var i
	= i 0
:draw_next_life
	var dx
	= dx (+ 10 3)
	var bar_w
	= bar_w (* 9 i)
	= dx (+ dx bar_w)
	var dy
	= dy (+ 319 3)
	filled_rectangle lr lg lb 255 lr lg lb 255 lr lg lb 255 lr lg lb 255 dx dy 7 16
	= i (+ i 1)
	? i life
	jl draw_next_life
:no_life_to_draw

	; draw score
	var text
	var score_int
	= score_int (number (floor score))
	= text (string_format "%" score_int)
	var text_w
	= text_w (font_width font text)
	var text_x
	= text_x (- 640 15 text_w)
	var font_h
	= font_h (font_height font)
	font_draw font 0 255 0 255 text text_x (- 330 (/ font_h 2))

	? life 0
	jg dont_draw_flood

	var sw sh
	explode (get_buffer_size) sw sh

	start_primitives

	var i
	for i 0 (< i sw) 1 flood
		var j
		var amp amp2
		= amp 0
		= amp2 0
		for j 0 (< j (vector_size ph)) 1 flood2
			var f f2
			= f (sin (+ (* [ph j] PI 2) (* (/ i sw) [pw j])))
			= f2 (sin (+ (* [ph j] PI 2) (* (/ (+ i 320 ) sw) [pw j])))
			var t
			= t (% (get_ticks) (* [amm j] 2))
			if (>= t [amm j]) greater less
				= t (- 1 (/ (- t [amm j]) [amm j]))
			:greater
				= t (/ t [amm j])
			:less
			= f (* f (* [am j] t))
			= f2 (* f2 (* [am j] t))
			= amp (+ amp f)
			= amp2 (+ amp2 f2)
		:flood2
		var h
		= h (+ sh (* buf 2))
		var y
		= y (/ exit_count 180)
		= y (* y h)
		= y (+ y amp)
		= y (- y buf)
		filled_rectangle 0 68 80 80 0 68 80 80 0 68 80 80 0 68 80 80 i y 1 (- sh y)
		var h
		= h (+ sh (* buf 2))
		var y
		= y (/ exit_count 180)
		= y (* y h)
		= y (+ y amp2)
		= y (- y 15)
		= y (- y buf)
		filled_rectangle 0 68 80 80 0 68 80 80 0 68 80 80 0 68 80 80 i y 1 (- sh y)
	:flood

	end_primitives

:dont_draw_flood
}

function event type a b c d
{
	if (== type EVENT_TICK) go
		var i
		for i 0 (< i (vector_size ph)) 1 loop
			= [ph i] (+ [ph i] [sp i])
		:loop
	:go
}
