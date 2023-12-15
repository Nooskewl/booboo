number f
font_load f "vga.ttf" 32 0

number fw
number fh

font_height f fh
font_width f fw "0"

function draw
{
	number cols
	number rows

	= cols (+ 1 (/ 640 fw))
	= rows (+ 1 (/ 360 fh))

	number x
	number y

	for y 0 (< y rows) 1 next_y
		for x 0 (< x cols) 1 next_x
			number dx
			number dy
			= dx (* x fw)
			= dy (* y fh)
			number r
			string c
			rand r 0 1
			if (== r 0) c_zero c_one
			= c "0"
:c_zero
			= c "1"
:c_one
			font_draw f 32 32 32 255 c dx dy
:next_x
:next_y
}
