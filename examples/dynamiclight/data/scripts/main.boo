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

	= m (mul m (rotate (/ PI 60) 1 0 0))
	= m (mul m (rotate (/ PI 120) 0 1 0))
	vector t
	= t (translate 100 0 0)
	= t (mul m t)
	print "% % % %\n" [t 0 0] [t 0 1] [t 0 2] [t 0 3]
	print "% % % %\n" [t 1 0] [t 1 1] [t 1 2] [t 1 3]
	print "% % % %\n" [t 2 0] [t 2 1] [t 2 2] [t 2 3]
	print "% % % %\n" [t 3 0] [t 3 1] [t 3 2] [t 3 3]
	print "\n"
	= light_pos (mul t start_light_pos)
	vector_erase light_pos 3

	print "% % %\n" [light_pos 0] [light_pos 1] [light_pos 2]
}
