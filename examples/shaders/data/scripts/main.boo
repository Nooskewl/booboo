var shader
= shader (shader_load "" "glow")

var chilly
= chilly (image_load "chilly.png")

function draw
{
	var t
	= t (get_ticks)
	= t (% t 1000)
	if (< t 500) a b
		= t (/ t 500)
	:a
		= t (- 1 (/ (- t 500) 500))
	:b

	shader_use shader

	shader_set_float shader "t" t

	var w h
	explode (image_size chilly) w h
	= w (/ w 2)
	= h (/ h 2)

	image_draw chilly 255 255 255 255 (- (/ 640 2) w) (- (/ 360 2) h) 0 0

	shader_use_default
}
