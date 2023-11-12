number grass
image_load grass "misc/grass.tga"
number robot
image_load robot "misc/robot.tga"
number img_w
number img_h
image_size grass img_w img_h

function draw
{
	number w
	= w (/ 640 img_w)
	number h
	= h (/ 360 img_h)

	number x
	number y
	number xx
	number yy

	image_start grass

	for y 0 h 1 loop_y
		for x 0 w 1 loop_x
			= xx x
			* xx img_w
			= yy y
			* yy img_h
			image_draw grass 255 255 255 255 xx yy 0 0
:loop_x
:loop_y

	image_end grass

	image_start robot

	for x 0 17 1 loop_robot
		rand xx 0 w
		* xx img_w
		rand yy 0 h
		* yy img_h
		image_draw robot 255 255 255 255 xx yy 0 0
:loop_robot

	image_end robot
}
