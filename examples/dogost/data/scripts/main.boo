var mmls
var files
= files (file_list)
var nfiles
= nfiles (vector_size files)
var i
for i 0 (< i nfiles) 1 check_file
	var matches
	= matches (string_matches [files i] "mml$")
	if (== TRUE matches) add_it
		var file
		= file [files i]
		= file (string_replace file ".*\\/(.*\\.mml)" "$1")
		vector_add mmls file
	:add_it
:check_file
var size
= size (vector_size mmls)

var font
= font (font_load "font.ttf" 16 1)
var fh
= fh (font_height font)

var selected
= selected 0

var W H
explode (get_screen_size) W H

var my_mml
= my_mml -1

function draw
{
	var y
	= y 10

	var i
	for i 0 (< i size) 1 draw_mml_name
		var r g b
		= r 255
		= g 255
		= b 255
		if (== selected i) draw_bar
			= r 0
			= g 0
			= b 0
			filled_rectangle 255 216 0 255 255 216 0 255 255 216 0 255 255 216 0 255 0 y W fh
		:draw_bar
		font_draw font r g b 255 [mmls i] 10 y
		= y (+ y fh)
	:draw_mml_name

	font_draw font 255 216 0 255 "Press ENTER to play. Press SPACE to stop." 10 (- 360 fh 10)
}

function event type a b c d
{
	if (== type EVENT_KEY_DOWN) its_a_key
		if (&& (== a KEY_UP) (> selected 0)) dec (&& (== a KEY_DOWN) (< selected (- size 1))) inc
			= selected (- selected 1)
		:dec
			= selected (+ selected 1)
		:inc
		if (&& (== a KEY_SPACE) (!= my_mml -1)) stop_it (== a KEY_RETURN) play_it
			mml_stop my_mml
			= my_mml -1
		:stop_it
			if (!= my_mml -1) stop_it2
				mml_stop my_mml
			:stop_it2
			= my_mml (mml_load [mmls selected])
			= VOID (mml_play my_mml 1.0 TRUE)
		:play_it
	:its_a_key
}
