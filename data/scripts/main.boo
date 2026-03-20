var scanline_skip scanline_alpha scanline_h
= scanline_h 2
= scanline_skip 4
= scanline_alpha 32
;= scanline_alpha 0 ; disable the effect

var TOP
= TOP 20

var widget
= widget (mml_load "sfx/widget.mml")
var button
= button (mml_load "sfx/button.mml")

var W H
var font
var small_font
var fh
var num

call rsz FALSE

var more_down
= more_down (image_load "ui/more_down.png")

var go_ok
= go_ok 0

var old_a old_b
= old_a 0
= old_b 0

var u_time d_time
= u_time 0
= d_time 0

var old_b1 old_b3
= old_b1 0
= old_b3 0

var old_esc
= old_esc 0

var old_home old_end
= old_home 0
= old_end 0

var selected
var top
var filenames
var is_dir
var dir

var argv
= argv (get_args)
var i
var sz
= sz (vector_size argv)
var found
= found 0
for i 0 (< i sz) 1 check_arg
	var arg
	= arg [argv i]
	if (&& (== arg "+dir") (< i (- sz 1))) list_other
		var s
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

var inst

function rsz destroy_fonts
{
	if (== destroy_fonts TRUE) destroy
		font_destroy font
		font_destroy small_font
	:destroy

	explode (get_screen_size) W H
	resize W H

	= font (font_load "astron-boy.ttf" (/ H 15) 1)
	= small_font (font_load "astron-boy.ttf" (/ H 25) 1)

	= fh (font_height font)

	= num (floor (/ (- (- H (* fh 3.5)) 20) fh))
}

function chop_dir s
{
	var p
	= p (string_length s)
	= p (- p 1)
	if (< p 0) none
		return s
	:none
	var i
	for i p (>= i 0) -1 loop
		var c
		= c (string_char_at s i)
		var cs
		= cs (string_from_number c)
		if (|| (== cs "/") (== cs "\\\\")) found
			= i (+ i 1)
			= s (string_substr s i)
			return s
		:found
	:loop

	return s
}

function matches s regex
{
	var m
	= m (string_matches s regex)
	return m
}

