number img
image_load img "b1stable.tga"

number r g b
= r 0
= g #d8
= b #ff

function run
{
	number t
	get_ticks t

	number total
	= total (+ 1500 40 20 20 40)

	number tmp
	= tmp t
	/ tmp total
	floor tmp
	= t (- t (* tmp total))

	if (< t 1500) c (< t 1540) y (< t 1560) m (< t 1600) y2 m2
		call cyan
	:c
		call yellow
	:y
		call magenta
	:m
		call yellow
	:y2
		call magenta
	:m2
}

function cyan
{
	= r 0
	= g #d8
	= b #ff
}

function magenta
{
	= r #ff
	= g 0
	= b #d8
}

function yellow
{
	= r #ff
	= g #d8
	= b 0
}

function draw
{
	number sz
	= sz 192

	number x y
	= x (/ (- 640 sz) 2)
	= y (/ (- 360 sz) 2)

	image_stretch_region img r g b 255 0 0 16 16 x y sz sz 0 0
}
