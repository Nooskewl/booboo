var bloom
= bloom (shader_load "" "bloom")
const BLOOMS 2

var SCR_W SCR_H
explode (get_buffer_size) SCR_W SCR_H

var bloombufs buf orig_buf bbidx
= buf (image_create SCR_W SCR_H)
vector_add bloombufs buf
= buf (image_create SCR_W SCR_H)
vector_add bloombufs buf
= orig_buf (image_create SCR_W SCR_H)
= bbidx 0

function draw_bloom img loops
{
	if (< loops 1) ret
		return
	:ret
	set_target [bloombufs bbidx]
	clear 0 0 0 0
	shader_use bloom
	shader_set_int bloom "tex_w" SCR_W
	shader_set_int bloom "tex_h" SCR_H
	shader_set_bool bloom "horizontal" TRUE
	image_draw img 255 255 255 255 0 0
	var old
	= old bbidx
	= bbidx (+ bbidx 1)
	if (> bbidx 1) set03
		= bbidx 0
	:set03
	set_target [bloombufs bbidx]
	clear 0 0 0 0
	shader_set_bool bloom "horizontal" FALSE
	image_draw [bloombufs old] 255 255 255 255 0 0

	var i
	for i 0 (< i (- loops 1)) 1 loop
		shader_set_bool bloom "horizontal" TRUE
		var old
		= old bbidx
		= bbidx (+ bbidx 1)
		if (> bbidx 1) set0
			= bbidx 0
		:set0
		set_target [bloombufs bbidx]
		clear 0 0 0 0
		image_draw [bloombufs old] 255 255 255 255 0 0
		
		shader_set_bool bloom "horizontal" FALSE
		= old bbidx
		= bbidx (+ bbidx 1)
		if (> bbidx 1) set02
			= bbidx 0
		:set02
		set_target [bloombufs bbidx]
		clear 0 0 0 0
		image_draw [bloombufs old] 255 255 255 255 0 0
	:loop

	shader_use_default
}