function list_dir name
{
	= name (get_full_path name)
	= name (string_replace name "[\\\\]" "/")

	var len
	= len (string_length name)
	var sub
	= sub (string_substr name (- len 2))
	if (&& (== sub "..") (>= len 5)) check_collapse
		var sub
		= sub (string_substr name (- len 5))
		if (&& (!= sub "../..") (!= sub "..\\..")) collapse
			var count
			= count 0
			var p
			for p (- len 1) (>= p 0) -1 scan
				var c
				= c (string_char_at name p)
				var ch
				= ch (string_from_number c)
				if (|| (== ch "/") (== ch "\\")) found_slash
					= count (+ count 1)
					if (== count 2) really_collapse
						var sub
						= sub (string_substr name 0 p)
						= name sub
						goto done_collapse
					:really_collapse
				:found_slash
			:scan
		:collapse
	:check_collapse
	:done_collapse

	= dir name
	= name (+ name "/*")
	explode (list_directory name) filenames is_dir

	if (|| (matches dir "^[A-Z]:/$") (matches dir "^/$")) no_up has_up
		var drives
		= drives (list_drives)
		var sz
		= sz (vector_size drives)
		var i
		var place
		= place 0
		for i 0 (< i sz) 1 add_drive
			var drive
			= drive [drives i]
			= drive (+ drive ":")
			var sub
			= sub (string_substr dir 0 1)
			var tmp
			= tmp [drives i]
			if (!= sub [drives i]) really_add_drive
				vector_insert filenames place drive
				vector_insert is_dir place TRUE
				= place (+ place 1)
			:really_add_drive
		:add_drive
	:no_up
		vector_insert filenames 0 ".."
		vector_insert is_dir 0 TRUE
	:has_up

	= selected 0
	= top 0
	= go_ok 0
	var sz
	= sz (vector_size filenames)
	var i
	for i 0 (< i sz) 1 loop_good
		var s
		= s [filenames i]
		call_result s chop_dir s
		if (== s "data.cpa") is_cpa
			= go_ok 1
			goto done_list
		:is_cpa
		if (== s "data") is_data_dir
			var orig
			= orig (+ [filenames i] "/scripts/main.boo")
			var f
			= f (file_open orig "r")
			if (!= -1 f) is_booboo_app
				file_close f
				= go_ok 1
				goto done_list
			:is_booboo_app
		:is_data_dir
	:loop_good
:done_list
	for i 0 (< i sz) 1 loop_bad
		var s
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
	var sz
	= sz (vector_size filenames)
	var i
	var y
	= y TOP

	var bg_r bg_g bg_b fg_r fg_g fg_b
	= bg_r 255
	= bg_g 0
	= bg_b 216

	filled_rectangle 0 0 0 255 0 0 0 255 bg_r bg_g bg_b 255 bg_r bg_g bg_b 255 0 (- H (* fh 3.5)) W (* fh 3.5)
	var found_text
	var found_r found_g found_b
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
	var w
	= w (font_width font found_text)
	font_draw font found_r found_g found_b 255 found_text (- (- W w) 26) (+ 5 (- H (* fh 2.5)))

	var w
	= w (/ (font_width small_font dir) 2)
	var sfh
	= sfh (font_height small_font)
	filled_rectangle 255 0 216 255 255 0 216 255 255 0 216 255 255 0 216 255 (- (/ W 2) w 5) (+ 10 fh (- H (* fh 2.5))) (+ 10 (* w 2)) (+ sfh 4)
	font_draw small_font 0 0 0 255 dir (- (/ W 2) w) (+ 12 fh (- H (* fh 2.5)))

	for i top (&& (< i sz) (< i (+ top num))) 1 loop
		var s
		= s [filenames i]
		call_result s chop_dir s
		if (== [is_dir i] TRUE) add_slash
			= s (+ s "/")
		:add_slash
		var c
		= c 128
		if (== i selected) draw_bg
			= c 255
			filled_rectangle 0 0 0 255 0 0 0 255 0 216 255 255 0 216 255 255 0 y W (/ fh 2)
			filled_rectangle 0 216 255 255 0 216 255 255 0 0 0 255 0 0 0 255 0 (+ y (/ fh 2)) W (/ fh 2)
		:draw_bg
		font_draw font c c c 255 s 25 y FALSE
		= y (+ y fh)
	:loop

	if (> top 0) draw_more_up
		image_draw more_down 0 216 255 255 (- (/ W 2) 7) (- TOP 8) 0 1
	:draw_more_up

	if (< (+ top num) sz) draw_more_down
		image_draw more_down 0 216 255 255 (- (/ W 2) 7) (+ TOP (* fh num)) 0 0
	:draw_more_down

	start_primitives

	var i
	for i 0 (< i H) scanline_skip do_scanline
		line 0 0 0 scanline_alpha 0 (+ i 0.5) W (+ i 0.5) scanline_h
	:do_scanline

	end_primitives
}

function sel_up sound
{
	if (== sound 1) play
		= inst (mml_play widget 1 0)
	:play
	= selected (- selected 1)
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
		= inst (mml_play widget 1 0)
	:play
	var sz
	= sz (vector_size filenames)
	= selected (+ selected 1)
	if (>= selected sz) fix
		= selected (- sz 1)
	:fix
	if (<= (+ top num) selected) adjust
		= top (- selected (- num 1))
	:adjust
}

function navigate
{
	= inst (mml_play button 1 0)
	var s
	= s [filenames selected]
	call_result s chop_dir s
	if (== TRUE [is_dir selected]) go_dir
		var d
		if (matches s "^[A-Z]:$") is_root_windows not_root
			= d (+ s "/")
		:is_root_windows
			= d (+ dir "/" s)
		:not_root
		call list_dir d
	:go_dir
}

function launch
{
	= inst (mml_play button 1 0)
	if (== go_ok 0) cant
		return
	:cant
	var cfg
	= cfg (cfg_load "com.nooskewl.launcher.reload")
	cfg_set_string cfg "launch" dir
	var success
	= success (cfg_save cfg "com.nooskewl.launcher.reload")
	exit 0
}

