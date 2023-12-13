number font
font_load font "font.ttf" 20 1

number num
= num 360
/ num 20
floor num
- num 2

number old_u old_d old_a old_b
= old_u 0
= old_d 0
= old_a 0
= old_b 0

number old_b1 old_b3
= old_b1 0
= old_b3 0

number selected
number top
vector filenames
string dir

call list_dir "."

function list_dir name
{
	= dir name
	+ name "/*"
	list_directory filenames name
	vector_insert filenames 0 "../"
	= selected 0
	= top 0
}

function chop_dir s
{
	number p
	string_length s p
	- p 2
	if (< p 0) none
		return s
:none
	number i
	for i p (>= i 0) -1 loop
		number c
		string_char_at s c i
		string cs
		string_from_number cs c
		if (|| (== cs "/") (== cs "\\\\")) found
			+ i 1
			string_substr s i
			return s
:found
:loop

	return s
}

function draw
{
	number sz
	vector_size filenames sz
	number i
	number y
	= y 0

	filled_rectangle 255 0 216 255 255 0 216 255 255 0 216 255 255 0 216 255 0 (+ (* num 20) 5) 640 (- 360 (+ (* num 20) 5))
	string str
	= str "[A/Z/LMB] Navigate       [B/X/RMB] Launch Here"
	number w
	font_width font w str
	/ w 2
	font_draw font 255 216 0 255 str (- 320 w) (+ (* num 20) 10)

	for i top (&& (< i sz) (< i (+ top num))) 1 loop
		string s
		= s [filenames i]
		call_result s chop_dir s
		number c
		= c 128
		if (== i selected) draw_bg
			= c 255
			filled_rectangle 0 216 255 255 0 216 255 255 0 216 255 255 0 216 255 255 0 y 640 20
:draw_bg
		font_draw font c c c 255 s 5 y
		+ y 20
:loop
}
function sel_up
{
	- selected 1
	if (< selected 0) zero
		= selected 0
:zero
	if (< selected top) dec_top
		= top selected
:dec_top
}

function sel_down
{
	number sz
	vector_size filenames sz
	+ selected 1
	if (>= selected sz) fix
		= selected (- sz 1)
:fix
	if (<= (+ top num) selected) adjust
		= top (- selected (- num 1))
:adjust
}

function navigate
{
	string s
	= s [filenames selected]
	call_result s chop_dir s
	number len
	string_length s len
	number c
	string_char_at s c (- len 1)
	string cs
	string_from_number cs c
	if (|| (== cs "/") (== cs "\\\\")) go_dir
		string d
		= d dir
		+ d "/" s
		call list_dir d
:go_dir
}

function launch
{
	number cfg
	cfg_load cfg "com.b1stable.launcher.reload"
	cfg_set_string cfg "launch" dir
	number success
	cfg_save cfg success "com.b1stable.launcher.reload"
	exit 0
}

function run
{
	number b1 b2 b3 wheel
	mouse_get_buttons b1 b2 b3 wheel

	if (== wheel 1) wheel_up (== wheel -1) wheel_down
		call sel_up
:wheel_up
		call sel_down
:wheel_down

	if (&& (== b1 1) (== old_b1 0)) mouse_b1 (&& (== b3 1) (== old_b3 0)) mouse_b3
		call navigate
:mouse_b1
		call launch
:mouse_b3

	= old_b1 b1
	= old_b3 b3

	number sz
	vector_size filenames sz

	number joy_x1
	number joy_y1
	number joy_x2
	number joy_y2
	number joy_x3
	number joy_y3
	number joy_l
	number joy_r
	number joy_u
	number joy_d
	number joy_a
	number joy_b
	number joy_x
	number joy_y
	number joy_lb
	number joy_rb
	number joy_ls
	number joy_rs
	number joy_back
	number joy_start

	joystick_poll 0 joy_x1 joy_y1 joy_x2 joy_y2 joy_x3 joy_y3 joy_l joy_r joy_u joy_d joy_a joy_b joy_x joy_y joy_lb joy_rb joy_ls joy_rs joy_back joy_start

	; This provides some keyboard support mapped to joystick

	number _key_l _key_r _key_u _key_d _key_a _key_b _key_back
	key_get _key_l KEY_LEFT
	key_get _key_r KEY_RIGHT
	key_get _key_u KEY_UP
	key_get _key_d KEY_DOWN
	key_get _key_a KEY_z
	key_get _key_b KEY_x
	key_get _key_back KEY_ESCAPE

	= joy_l (|| (== _key_l 1) (== joy_l 1))
	= joy_r (|| (== _key_r 1) (== joy_r 1))
	= joy_u (|| (== _key_u 1) (== joy_u 1))
	= joy_d (|| (== _key_d 1) (== joy_d 1))
	= joy_a (|| (== _key_a 1) (== joy_a 1))
	= joy_b (|| (== _key_b 1) (== joy_b 1))
	= joy_back (|| (== _key_back 1) (== joy_back 1))

	if (&& (== joy_u 1) (== old_u 0)) do_up (&& (== joy_d 1) (== old_d 0)) do_down
		call sel_up
:do_up
		call sel_down
:do_down

	if (&& (== joy_a 1) (== old_a 0)) dig
		call navigate
:dig

	if (&& (== joy_b 1) (== old_b 0)) go
		call launch
:go

	= old_u joy_u
	= old_d joy_d
	= old_a joy_a
	= old_b joy_b
}
