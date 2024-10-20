number scanline_skip scanline_alpha
= scanline_skip 2
= scanline_alpha 128

number TOP
= TOP 20

number widget
mml_load widget "sfx/widget.mml"
number button
mml_load button "sfx/button.mml"

number W H
number font
number small_font
number fh
number num

call rsz

number more_down
image_load more_down "ui/more_down.png"

number go_ok
= go_ok 0

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

number old_pgup old_pgdn old_home old_end
= old_pgup 0
= old_pgdn 0
= old_home 0
= old_end 0

number selected
number top
vector filenames
string dir

vector argv
args argv
number i
number sz
vector_size argv sz
number found
= found 0
for i 0 (< i sz) 1 check_arg
	string arg
	= arg [argv i]
	if (&& (== arg "+dir") (< i (- sz 1))) list_other
		string s
		= s [argv (+ i 1)]
		call list_dir [argv (+ i 1)]
		= found 1
		goto done_args
	:list_other
:check_arg
:done_args

if (== found 0) list_current
	call list_dir "./"
:list_current

function rsz
{
	get_screen_size W H
	resize W H

	font_load font "font.ttf" (/ H 25) 1
	font_load small_font "font.ttf" (/ H 35) 1

	font_height font fh

	= num (- (- H (* fh 3.5)) 20)
	/ num fh
	floor num
}

