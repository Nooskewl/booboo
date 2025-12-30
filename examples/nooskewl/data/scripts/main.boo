set_cull_mode NO_FACE

var model

= model (model_load "nooskewl.x")

set_3d

var rx rz count
= rx 0
= rz 0
= count 0

function draw
{
	clear 0 0 0

	model_draw model 255 255 255 255
}

function run
{
	= count (+ count 1)
	if (> count 480) zero
		= count 0
	:zero
	if (< count 240) rot z
		= rx (+ rx (/ PI 120))
		= rz (+ rz (/ PI 60))
	:rot
		= rx 0
		= rz 0
	:z

	identity_3d
	translate_3d 0 0 -1
	rotate_3d PI 0 0 1
	rotate_3d PI 0 1 0
	rotate_3d (/ PI 2) 1 0 0
	scale_3d 0.015 0.015 0.015
	rotate_3d rx 1 0 0
	rotate_3d rz 0 0 1
}
