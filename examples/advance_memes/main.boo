resize 540 960

var font
= font (font_load "c:/windows/fonts/impact.ttf" 48 TRUE 512 TRUE)

var pics bananas i

for i 0 (< i 3) 1 loop
	var tmp
	= tmp (image_load (string_format "%.png" (+ i 1)))
	vector_add pics tmp
:loop

var pic next_pic
= pic (rand 0 2)
= next_pic (+ (get_ticks) 250)

= bananas (image_load "bananas.png")

var nanners nanner_y
= nanner_y 960

var next_nanner
= next_nanner (+ (get_ticks) 100)

function run
{
	if (< next_pic (get_ticks)) go1
		= next_pic (+ (get_ticks) 250)
		= pic (rand 0 2)
	:go1
	if (< next_nanner (get_ticks)) go2
		if (>= (vector_size nanners) 30) clear_it add_one
			vector_clear nanners
			= nanner_y 960
		:clear_it
			var nx ny na flags
			var sz
			= sz (vector_size nanners)
			if (== 0 (% sz 2)) left right
				= nx (rand -50 200)
				= flags FALSE
			:left
				= nx (rand 340 590)
				= flags TRUE
			:right
			= ny (+ nanner_y (rand -15 15))
			var v
			vector_init v nx ny 0 flags
			vector_add nanners v
			var i
			for i 0 (< i (vector_size nanners)) 1 next
				= [nanners i 2] (* (/ (- (rand 0 1000) 500) 500) (/ PI 6))
			:next
			= nanner_y (- nanner_y 20)
		:add_one
		= next_nanner (+ (get_ticks) 100)
	:go2
}

function draw
{
	image_draw [pics pic] 255 255 255 255 0 0

	var w h
	explode (image_size bananas) w h
	var i
	for i 0 (< i (vector_size nanners)) 1 loop
		image_draw_rotated_scaled bananas 255 255 255 255 (/ w 2) (/ h 2) [nanners i 0] [nanners i 1] [nanners i 2] 1 1 [nanners i 3]
	:loop
	
	call draw_text "10 times 10" 20
	call draw_text "is 100 in binary" 80
	call draw_text "Stonehenge confirmed!" 880
}

function draw_text s y
{
	var x
	= x (- (/ 540 2) (/ (font_width font s) 2))
	var xx yy
	for yy -8 (< yy 9) 1 next_y
		for xx -8 (< xx 9) 1 next_x
			font_draw font 0 0 0 255 s (+ x xx) (+ y yy)
		:next_x
	:next_y
	font_draw font 255 255 255 255 s x y
}