function chop_dir s
{
	number p
	string_length p s
	- p 2
	if (< p 0) none
		return s
	:none
	number i
	for i p (>= i 0) -1 loop
		number c
		string_char_at c s i
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

function matches s regex
{
	number m
	string_matches m s regex
	return m
}

function list_dir name
{
	get_full_path name name
	string_replace name name "[\\\\]" "/"

	number len
	string_length len name
	string sub
	= sub name
	string_substr sub (- len 3)
	if (&& (== sub "../") (>= len 6)) check_collapse
		string sub
		= sub name
		string_substr sub (- len 6)
		if (&& (!= sub "../../") (!= sub "..\\../")) collapse
			number count
			= count 0
			number p
			for p (- len 1) (>= p 0) -1 scan
				number c
				string_char_at c name p
				string ch
				string_from_number ch c
				if (|| (== ch "/") (== ch "\\")) found_slash
					+ count 1
					if (== count 3) really_collapse
						string sub
						= sub name
						string_substr sub 0 (+ p 1)
						= name sub
						goto done_collapse
					:really_collapse
				:found_slash
			:scan
		:collapse
	:check_collapse
	:done_collapse

	= dir name
	+ name "*"
	list_directory filenames name

	if (|| (matches dir "^.:[/\\\\]$") (matches dir "^/$")) no_up has_up
		vector drives
		list_drives drives
		number sz
		vector_size drives sz
		number i
		number place
		= place 0
		for i 0 (< i sz) 1 add_drive
			string drive
			= drive [drives i]
			+ drive ":/"
			string sub
			= sub dir
			string_substr sub 0 1
			string tmp
			= tmp [drives i]
			if (!= sub [drives i]) really_add_drive
				vector_insert filenames place drive
				+ place 1
			:really_add_drive
		:add_drive
	:no_up
		vector_insert filenames 0 "../"
	:has_up

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
	= y TOP

	number bg_r bg_g bg_b fg_r fg_g fg_b
	= bg_r 255
	= bg_g 0
	= bg_b 216

	filled_rectangle 0 0 0 255 0 0 0 255 bg_r bg_g bg_b 255 bg_r bg_g bg_b 255 0 (- H (* fh 3.5)) W (* fh 3.5)
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
	font_draw font 255 255 255 255 "Enter Directory: A/Return/LMB" 25 (+ 5 (- H (* fh 3.5)))
	number w
	font_width font w found_text
	font_draw font found_r found_g found_b 255 found_text (- (- W w) 26) (+ 5 (- H (* fh 3.5)))

	number w
	font_width small_font w dir
	/ w 2
	number sfh
	font_height small_font sfh
	filled_rectangle 255 0 216 255 255 0 216 255 255 0 216 255 255 0 216 255 (- (/ W 2) w 5) (+ 10 fh (- H (* fh 3.5))) (+ 10 (* w 2)) (+ sfh 4)
	font_draw small_font 0 0 0 255 dir (- (/ W 2) w) (+ 12 fh (- H (* fh 3.5)))

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

	if (> top 0) draw_more_up
		image_draw more_down 0 216 255 255 (- (/ W 2) 7) (- TOP 8) 0 1
	:draw_more_up

	if (< (+ top num) sz) draw_more_down
		image_draw more_down 0 216 255 255 (- (/ W 2) 7) (+ TOP (* fh num)) 0 0
	:draw_more_down

	start_primitives

	number i
	for i 0 (< i H) scanline_skip do_scanline
		line 0 0 0 scanline_alpha 0 (+ i 0.5) W (+ i 0.5) 1
	:do_scanline

	end_primitives
}

function sel_up sound
{
	if (== sound 1) play
		mml_play widget 1 0
	:play
	- selected 1
	if (< selected 0) zero
		= selected 0
	:zero
	if (< selected top) dec_top
		= top selected
	:dec_top
}

function sel_down sound
{
	if (== sound 1) play
		mml_play widget 1 0
	:play
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
	mml_play button 1 0
	string s
	= s [filenames selected]
	call_result s chop_dir s
	number len
	string_length len s
	number c
	string_char_at c s (- len 1)
	string cs
	string_from_number cs c
	if (|| (== cs "/") (== cs "\\\\")) go_dir
		string d
		if (matches s "^.:[/\\\\]$") is_root_windows not_root
			= d s
		:is_root_windows
			= d dir
			+ d s
		:not_root
		call list_dir d
	:go_dir
}

function launch
{
	mml_play button 1 0
	if (== go_ok 0) cant
		return
	:cant
	number cfg
	cfg_load cfg "com.illnorth.launcher.reload"
	cfg_set_string cfg "launch" dir
	number success
	cfg_save cfg success "com.illnorth.launcher.reload"
	exit 0
}

function run
{
	number b1 b2 b3 wheel mx my
	mouse_get_buttons b1 b2 b3 wheel
	mouse_get_position mx my

	if (== wheel 1) wheel_up (== wheel -1) wheel_down
		call sel_up 1
	:wheel_up
		call sel_down 1
	:wheel_down

	if (&& (== b1 1) (== old_b1 0)) mouse_b1 (&& (== b3 1) (== old_b3 0)) mouse_b3
		if (&& (>= my TOP) (< my (+ TOP (* num fh)))) check_click
			number sel
			= sel (+ top (/ (- my TOP) fh))
			floor sel
			number sz
			vector_size filenames sz
			if (&& (>= sel 0) (< sel sz)) check_click2
				mml_play button 1 0
				if (== sel selected) nav select
					call navigate
				:nav
					= selected sel
				:select
			:check_click2
		:check_click
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

	number pgup pgdn home _end
	key_get pgup KEY_PAGEUP
	key_get pgdn KEY_PAGEDOWN
	key_get home KEY_HOME
	key_get _end KEY_END

	if (&& (== pgup 1) (== old_pgup 0)) do_pgup (&& (== pgdn 1) (== old_pgdn 0)) do_pgdn
		number i
		mml_play widget 1 0
		for i 0 (< i num) 1 pgup_slowly
			call sel_up 0
		:pgup_slowly
	:do_pgup
		number i
		mml_play widget 1 0
		for i 0 (< i num) 1 pgdn_slowly
			call sel_down 0
		:pgdn_slowly
	:do_pgdn

	= old_pgup pgup
	= old_pgdn pgdn

	number sz
	vector_size filenames sz
	number i

	if (&& (== home 1) (== old_home 0)) do_home (&& (== _end 1) (== old_end 0)) do_end
		mml_play widget 1 0
		for i 0 (< i sz) 1 home_slowly
			call sel_up 0
		:home_slowly
	:do_home
		mml_play widget 1 0
		for i 0 (< i sz) 1 end_slowly
			call sel_down 0
		:end_slowly
	:do_end

	= old_home home
	= old_end _end

	if (&& (== joy_u 1) (== old_u 0)) do_up (&& (== joy_d 1) (== old_d 0)) do_down
		call sel_up 1
	:do_up
		call sel_down 1
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
			call sel_up 1
		:repeat_u
	:check_u_repeat
		= u_time 0
	:zero_u_repeat

	if (== joy_d 1) check_d_repeat zero_d_repeat
		+ d_time 1
		if (> d_time 10) repeat_d
			= d_time 0
			call sel_down 1
		:repeat_d
	:check_d_repeat
		= d_time 0
	:zero_d_repeat

	= old_u joy_u
	= old_d joy_d
	= old_a joy_a
	= old_b joy_b

	number sw sh
	get_screen_size sw sh
	if (|| (!= sw W) (!= sh H)) do_rsz
		call rsz
	:do_rsz
}
