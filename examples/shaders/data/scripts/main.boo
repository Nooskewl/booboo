number shader
shader_load shader "glsl/default_vertex.txt" 2 "glsl/darkness_fragment.txt" 2
shader_use shader 1
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

	number mx my
	get_screen_size mx my
	/ mx 2
	/ my 2

	number bw bh
	get_buffer_size bw bh
	/ bw 2

	shader_use shader
	shader_set_float shader "t" t
	shader_set_float shader "mush_x" mx
	shader_set_float shader "mush_y" my
	shader_set_float shader "maxx" bw
	filled_rectangle 0 0 0 255 0 0 0 255 0 0 0 255 0 0 0 255 0 0 640 360
	shader_use_default
}

function run
{
	+ ticks 1
}
