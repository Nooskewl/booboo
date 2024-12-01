resize 320 180

number sprite
sprite_load sprite "pleasant"

vector names
vector files
file_list files
number i nfiles
vector_size files nfiles
for i 0 (< i nfiles) 1 check_next
	number matches
	string_matches matches [files i] "pleasant\\/.*json$"
	if (== TRUE matches) add_it
		string f
		= f [files i]
		string_replace f ".*\\/(.*).json" "$1"
		if (!= "sprite" f) really_add
			vector_add names f
		:really_add
	:add_it
:check_next

call callback sprite

function callback id
{
	number sz
	vector_size names sz
	number r
	rand r 0 (- sz 1)
	sprite_set_animation id [names r] callback
}

function draw
{
	number w h
	sprite_current_frame_size sprite w h

	number dx dy
	= dx (- 160 (/ w 2))
	= dy (- 90 (/ h 2))

	sprite_draw sprite 255 255 255 255 dx dy 0 0
}
