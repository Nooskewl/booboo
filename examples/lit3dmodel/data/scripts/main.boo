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

vector light_pos
vector_init light_pos 0 1000000 0

function draw
{
	clear 100 100 255

	shader_use shader
	shader_set_float_vector shader "light_pos" light_pos

	model_draw model 255 255 255 255
}

function run
{
	+ rx (/ PI 60)
	+ ry (/ PI 120)
	identity_3d
	translate_3d 0 0 -5
	rotate_3d rx 1 0 0
	rotate_3d ry 0 1 0
	scale_3d 0.25 0.25 0.25
}
