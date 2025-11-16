resize 320 180
vector rings ring
number img i

number nrings
= nrings 5

for i 0 (< i nrings) 1 next_ring
	image_load img "misc/ring.png"
	vector_add ring img
	number r
	rand r 40 280
	vector_add ring r
	rand r 40 140
	vector_add ring r
	vector_add rings ring
	vector_clear ring
:next_ring

function draw
{
	number i

	for i 0 (< i nrings) 1 loop
		vector ring
		= ring [rings i]
		image_draw [ring 0] 255 255 255 255 (- [ring 1] 24) (- [ring 2] 24)
	:loop
}

function event type a b c d
{
	if (== type EVENT_MOUSE_DOWN) click
		number mx my i tlx tly
		= mx c
		= my d
		for i 0 (< i nrings) 1 next
			vector ring
			= ring [rings i]
			= tlx (- [ring 1] 24)
			= tly (- [ring 2] 24)
			if (&& (>= mx tlx) (>= my tly) (< mx (+ tlx 48)) (< my (+ tly 48))) hit
				vector pix
				image_read_texture [ring 0] pix
				number x y
				for y -5 (<= y 5) 1 next_y
					for x -5 (<= x 5) 1 next_x
						number l
						= l (+ (* x x) (* y y))
						sqrt l
						if (<= l 5) go
							number xx yy
							= xx (- (+ mx x) tlx)
							= yy (- (+ my y) tly)
							if (! (|| (< xx 0) (< yy 0) (>= xx 48) (>= yy 48))) check_it
								= yy (- 48 yy)
								if (== 255 [pix yy xx 0]) swap_it
									= [pix yy xx 0] #24
									= [pix yy xx 1] #24
									= [pix yy xx 2] #24
								:swap_it
							:check_it
						:go
					:next_x
				:next_y
				number img
				image_to_texture img pix
				image_destroy [ring 0]
				= [ring 0] img
				= [rings i] ring
			:hit
		:next
	:click
}
