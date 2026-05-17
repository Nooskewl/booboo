text_clear

var i
for i 0 (< i 100) 1 loop
	var f
	= f (* (/ i 100) PI 2)
	var x y
	= x (+ (* (cos f) 10) 40)
	= y (+ (* (sin f) 10) 12)
	text_set_cursor_pos x y
	print "#"
:loop

text_set_cursor_pos 0 24
