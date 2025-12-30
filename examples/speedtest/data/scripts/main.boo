const SCR_W 1280 SCR_H 720

resize SCR_W SCR_H

var img
= img (image_load "bingo.png")

var w h
explode (image_size img) w h

var font
= font (font_load "font.ttf" 64 1)

var imgs

call spawn

function spawn
{
	var v dx dy
	= dx (rand 0 1)
	= dy (rand 0 1)
	if (== dx 0) neg_dx
		= dx -1
	:neg_dx
	if (== dy 0) neg_dy
		= dy -1
	:neg_dy
	vector_init v (rand (/ w 2) (- SCR_W (/ w 2))) (rand (/ h 2) (- SCR_H (/ h 2))) dx dy
	vector_add imgs v
}

function draw
{
	var sz i
	= sz (vector_size imgs)

	image_start img

	for i 0 (< i sz) 1 next
		image_draw img 255 255 255 255 (- [imgs i 0] (/ w 2)) (- [imgs i 1] (/ h 2))
	:next

	image_end img

	font_draw font 255 255 0 255 (string_format "%" sz) 5 5
}

function run
{
	var sz i
	= sz (vector_size imgs)

	image_start img

	for i 0 (< i sz) 1 next
		= [imgs i 0] (+ [imgs i 0] [imgs i 2])
		= [imgs i 1] (+ [imgs i 1] [imgs i 3])
		if (< [imgs i 0] (/ w 2)) pos_x (< [imgs i 1] (/ h 2)) pos_y (> [imgs i 0] (- SCR_W (/ w 2))) neg_x (> [imgs i 1] (- SCR_H (/ h 2))) neg_y
			= [imgs i 0] (/ w 2)
			= [imgs i 2] (neg [imgs i 2])
		:pos_x
			= [imgs i 1] (/ h 2)
			= [imgs i 3] (neg [imgs i 3])
		:pos_y
			= [imgs i 0] (- SCR_W (/ w 2))
			= [imgs i 2] (neg [imgs i 2])
		:neg_x
			= [imgs i 1] (- SCR_H (/ h 2))
			= [imgs i 3] (neg [imgs i 3])
		:neg_y
	:next
}

function event type a b c d
{
	if (== type EVENT_KEY_DOWN) key
		if (== a KEY_UP) up (&& (== a KEY_DOWN) (> (vector_size imgs) 1)) down
			call spawn
		:up
			vector_erase imgs (- (vector_size imgs) 1)
		:down
	:key
}
