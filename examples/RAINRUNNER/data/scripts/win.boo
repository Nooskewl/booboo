var clouds
var cloud
vector_add cloud 0
vector_add cloud 0
= [cloud 0] 360
= [cloud 1] 150
vector_add clouds cloud
= [cloud 0] 280
= [cloud 1] 150
vector_add clouds cloud
= [cloud 0] 335
= [cloud 1] 175
vector_add clouds cloud
= [cloud 0] 305
= [cloud 1] 175
vector_add clouds cloud

var count1
= count1 300
var count2
= count2 600

function draw_cloud cx cy
{
	var x
	var y
	= x cx
	= y cy
	= x (- x 20)
	= y (- y 20)
	filled_circle 224 224 224 255 x y 15 -1
	= x cx
	= y cy
	= x (- x 35)
	= y (- y 10)
	filled_circle 224 224 224 255 x y 20 -1
	= x cx
	= y cy
	= x (- x 15)
	= y (+ y 5)
	filled_circle 224 224 224 255 x y 15 -1
	= x cx
	= y cy
	= x (+ x 5)
	= y (+ y 15)
	filled_circle 224 224 224 255 x y 25 -1
	= x cx
	= y cy
	= x (+ x 15)
	= y (+ y 10)
	filled_circle 224 224 224 255 x y 15 -1
}

function draw_cloud_flip cx cy
{
	var x
	var y
	= x cx
	= y cy
	= x (+ x 20)
	= y (- y 20)
	filled_circle 224 224 224 255 x y 15 -1
	= x cx
	= y cy
	= x (+ x 35)
	= y (- y 10)
	filled_circle 224 224 224 255 x y 20 -1
	= x cx
	= y cy
	= x (+ x 15)
	= y (+ y 5)
	filled_circle 224 224 224 255 x y 15 -1
	= x cx
	= y cy
	= x (- x 5)
	= y (+ y 15)
	filled_circle 224 224 224 255 x y 25 -1
	= x cx
	= y cy
	= x (- x 15)
	= y (+ y 10)
	filled_circle 224 224 224 255 x y 15 -1
}

function draw
{
	var c
	var c1
	= c1 count1
	? c1 0
	jge no_fix_clear
	= c1 0
:no_fix_clear
	= c 300
	= c (- c c1)
	= c (/ c 300)
	var cr
	var cg
	var cb
	= cr 0
	= cg 216
	= cg (* cg c)
	= cb 255
	= cb  (* cb c)
	clear cr cg cb

	var sr
	var sg
	var sb
	= sr 128
	= sr (* sr c)
	= sr (+ sr 127)
	= sg 108
	= sg (* sg c)
	= sg (+ sg 108)
	= sb 0
	filled_circle sr sg sb 255 320 150 50 -1

	var num_clouds
	= num_clouds (vector_size clouds)

	var i
	= i 0
:next_cloud
	var cloud
	= cloud [clouds i]
	var x
	var y
	explode cloud x y
	? x 320
	jge flip_cloud
	call draw_cloud x y
	goto next_draw_cloud_iteration
:flip_cloud
	call draw_cloud_flip x y
:next_draw_cloud_iteration
	= i (+ i 1)
	? i num_clouds
	jl next_cloud
}

function run
{
	= count1 (- count1 1)
	? count1 0
	jle dont_update_clouds

	var num_clouds
	= num_clouds (vector_size clouds)

	var i
	= i 0
:next_cloud_update
	var cloud
	= cloud [clouds i]
	var x
	var y
	explode cloud x y
	? x 320
	jge flip_cloud_update
	=x (- x 0.5)
	= [cloud 0] x
	= [clouds i] cloud
	goto next_update_iteration
:flip_cloud_update
	= x (+ x 0.5)
	= [cloud 0] x
	= [clouds i] cloud
:next_update_iteration
	= i (+ i 1)
	? i num_clouds
	jl next_cloud_update
:dont_update_clouds

	= count2 (- count2 1)
	? count2 0
	jg not_done
	reset "enter_score.boo"
:not_done
}
