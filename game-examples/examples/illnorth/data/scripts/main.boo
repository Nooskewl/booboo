number outer
number inner
image_load outer "misc/outer.tga"
image_load inner "misc/inner.tga"

number angle
= angle 0.0

function run
{
	+ angle 0.0418
}

function draw
{
	clear 0 0 0
	image_draw_rotated_scaled inner 255 255 255 255 128 128 320 160 0.0 1.0 1.0 0 0
	image_draw_rotated_scaled outer 255 255 255 255 129 148 320 180 angle 1.0 1.0 0 0
}