function run
{
	var b1 b2 b3 wheel mx my
	explode (mouse_get_buttons) b1 b2 b3 wheel
	explode (mouse_get_position) mx my

	if (&& (== b1 1) (== old_b1 0)) mouse_b1 (&& (== b3 1) (== old_b3 0)) mouse_b3
		if (&& (>= my TOP) (< my (+ TOP (* num fh)))) check_click
			var sel
			= sel (floor (+ top (/ (- my TOP) fh)))
			var sz
			= sz (vector_size filenames)
			if (&& (>= sel 0) (< sel sz)) check_click2
				= inst (mml_play button 1 0)
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

	var sz
	= sz (vector_size filenames)

	include "poll_joystick.inc"

	var kret kspace
	= kret (key_get KEY_RETURN)
	= kspace (key_get KEY_SPACE)

	= joy_a (|| joy_a kret)
	= joy_b (|| joy_b kspace)

	var kesc
	= kesc (key_get KEY_ESCAPE)
	if (&& (== kesc 1) (== old_esc 0)) quick_up
		= selected 0
		call navigate
	:quick_up

	= old_esc kesc

	var home _end
	= home (key_get KEY_HOME)
	= _end (key_get KEY_END)

	var sz
	= sz (vector_size filenames)
	var i

	if (&& (== home 1) (== old_home 0)) do_home (&& (== _end 1) (== old_end 0)) do_end
		= inst (mml_play widget 1 0)
		for i 0 (< i sz) 1 home_slowly
			call sel_up 0
		:home_slowly
	:do_home
		= inst (mml_play widget 1 0)
		for i 0 (< i sz) 1 end_slowly
			call sel_down 0
		:end_slowly
	:do_end

	= old_home home
	= old_end _end

	if (&& (== joy_a 1) (== old_a 0)) dig
		call navigate
	:dig

	if (&& (== joy_b 1) (== old_b 0)) go
		call launch
	:go

	= old_a joy_a
	= old_b joy_b

	var sw sh
	explode (get_screen_size) sw sh
	if (|| (!= sw W) (!= sh H)) do_rsz
		call rsz TRUE
	:do_rsz
}

function event type a b c d
{
	if (== type EVENT_MOUSE_WHEEL) is_wheel
		var old_top
		= old_top top
		var i
		if (> b 0) is_up (< b 0) is_down
			for i 0 (< i b) 1 do_up
				call sel_up (== i 0)
			:do_up
			if (== top old_top) dec_top
				= top (- top b)
				if (< top 0) equal_0
					= top 0
				:equal_0
			:dec_top
		:is_up
			for i 0 (< i (* b -1)) 1 do_down
				call sel_down (== i 0)
			:do_down
			if (== top old_top) inc_top
				= top (+ top (* b -1))
				var sz
				= sz (vector_size filenames)
				if (> (+ top num) sz) adjust2
					= top (- sz num)
					if (< top 0) zero
						= top 0
					:zero
				:adjust2
			:inc_top
		:is_down
	:is_wheel

	if (== type EVENT_JOY_DOWN) is_joy_b
		if (== b JOY_U) is_joy_up (== b JOY_D) is_joy_down
			call sel_up 1
		:is_joy_up
			call sel_down 1
		:is_joy_down
	:is_joy_b

	var _do_pgup _do_pgdn
	= _do_pgup FALSE
	= _do_pgdn FALSE

	if (== type EVENT_KEY_DOWN) is_key_b
		if (== a KEY_UP) is_key_up (== a KEY_DOWN) is_key_down (== a KEY_PAGEUP) do_pgup (== a KEY_PAGEDOWN) do_pgdn
			call sel_up 1
		:is_key_up
			call sel_down 1
		:is_key_down
			= _do_pgup TRUE
		:do_pgup
			= _do_pgdn TRUE
		:do_pgdn
 	:is_key_b

	if (== type EVENT_JOY_DOWN) do_joy_down
		if (== b JOY_LB) joy_pgup (== b JOY_RB) joy_pgdn
			= _do_pgup TRUE
		:joy_pgup
			= _do_pgdn TRUE
		:joy_pgdn
	:do_joy_down

	if (== 1 _do_pgup) pgup (== 1 _do_pgdn) pgdn
		var i
		= inst (mml_play widget 1 0)
		for i 0 (< i num) 1 pgup_slowly
			call sel_up 0
		:pgup_slowly
	:pgup
		var i
		= inst (mml_play widget 1 0)
		for i 0 (< i num) 1 pgdn_slowly
			call sel_down 0
		:pgdn_slowly
	:pgdn
}
