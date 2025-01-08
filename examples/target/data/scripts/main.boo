number img
image_create img 20 20

call draw_img

function draw_img
{
	set_target img
	clear 0 0 0
	circle 255 0 216 255 10 10 9.5 1.2 -1
	filled_circle 255 0 216 255 6 6 1.2 -1
	filled_circle 255 0 216 255 14 6 1.2 -1
	line 255 0 216 255 2 6 10 19 1.2
	line 255 0 216 255 18 6 10 19 1.2
	set_target_backbuffer
}

function draw
{
	image_stretch_region img 255 255 255 255 0 0 20 20 0 0 640 360 0 0
}

; This is only required for D3D, otherwise image will be blank on a lost/found event
function found_device
{
	if (== 1 D3D9) redraw
		call draw_img
	:redraw
}
