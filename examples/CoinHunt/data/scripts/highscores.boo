function zero_padded_string_from_number n
{
	var s

	? n 10
	jl do_pad
	= s (string_format "%" n)
	goto done_pad
:do_pad
	= s (string_format "0%" n)
:done_pad
	return s
}

clear 0 0 0

var font
= font (font_load "skateblade.ttf" 92 1)
var small_font
= small_font (font_load "font.ttf" 32 1)

var highlight
= highlight -1

var cfg
= cfg (cfg_load "com.nooskewl.coinhunt")
var exists
= exists (cfg_exists cfg "last_score")
? exists 0
jne dont_set_defaults
cfg_set_number cfg "score0" 14400
cfg_set_number cfg "score1" 18000
cfg_set_number cfg "score2" 21600
cfg_set_number cfg "score3" 25200
cfg_set_number cfg "score4" 28800
cfg_set_string cfg "name0" "ILL"
cfg_set_string cfg "name1" "ILL"
cfg_set_string cfg "name2" "ILL"
cfg_set_string cfg "name3" "ILL"
cfg_set_string cfg "name4" "ILL"
:dont_set_defaults

var names
var scores
var n
var s
= n (cfg_get_string cfg "name0")
vector_add names n
= n (cfg_get_string cfg "name1")
vector_add names n
= n (cfg_get_string cfg "name2")
vector_add names n
= n (cfg_get_string cfg "name3")
vector_add names n
= n (cfg_get_string cfg "name4")
vector_add names n
= s (cfg_get_number cfg "score0")
vector_add scores s
= s (cfg_get_number cfg "score1")
vector_add scores s
= s (cfg_get_number cfg "score2")
vector_add scores s
= s (cfg_get_number cfg "score3")
vector_add scores s
= s (cfg_get_number cfg "score4")
vector_add scores s
var last_score
= last_score (cfg_get_number cfg "last_score")
? last_score 0
jl no_highlight
var i
= i 0
:next_highlight_check
var j
= j 4
= j (- j i)
var s
= s [scores j]
? s last_score
jne continue_loop
= highlight j
goto no_highlight
:continue_loop
= i (+ i 1)
? i 5
jl next_highlight_check
:no_highlight
	
var text
= text "High Scores"
var tw
var th
= tw (font_width font text)
= th (font_height font)
= tw (/ tw 2)
var dx
var dy
= dx (- 320 tw)
= dy 50
font_draw font 223 113 38 255 text dx dy

var dy
= dy (+ 50 th 20)
var w
var h
= w 256
= h 32
var rect_x1
= rect_x1 (- 320 128)

var i
= i 0
:draw_next_score2
var m
= m i
= m (% m 2)
? m 1
jne no_bg2
filled_rectangle 32 32 32 255 32 32 32 255 32 32 32 255 32 32 32 255 rect_x1 dy w h
:no_bg2
var text
var sc
= text [names i]
= sc [scores i]
var text2
var seconds
= seconds (floor (/ sc 60))

var hundredths
= hundredths (floor (* (- (/ sc 60) seconds) 100))

var minutes
= minutes (floor (/ seconds 60))
var tmp
= tmp (* minutes 60)
= seconds (- seconds tmp)

var ms
var ss
var hs
call_result ms zero_padded_string_from_number minutes
call_result ss zero_padded_string_from_number seconds
call_result hs zero_padded_string_from_number hundredths

= text2 (string_format "%:%.%" ms ss hs)
var tr
var tg
var tb
? i highlight
jne white2
= tr 251
= tg 242
= tb 54
goto after_colour2
:white2
= tr 255
= tg 255
= tb 255
:after_colour2
var i_x
var i_y
var small_h
= small_h (font_height small_font)
var pad
= pad (/ (- h small_h) 2)
= small_h (/ small_h 2)
= i_x (+ rect_x1 pad)
= i_y dy
var half
= half (/ h 2)
= i_y (- (+ i_y half) small_h)
;font_draw small_font tr tg tb 255 text i_x i_y
= i_x (- (+ 320 128) pad)
var sw
= sw (font_width small_font text2)
= i_x (- i_x sw)
;font_draw small_font tr tg tb 255 text2 i_x i_y
= dy (+ dy h)
= i (+ i 1)
? i 5
jl draw_next_score2

var dy
= dy (+ 50 th 20)

var i
= i 0
:draw_next_score
var m
= m (% i 2)
? m 1
jne no_bg
;filled_rectangle 32 32 32 255 32 32 32 255 32 32 32 255 32 32 32 255 rect_x1 dy w h
:no_bg
var text
var sc
= text [names i]
= sc [scores i]
var text2
var seconds
= seconds (floor (/ sc 60))

var hundredths
= hundredths (floor (* (- (/ sc 60) seconds) 100))

var minutes
= minutes (floor (/ seconds 60))
var tmp
= tmp (* minutes 60)
= seconds (- seconds tmp)

var ms
var ss
var hs
call_result ms zero_padded_string_from_number minutes
call_result ss zero_padded_string_from_number seconds
call_result hs zero_padded_string_from_number hundredths

= text2 (string_format "%:%.%" ms ss hs)
var tr
var tg
var tb
? i highlight
jne white
= tr 251
= tg 242
= tb 54
goto after_colour
:white
= tr 255
= tg 255
= tb 255
:after_colour
var i_x
var i_y
var small_h
= small_h (font_height small_font)
var pad
= pad (/ (- h small_h) 2)
= small_h (/ small_h 2)
= i_x (+ rect_x1 pad)
= i_y dy
var half
= half (/ h 2)
= i_y (- (+ i_y half) small_h)
font_draw small_font tr tg tb 255 text i_x i_y
= i_x (- (+ 320 128) pad)
var sw
= sw (font_width small_font text2)
= i_x (- i_x sw)
font_draw small_font tr tg tb 255 text2 i_x i_y
= dy (+ dy h)
= i (+ i 1)
? i 5
jl draw_next_score

flip
delay 10000
cfg_set_number cfg "last_score" -1
var success
= success (cfg_save cfg "com.nooskewl.coinhunt")
reset "main.boo"
