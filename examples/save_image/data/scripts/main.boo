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

number font
font_load font "font.ttf" 16 1

number s
= s 0

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
	clear 0 0 0
	triangle 255 0 216 255 [pts 0 0] [pts 0 1] [pts 1 0] [pts 1 1] [pts 2 0] [pts 2 1] 5

	font_draw font 255 255 255 255 "Press S to save screenshot" 15 15

	number k
	key_get k KEY_S
	if (&& (!= k 0) (== s 0)) save_it (== k 0) clear_it
		= s 1
		string home
		getenv home "USERPROFILE"
		if (== home "") get_home
			getenv home "HOME"
		:get_home
		string filename
		string_format filename "%/save_image_screenshot.png" home
		screenshot filename
		string n
		string_format n "% saved!" filename
		add_notification n
	:save_it
		= s 0
	:clear_it
}
