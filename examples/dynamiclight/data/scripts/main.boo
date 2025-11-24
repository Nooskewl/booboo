var model
= model (model_load "zeus.x")
model_set_animation model "ArmatureAction"

var shader
= shader (shader_load "lit_3d_vertex" "lit_3d_fragment")

set_3d

var start_light_pos light_pos
vector_init start_light_pos 0 0 0 1
vector_init light_pos 0 0 0
var m
= m (identity 4)

function draw
{
	clear 100 100 255

	shader_use shader
	shader_set_float_vector shader "light_pos" light_pos

	model_draw model 255 255 255 255
}

function run
{
	identity_3d
	translate_3d 0 -0.5 -5
	scale_3d 0.25 0.25 0.25

	= m (mul m (rotate (/ PI 120) 0 1 0))
	= light_pos (mul m (translate 1000 0 0) start_light_pos)
	vector_erase light_pos 3
}
