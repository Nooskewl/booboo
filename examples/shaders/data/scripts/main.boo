var shader
= shader (shader_load "" "red_fragment")

var tree
= tree (image_load "tree.png")

function draw
{
	shader_use shader

	var w h
	explode (image_size tree) w h
	= w (/ w 2)
	= h (/ h 2)

	image_draw tree 255 255 255 255 (- (/ 640 2) w) (- (/ 360 2) h) 0 0

	shader_use_default
}
