number font
font_load font "font.ttf" 20 1

number go_ok
= go_ok 0

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

function list_dir name
{
	= dir name
	+ name "/*"
	list_directory filenames name
	vector_insert filenames 0 "../"
	= selected 0
	= top 0
	= go_ok 0
	number sz
	vector_size filenames sz
	number i
	for i 0 (< i sz) 1 loop_good
		string s
		= s [filenames i]
		call_result s chop_dir s
		if (== s "data.cpa") is_cpa
			= go_ok 1
			goto done_list
:is_cpa
		if (== s "data/") is_data_dir
			string orig
			= orig [filenames i]
			+ orig "scripts/main.boo"
			number f
			file_open f orig "r"
			if (!= -1 f) is_booboo_app
				file_close f
				= go_ok 1
				goto done_list
:is_booboo_app
:is_data_dir
:loop_good
:done_list
	for i 0 (< i sz) 1 loop_bad
		string s
		= s [filenames i]
		call_result s chop_dir s
		if (|| (== s "BooBoo.exe") (== s "BooBoo")) dont_relaunch
			= go_ok 0
			goto really_done_list
:dont_relaunch
:loop_bad
:really_done_list
}

function draw
{
	number sz
	vector_size filenames sz
	number i
	number y
	= y 0

	number bg_r bg_g bg_b fg_r fg_g fg_b

	if (== go_ok 1) highlight default
		= bg_r 255
		= bg_g 0
		= bg_b 216
		= fg_r 255
		= fg_g 216
		= fg_b 0
:highlight
		= bg_r 64
		= bg_g 64
		= bg_b 64
		= fg_r 128
		= fg_g 128
		= fg_b 128
:default

	filled_rectangle bg_r bg_g bg_b 255 bg_r bg_g bg_b 255 bg_r bg_g bg_b 255 bg_r bg_g bg_b 255 0 (+ (* num 20) 5) 640 (- 360 (+ (* num 20) 5))
	string str
	= str "[A/Z/LMB] Navigate       [B/X/RMB] Launch Here"
	number w
	font_width font w str
	/ w 2
	font_draw font fg_r fg_g fg_b 255 str (- 320 w) (+ (* num 20) 10)

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
	if (== go_ok 0) cant
		return
:cant
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

	include "poll_joystick.inc"

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
