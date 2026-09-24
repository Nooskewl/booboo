var img
= img (image_load "0.png")

var w h
explode (image_size img) w h

var bw bh
explode (get_buffer_size) bw bh

var p
= p (/ bw w)

var y
= y (/ (- bh (* h p)) 2)

function draw
{
	image_stretch_region img 255 255 255 255 0 0 w h 0 y bw (* h p)
}
