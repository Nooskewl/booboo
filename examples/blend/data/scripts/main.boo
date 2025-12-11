resize 1280 720

var font
= font (font_load "font.ttf" 16 1)

var src dest
= src BLEND_ONE
= dest BLEND_INVSRCALPHA

var sphere
= sphere (image_load "sphere.png")

var spheres v i
for i 0 (< i 10) 1 again
	vector_init v (rand 0 1280) (rand 0 720) (- (rand 0 2) 1) (- (rand 0 2) 1)
	vector_add spheres v
:again

var name_map	
vector_add name_map "BLEND_ZERO"
vector_add name_map "BLEND_ONE"
vector_add name_map "BLEND_SRCCOLOR"
vector_add name_map "BLEND_INVSRCCOLOR"
vector_add name_map "BLEND_SRCALPHA"
vector_add name_map "BLEND_INVSRCALPHA"

function tostr n
{
	return [name_map n]
}
function draw
{
	set_blend_mode src dest

	var i
	for i 0 (< i 10) 1 again
		image_draw sphere 255 255 255 255 (- [spheres i 0] (/ [(image_size sphere) 0] 2)) (- [spheres i 1] (/ [(image_size sphere) 1] 2))
	:again
	
	set_blend_mode BLEND_ONE BLEND_INVSRCALPHA
	var s
	= s (string_format "SRC: % DEST: %" (tostr src) (tostr dest))
	font_draw font 0 0 0 255 s 11 10
	font_draw font 0 0 0 255 s 10 11
	font_draw font 0 0 0 255 s 11 11
	font_draw font 255 255 0 255 s 10 10
}

function run
{
	var i
	for i 0 (< i 10) 1 again
		= [spheres i 0] (+ [spheres i 0] [spheres i 2])
		= [spheres i 1] (+ [spheres i 1] [spheres i 3])
		if (< [spheres i 0] 0) pos_x (< [spheres i 1] 0) pos_y (>= [spheres i 0] 1280) neg_x (>= [spheres i 1] 720) neg_y
			= [spheres i 0] 0
			= [spheres i 2] (neg [spheres i 2])
		:pos_x
			= [spheres i 1] 0
			= [spheres i 3] (neg [spheres i 3])
		:pos_y
			= [spheres i 0] 1279
			= [spheres i 2] (neg [spheres i 2])
		:neg_x
			= [spheres i 1] 719
			= [spheres i 3] (neg [spheres i 3])
		:neg_y
	:again
}

function event type a b c d
{
	if (== type EVENT_KEY_DOWN) check_key
		if (== a KEY_UP) up (== a KEY_DOWN) down (== a KEY_LEFT) left (== a KEY_RIGHT) right
			= src (- src 1)
			if (< src 0) wrap1
				= src 5
			:wrap1
		:up
			= src (+ src 1)
			= src (% src 6)
		:down
			= dest (- dest 1)
			if (< dest 0) wrap3
				= dest 5
			:wrap3
		:left
			= dest (+ dest 1)
			= dest (% dest 6)
		:right
	:check_key
}
