number tex
image_load tex "brick.png"

number model
model_load model "zeus.x"
model_set_animation model "ArmatureAction"

number shader
shader_load shader "lit_3d_vertex" "lit_3d_fragment"

number rx ry
= rx 0
= ry 0

set_3d

vector start_light_pos light_pos
vector_init start_light_pos 0 0 0 1
= light_pos start_light_pos
vector m
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
	vector t
	= t (translate 100 0 0)
	= t (mul m t)
	= light_pos (mul t start_light_pos)
	vector_erase light_pos 3
}
