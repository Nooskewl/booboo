vector pts
vector v
vector_add v 100
vector_add v 50
vector_add v 1
vector_add v -1
vector_add pts v
vector_clear v
vector_add v 480
vector_add v 200
vector_add v 1
vector_add v 1
vector_add pts v
vector_clear v
vector_add v 300
vector_add v 300
vector_add v -1
vector_add v -1
vector_add pts v

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
	triangle 255 0 216 255 [pts 0 0] [pts 0 1] [pts 1 0] [pts 1 1] [pts 2 0] [pts 2 1] 5
}
