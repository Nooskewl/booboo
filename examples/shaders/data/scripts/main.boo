number shader
shader_load shader "red_fragment" 2

number tree
image_load tree "tree.png"

function draw
{
	shader_use shader

	number w h
	image_size tree w h
	/ w 2
	/ h 2

	image_draw tree 255 255 255 255 (- (/ 640 2) w) (- (/ 360 2) h) 0 0

	shader_use_default
}
