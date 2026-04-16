var SCR_W SCR_H
explode (get_screen_size) SCR_W SCR_H
resize SCR_W SCR_H

const SEEK 1000

var have_music random prefix
= have_music FALSE

var file dir
= file (file_open "dir.txt" "r")
if (< file 0) def found
	= dir (string_format "%\\Music" (getenv "USERPROFILE"))
:def
	= dir (file_read_line file)
	= dir (string_trim dir)
:found

var mp3s mp3_dirs mp3_names
vector_add mp3s ""
vector_add mp3_dirs TRUE
vector_add mp3_names "Music"
call read_dir dir "Music"
var size
= size (vector_size mp3s)

var font_h
= font_h 18
var font fh lines
= font -1
call load_font

var selected top
= selected 0
= top 0

var my_mp3 my_inst curr
= my_mp3 -1
= curr -1

var do_play
= do_play FALSE

function draw
{
	clear 128 128 128

	var y
	= y 20

	var i
	for i top (&& (< i size) (< i (+ top lines))) 1 draw_mp3_name
		var r g b
		= r 255
		= g 255
		= b 255
		if (== selected i) draw_bar
			filled_rectangle 0 0 255 255 0 0 255 255 0 0 255 255 0 0 255 255 0 y SCR_W fh
		:draw_bar
		if (== i curr) highlight
			= r 0
			= g 255
			= b 255
		:highlight
		font_draw font r g b 255 [mp3_names i] 10 y FALSE
		= y (+ y fh)
	:draw_mp3_name

	var txt
	if (== TRUE [mp3_dirs selected]) dir_msg reg_msg
		= txt "ENTER: Play  R: Random  ESCAPE: Skip  SPACE: Stop  LEFT/RIGHT: Seek"
	:dir_msg
		= txt "ENTER: Play  ESCAPE: Skip  SPACE: Stop  LEFT/RIGHT: Seek"
	:reg_msg

	font_draw font 255 255 0 255 txt (- SCR_W (font_width font txt) 10) (- SCR_H fh 10)
		var y
		= y (+ (* lines fh) 40)
		triangle 255 0 255 255 15 (+ y 10) 10 y 20 y 2
	if (> top 0) draw_up
		triangle 255 0 255 255 15 1 10 11 20 11 2
	:draw_up
	if (< (+ top lines) size) draw_down
	:draw_down

	if (!= my_mp3 -1) draw_status
		var len elapsed
		= len (sample_length my_mp3)
		= elapsed (sample_elapsed my_mp3)
		var p
		= p (/ elapsed len)
		line 0 0 0 255 10 (- SCR_H 15) 160 (- SCR_H 15) 2
		filled_circle 255 255 255 255 (+ 10 (* p 150)) (- SCR_H 15) 4
		circle 0 0 0 255 (+ 10 (* p 150)) (- SCR_H 15) 4
	:draw_status
}

function event type a b c d
{
	if (== type EVENT_KEY_DOWN) its_a_key
		if (== a KEY_UP) dec (== a KEY_DOWN) inc (== a KEY_PAGEUP) pgup (== a KEY_PAGEDOWN) pgdn (== a KEY_HOME) _home (== a KEY_END) _end
			call cursor_up
		:dec
			call cursor_down
		:inc
			var i
			for i 0 (< i lines) 1 nl
				call cursor_up
			:nl
		:pgup
			var i
			for i 0 (< i lines) 1 nl2
				call cursor_down
			:nl2
		:pgdn
			var i
			for i 0 (< i size) 1 nl3
				call cursor_up
			:nl3
		:_home
			var i
			for i 0 (< i size) 1 nl4
				call cursor_down
			:nl4
		:_end
		if (&& (== a KEY_SPACE) (!= my_mp3 -1)) stop_it (== a KEY_RETURN) play_it (== a KEY_R) rando (== a KEY_MINUS) font_dec (== a KEY_EQUALS) font_inc
			sample_stop my_inst
			= my_mp3 -1
		:stop_it
			if (!= my_mp3 -1) stop_it2
				sample_destroy my_mp3
			:stop_it2
			= curr selected
			if (== TRUE [mp3_dirs selected]) dir file
				= prefix [mp3_names selected]
				call next_playlist
			:dir
				= prefix ""
			:file
			= random FALSE
			call play_mp3
		:play_it
			if (== TRUE [mp3_dirs selected]) ok
				if (!= my_mp3 -1) stop_it3
					sample_destroy my_mp3
				:stop_it3
				= prefix [mp3_names selected]
				= random TRUE
				call rand_track
				call play_mp3
			:ok
		:rando
			if (> font_h 12) dec_it
				= font_h (- font_h 1)
				call load_font
			:dec_it
		:font_dec
			if (< font_h 32) inc_it
				= font_h (+ font_h 1)
				call load_font
			:inc_it
		:font_inc
		if (&& (!= my_mp3 -1) (== a KEY_LEFT)) seek_left (&& (!= my_mp3 -1) (== a KEY_RIGHT)) seek_right (== a KEY_ESCAPE) skip
			var m
			= m (sample_elapsed my_inst)
			= m (max 0 (- m SEEK))
			sample_seek my_inst m
		:seek_left
			var len m
			= len (sample_length my_mp3)
			= m (sample_elapsed my_inst)
			= m (min len (+ m SEEK))
			sample_seek my_inst m
		:seek_right
			= do_play TRUE
		:skip
	:its_a_key
}

