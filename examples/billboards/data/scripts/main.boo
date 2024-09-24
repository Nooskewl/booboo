number tree
image_load tree "tree.png"

vector billboards

number num_billboards
= num_billboards 10

number i
for i 0 (< i num_billboards) 1 loop
	number b
	number x
	rand x 0 1000
	- x 500
	/ x 500
	* x 2
	number z
	rand z 0 1000
	- z 500
	/ z 500
	* z 10
	billboard_create b tree x 0 z 0.4 0.8
	vector_add billboards b
:loop
	
set_3d

translate_3d 0 0 -5

function draw
{
	clear 100 100 255

	number i
	for i 0 (< i num_billboards) 1 loop
		billboard_draw [billboards i] 255 255 255 255
	:loop
}

function eabs x
{
	if (< x 0) switch
		neg x
	:switch
	return x
}

function run
{
	include "poll_joystick.inc"

	neg joy_y1

	if (< (eabs joy_x1) 0.1) zero_x
		= joy_x1 0
	:zero_x
	if (< (eabs joy_y1) 0.1) zero_y
		= joy_y1 0
	:zero_y
	if (< (eabs joy_x2) 0.1) zero_z
		= joy_x2 0
	:zero_z

	/ joy_x1 25
	/ joy_y1 25
	/ joy_x2 25

	translate_3d joy_x1 joy_y1 joy_x2
}
