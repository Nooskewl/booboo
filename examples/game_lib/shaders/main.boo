number shader
shader_load shader "glsl/default_vertex.txt" 2 "glsl/darkness_fragment.txt" 2
shader_use shader 1
shader_set_int shader "screen_w" 640
shader_set_int shader "screen_h" 360
shader_set_float shader "mush_x" 320
shader_set_float shader "mush_y" 180
shader_set_float shader "maxx" 320
shader_set_colour shader "colour1" 0 0 0 255
shader_set_colour shader "colour2" 255 255 255 255
shader_use shader 0

number ticks
= ticks 0

function draw
{
	number t
	= t ticks
	% t 1000
	/ t 500
	if (>= t 1) pingpong
	- t 1
	= t (- 1 t)
:pingpong

	shader_use shader 1
	shader_set_float shader "t" t
	filled_rectangle 0 0 0 255 0 0 0 255 0 0 0 255 0 0 0 255 0 0 640 360
	shader_use shader 0
}

function run
{
	+ ticks 1
}