function run
{
	if (== do_play TRUE) go
		= do_play FALSE
		sample_destroy my_mp3
		= my_mp3 -1
		if (== TRUE random) r next
			call rand_track
		:r
			if (== prefix "") reg list
				:loop
				= curr (+ curr 1)
				if (>= curr size) zero
					= curr 0
				:zero
				if (&& (== TRUE [mp3_dirs curr]) (== TRUE have_music)) again
					goto loop
				:again
			:reg
				call next_playlist
			:list
		:next
		call play_mp3
	:go

	var w h
	explode (get_screen_size) w h
	if (|| (!= w SCR_W) (!= h SCR_H)) res
		resize w h
		= SCR_W w
		= SCR_H h
		call load_font
	:res
}

function fill_prefix
{
	var vec
	var len
	= len (string_length prefix)
	var i
	for i 0 (< i size) 1 next
		if (== FALSE [mp3_dirs i]) check
			if (== (string_substr [mp3_names i] 0 len) prefix) good
				vector_add vec i
			:good
		:check
	:next
	return vec
}

function rand_track
{
	var vec
	= vec (fill_prefix)
	var r
	= r (rand 0 (- (vector_size vec) 1))
	= curr [vec r]
}

function next_playlist
{
	var vec
	= vec (fill_prefix)

	var index
	for index 0 (< index (vector_size vec)) 1 again
		if (== curr [vec index]) done
			break
		:done
	:again

	= index (+ index 1)
	if (>= index (vector_size vec)) zero
		= index 0
	:zero
	= curr [vec index]
}

function mp3_end inst
{
	= do_play TRUE
}

function play_mp3
{
	= my_mp3 (sample_load [mp3s curr] TRUE)
	= my_inst (sample_play my_mp3 1.0 FALSE (sample_length my_mp3) mp3_end)
}

function read_dir dir concat
{
	var files dirs
	explode (list_directory (+ dir "/*")) files dirs
	var nfiles
	= nfiles (vector_size files)
	var i
	for i 0 (< i nfiles) 1 check_file
		var file
		= file [files i]
		if (== TRUE [dirs i]) is_dir is_file
			vector_add mp3s (+ dir "/" file)
			vector_add mp3_dirs TRUE
			vector_add mp3_names (+ concat "/" file)
			call read_dir (+ dir "/" file) (+ concat "/" file)
		:is_dir
			var lc
			= lc (string_lowercase file)
			var matches
			= matches (|| (string_matches lc "mp3$") (string_matches lc "ogg$") (string_matches lc "flac$") (string_matches lc "wav$"))
			if (== TRUE matches) add_it
				= have_music TRUE
				vector_add mp3s (+ dir "/" file)
				vector_add mp3_dirs FALSE
				vector_add mp3_names (+ concat "/" file)
			:add_it
		:is_file
	:check_file
}

function load_font
{
	if (!= font -1) destroy
		font_destroy font
	:destroy
	= font (font_load "c:/Windows/Fonts/arial.ttf" font_h TRUE TRUE)
	= fh (font_height font)
	= lines (/ (- SCR_H 80) fh)
}

function cursor_up
{
	= selected (- selected 1)
	if (< selected 0) zero
		= selected 0
	:zero
	if (< selected top) dec
		= top (- top 1)
	:dec
}

function cursor_down
{
	= selected (+ selected 1)
	if (>= selected size) dec
		= selected (- size 1)
	:dec
	if (>= selected (+ top lines)) inc
		= top (+ top 1)
	:inc
}
