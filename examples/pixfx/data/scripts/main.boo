resize 320 180
var rings ring
var img i

var nrings
= nrings 5

for i 0 (< i nrings) 1 next_ring
	= img (image_load "misc/ring.png")
	var pix
	= pix (image_read_texture img)
	image_destroy img
	= img (image_to_texture pix)
	vector_add ring img
	var r
	= r (rand 40 280)
	vector_add ring r
	= r (rand 40 140)
	vector_add ring r
	vector_add rings ring
	vector_clear ring
:next_ring

function draw
{
	var i

	for i 0 (< i nrings) 1 loop
		var ring
		= ring [rings i]
		image_draw [ring 0] 255 255 255 255 (- [ring 1] 24) (- [ring 2] 24)
	:loop
}

function event type a b c d
{
	if (== type EVENT_MOUSE_DOWN) click
		var mx my i tlx tly
		= mx c
		= my d
		for i 0 (< i nrings) 1 next
			var ring
			= ring [rings i]
			= tlx (- [ring 1] 24)
			= tly (- [ring 2] 24)
			if (&& (>= mx tlx) (>= my tly) (< mx (+ tlx 48)) (< my (+ tly 48))) hit
				var pix
				= pix (image_read_texture [ring 0])
				var x y
				for y -5 (<= y 5) 1 next_y
					for x -5 (<= x 5) 1 next_x
						var l
						= l (sqrt (+ (* x x) (* y y)))
						if (<= l 5) go
							var xx yy
							= xx (- (+ mx x) tlx)
							= yy (- (+ my y) tly)
							if (! (|| (< xx 0) (< yy 0) (>= xx 48) (>= yy 48))) check_it
								if (== 255 [pix yy xx 0]) swap_it
									= [pix yy xx 0] #24
									= [pix yy xx 1] #24
									= [pix yy xx 2] #24
								:swap_it
							:check_it
						:go
					:next_x
				:next_y
				image_update [ring 0] pix
			:hit
		:next
	:click
}
