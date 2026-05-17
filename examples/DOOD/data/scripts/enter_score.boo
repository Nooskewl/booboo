var font
= font (font_load "Horror.ttf" 88 1)
var small_font
= small_font (font_load "font.ttf" 48 1)

var drip_sfx
var drop_ground_sfx
= drip_sfx (mml_create "@PO0 = { 0 1000 0 1000 0 }\nA o3 @TYPE3 @PO0 g32 @PO0")
= drop_ground_sfx (mml_create "@PO0 = { 0 1000 0 1000 0 }\nA o4 @TYPE3 @PO0 g32 @PO0")

var cfg
= cfg (cfg_load "com.nooskewl.doomed")
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
jge scored
reset "main.boo"
:scored
var place
var i
= i 0
:check_next_score
var s
= s [scores i]
? last_score s
jl on_to_the_next_one
= place i
goto have_highscore
:on_to_the_next_one
= i (+ i 1)
? i 5
jl check_next_score
reset "main.boo"

:have_highscore
? place 4
jge no_shift
var i
= i 4
:next_shift
var j
= j (- i 1)
var s
var n
= s [scores j]
= n [names j]
= [scores i] s
= [names i] n
= i (- i 1)
? i place
jg next_shift
:no_shift
= [scores place] last_score

var num_set
= num_set 0
var sel
= sel 0
var curr
= curr "A"

var old_joy_a
= old_joy_a 0
var old_joy_u
= old_joy_u 0
var old_joy_d
= old_joy_d 0

var initials
vector_add initials " "
vector_add initials " "
vector_add initials " "

function write_scores name
{
	= [names place] name

	var i
	= i 0
	:write_next_score
	var s
	var n
	= n [names i]
	= s [scores i]
	var str
	= str (string_format "score%" i)
	cfg_set_number cfg str s
	= str (string_format "name%" i)
	cfg_set_string cfg str n
	= i (+ i 1)
	? i 5
	jl write_next_score

	var success
	= success (cfg_save cfg "com.nooskewl.doomed")
	reset "highscores.boo"
}

function draw
{
	var text
	= text "HIGH SCORE"
	var tw
	var th
	= tw (font_width font text)
	= th (font_height font)
	= tw (/ tw 2)
	var dx
	var dy
	= dx (- 320 tw)
	= dy 50
	font_draw font 255 255 255 255 text dx dy

	var dx
	= dx (- 320 72)

	= dy (+ 50 th 20)

	? num_set 3
	jge done_draw_initials

	? num_set 0
	jle draw_curr

	var i
	= i 0
:draw_next_initial
	var s
	= s [initials i]
	var dx2
	= dx2 dx
	var w
	= w (font_width small_font s)
	= w (/ w 2)
	= dx2 (- dx2 w)
	font_draw small_font 0 255 0 255 s dx2 dy
	= dx (+ dx 50)
	= i (+ i 1)
	? i num_set
	jl draw_next_initial

:draw_curr
	var t
	= t (% (get_ticks) 500)
	? t 250
	jl done_draw_initials
	var dx2
	= dx2 dx
	var w
	= w (font_width small_font curr)
	= w (/ w 2)
	= dx2 (- dx2 w)
	font_draw small_font 0 255 0 255 curr dx2 dy

:done_draw_initials
}

function run
{
	include "poll_joystick.inc"

	? joy_a old_joy_a
	je no_press
	= old_joy_a joy_a
	? joy_a 1
	jne no_press
	= [initials num_set] curr
	= num_set (+ num_set 1)
	= VOID (mml_play drop_ground_sfx 1 0)
	? num_set 3
	jl no_press
	var name
	var c1
	var c2
	var c3
	= c1 [initials 0]
	= c2 [initials 1]
	= c3 [initials 2]
	= name (+ c1 c2 c3)
	call write_scores name

:no_press

	? joy_d old_joy_d
	je check_joy_u
	= old_joy_d joy_d
	? joy_d 1
	jne check_joy_u
	= VOID (mml_play drip_sfx 1 0)
	= sel (- sel 1)
	? sel 0
	jge check_joy_u
	= sel 35 ; A-Z and 0-9
	goto no_change
:check_joy_u
	? joy_u old_joy_u
	je no_change
	= old_joy_u joy_u
	? joy_u 1
	jne no_change
	= VOID (mml_play drip_sfx 1 0)
	= sel (+ sel 1)
	? sel 36
	jl no_change
	= sel 0
:no_change

	var ch
	? sel 26
	jge is_digit
	= ch (+ sel 65)
	= curr (string_from_number ch)
	goto done_set_curr

:is_digit
	= ch (+ (- sel 26) 48)
	= curr (string_from_number ch)

:done_set_curr
}
