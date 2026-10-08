var clouds
var cloud
vector_add cloud 0 0 0
= [cloud 0] 300
= [cloud 1] 150
= [cloud 2] 20
vector_add clouds cloud
= [cloud 0] 220
= [cloud 1] 150
= [cloud 2] 45
vector_add clouds cloud
= [cloud 0] 275
= [cloud 1] 175
= [cloud 2] 30
vector_add clouds cloud
= [cloud 0] 245
= [cloud 1] 175
= [cloud 2] 20
vector_add clouds cloud
= [cloud 0] 260
= [cloud 1] 1600
= [cloud 2] 35
vector_add clouds cloud
= [cloud 0] 240
= [cloud 1] 150
= [cloud 2] 15
vector_add clouds cloud
= [cloud 0] 215
= [cloud 1] 180
= [cloud 2] 20
vector_add clouds cloud
= [cloud 0] 270
= [cloud 1] 175
= [cloud 2] 30
vector_add clouds cloud
= [cloud 0] 180
= [cloud 1] 145
= [cloud 2] 20
vector_add clouds cloud
= [cloud 0] 185
= [cloud 1] 185
= [cloud 2] 15
vector_add clouds cloud

var count1
= count1 300
var count2
= count2 600

var next_flicker flicker_len glow_sz
= next_flicker 5
= flicker_len next_flicker
= glow_sz 0

function draw_cloud cx cy csz
{
	filled_circle 224 224 224 255 cx cy csz
}

function draw_cloud_flip cx cy csz
{
	= cx (- 640 cx)
	filled_circle 224 224 224 255 cx cy csz
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
	var x y sz
	explode cloud x y sz
	call draw_cloud x y sz
	call draw_cloud_flip x y sz
:next_draw_cloud_iteration
	= i (+ i 1)
	? i num_clouds
	jl next_cloud

	var alpha
	= alpha (/ next_flicker flicker_len)
	var y
	= y (- 360 glow_sz)
	var r g b
	= r (* 255 alpha)
	= g (* 216 alpha)
	= b 0
	= alpha (* alpha 255)
	filled_rectangle 0 0 0 0 0 0 0 0 r g b alpha r g b alpha 0 y 640 150
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
	= x (- x 0.35)
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

	= next_flicker (- next_flicker 1)
	if (<= next_flicker 0) flick
		= next_flicker (rand 5 25)
		= flicker_len next_flicker
		var f
		= f (- 1 (/ count2 600))
		= glow_sz (+ (* (/ (rand 0 1000) 1000) 100 f) (* f 50))
	:flick
}
