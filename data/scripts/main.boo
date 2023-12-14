number W H
= W 640
= H 360
;resize W H

number font
font_load font "font.ttf" 16 1

number fh
font_height font fh

number go_ok
= go_ok 0

number num
= num (- (- H (* fh 2.5)) 20)
/ num fh
floor num

number old_u old_d old_a old_b
= old_u 0
= old_d 0
= old_a 0
= old_b 0

number u_time d_time
= u_time 0
= d_time 0

number old_b1 old_b3
= old_b1 0
= old_b3 0

number old_esc
= old_esc 0

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
	= y 20

	number bg_r bg_g bg_b fg_r fg_g fg_b
	= bg_r 255
	= bg_g 0
	= bg_b 216

	filled_rectangle 0 0 0 255 0 0 0 255 bg_r bg_g bg_b 255 bg_r bg_g bg_b 255 0 (- H (* fh 2.5)) W (* fh 2.5)
	string found_text
	number found_r found_g found_b
	if (== go_ok 1) show_found not_found
		= found_text "BooBoo app detected! Launch: B/Space/RMB"
		= found_r 255
		= found_g 216
		= found_b 0
:show_found
		= found_text "No BooBoo app detected here..."
		= found_r 255
		= found_g 255
		= found_b 255
:not_found
	font_draw font 255 255 255 255 "Enter Directory: A/Return/LMB" 20 (+ 5 (- H (* fh 2.5)))
	number w
	font_width font w found_text
	font_draw font found_r found_g found_b 255 found_text (- (- W w) 21) (+ 5 (- H (* fh 2.5)))

	for i top (&& (< i sz) (< i (+ top num))) 1 loop
		string s
		= s [filenames i]
		call_result s chop_dir s
		number c
		= c 128
		if (== i selected) draw_bg
			= c 255
			filled_rectangle 0 0 0 255 0 0 0 255 0 216 255 255 0 216 255 255 0 y W (/ fh 2)
			filled_rectangle 0 216 255 255 0 216 255 255 0 0 0 255 0 0 0 255 0 (+ y (/ fh 2)) W (/ fh 2)
:draw_bg
		font_draw font c c c 255 s 25 y
		+ y fh
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

	number kret kspace
	key_get kret KEY_RETURN
	key_get kspace KEY_SPACE

	= joy_a (|| joy_a kret)
	= joy_b (|| joy_b kspace)

	number kesc
	key_get kesc KEY_ESCAPE
	if (&& (== kesc 1) (== old_esc 0)) quick_up
		= selected 0
		call navigate
:quick_up

	= old_esc kesc

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

	if (== joy_u 1) check_u_repeat zero_u_repeat
		+ u_time 1
		if (> u_time 10) repeat_u
			= u_time 0
			call sel_up
:repeat_u
:check_u_repeat
		= u_time 0
:zero_u_repeat

	if (== joy_d 1) check_d_repeat zero_d_repeat
		+ d_time 1
		if (> d_time 10) repeat_d
			= d_time 0
			call sel_down
:repeat_d
:check_d_repeat
		= d_time 0
:zero_d_repeat

	= old_u joy_u
	= old_d joy_d
	= old_a joy_a
	= old_b joy_b
}
