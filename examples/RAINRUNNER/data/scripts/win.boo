vector clouds
vector cloud
vector_add cloud 0
vector_add cloud 0
vector_set cloud 0 360
vector_set cloud 1 150
vector_add clouds cloud
vector_set cloud 0 280
vector_set cloud 1 150
vector_add clouds cloud
vector_set cloud 0 335
vector_set cloud 1 175
vector_add clouds cloud
vector_set cloud 0 305
vector_set cloud 1 175
vector_add clouds cloud

number count1
= count1 300
number count2
= count2 600

function draw_cloud cx cy
{
	number x
	number y
	= x cx
	= y cy
	- x 20
	- y 20
	filled_circle 224 224 224 255 x y 15 -1
	= x cx
	= y cy
	- x 35
	- y 10
	filled_circle 224 224 224 255 x y 20 -1
	= x cx
	= y cy
	- x 15
	+ y 5
	filled_circle 224 224 224 255 x y 15 -1
	= x cx
	= y cy
	+ x 5
	+ y 15
	filled_circle 224 224 224 255 x y 25 -1
	= x cx
	= y cy
	+ x 15
	+ y 10
	filled_circle 224 224 224 255 x y 15 -1
}

function draw_cloud_flip cx cy
{
	number x
	number y
	= x cx
	= y cy
	+ x 20
	- y 20
	filled_circle 224 224 224 255 x y 15 -1
	= x cx
	= y cy
	+ x 35
	- y 10
	filled_circle 224 224 224 255 x y 20 -1
	= x cx
	= y cy
	+ x 15
	+ y 5
	filled_circle 224 224 224 255 x y 15 -1
	= x cx
	= y cy
	- x 5
	+ y 15
	filled_circle 224 224 224 255 x y 25 -1
	= x cx
	= y cy
	- x 15
	+ y 10
	filled_circle 224 224 224 255 x y 15 -1
}

function draw
{
	number c
	number c1
	= c1 count1
	? c1 0
	jge no_fix_clear
	= c1 0
:no_fix_clear
	= c 300
	- c c1
	/ c 300
	number cr
	number cg
	number cb
	= cr 0
	= cg 216
	* cg c
	= cb 255
	* cb c
	clear cr cg cb

	number sr
	number sg
	number sb
	= sr 128
	* sr c
	+ sr 127
	= sg 108
	* sg c
	+ sg 108
	= sb 0
	filled_circle sr sg sb 255 320 150 50 -1

	number num_clouds
	vector_size clouds num_clouds

	number i
	= i 0
:next_cloud
	vector cloud
	vector_get clouds cloud i
	number x
	number y
	vector_get cloud x 0
	vector_get cloud y 1
	? x 320
	jge flip_cloud
	call draw_cloud x y
	goto next_draw_cloud_iteration
:flip_cloud
	call draw_cloud_flip x y
:next_draw_cloud_iteration
	+ i 1
	? i num_clouds
	jl next_cloud
}

function run
{
	- count1 1
	? count1 0
	jle dont_update_clouds

	number num_clouds
	vector_size clouds num_clouds

	number i
	= i 0
:next_cloud_update
	vector cloud
	vector_get clouds cloud i
	number x
	number y
	vector_get cloud x 0
	vector_get cloud y 1
	? x 320
	jge flip_cloud_update
	- x 0.5
	vector_set cloud 0 x
	vector_set clouds i cloud
	goto next_update_iteration
:flip_cloud_update
	+ x 0.5
	vector_set cloud 0 x
	vector_set clouds i cloud
:next_update_iteration
	+ i 1
	? i num_clouds
	jl next_cloud_update
:dont_update_clouds

	- count2 1
	? count2 0
	jg not_done
	reset "enter_score.boo"
:not_done
}
