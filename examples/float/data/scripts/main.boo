number r1 r2 x y
vector pts
number i
for i 0 (< i 3) 1 init
	rand x 50 589
	rand y 50 309
	rand r1 0 1
	if (== r1 0) neg_r1
		= r1 -1
:neg_r1
	rand r2 0 1
	if (== r2 0) neg_r2
		= r2 -1
:neg_r2
	vector v
	vector_add v x
	vector_add v y
	vector_add v r1
	vector_add v r2
	vector_add pts v
:init

function run
{
	number sz
	vector_size pts sz

	number i
	for i 0 (< i sz) 1 update
		vector v
		= v [pts i]
		+ [v 0] [v 2]
		+ [v 1] [v 3]
		if (< [v 0] 50) too_left (> [v 0] (- 640 50)) too_right
			neg [v 2]
:too_left
			neg [v 2]
:too_right
		if (< [v 1] 50) too_up (> [v 1] (- 360 50)) too_down
			neg [v 3]
:too_up
			neg [v 3]
:too_down
		= [pts i] v
:update
}

function draw
{
	triangle 255 0 216 255 [pts 0 0] [pts 0 1] [pts 1 0] [pts 1 1] [pts 2 0] [pts 2 1] 10
}
