function zero_padded_string_from_number n
{
	string s

	? n 10
	jl do_pad
	string_format s "%" n
	goto done_pad
:do_pad
	string_format s "0%" n
:done_pad
	return s
}

clear 0 0 0

number font
font_load font "font.ttf" 96 1
number small_font
font_load small_font "font.ttf" 32 1

number highlight
= highlight -1

number cfg
cfg_load cfg "com.nooskewl.coinhunt"
number exists
cfg_exists cfg exists "last_score"
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

vector names
vector scores
string n
number s
cfg_get_string cfg n "name0"
vector_add names n
cfg_get_string cfg n "name1"
vector_add names n
cfg_get_string cfg n "name2"
vector_add names n
cfg_get_string cfg n "name3"
vector_add names n
cfg_get_string cfg n "name4"
vector_add names n
cfg_get_number cfg s "score0"
vector_add scores s
cfg_get_number cfg s "score1"
vector_add scores s
cfg_get_number cfg s "score2"
vector_add scores s
cfg_get_number cfg s "score3"
vector_add scores s
cfg_get_number cfg s "score4"
vector_add scores s
number last_score
cfg_get_number cfg last_score "last_score"
? last_score 0
jl no_highlight
number i
= i 0
:next_highlight_check
number j
= j 4
- j i
number s
vector_get scores s j
? s last_score
jne continue_loop
= highlight j
goto no_highlight
:continue_loop
+ i 1
? i 5
jl next_highlight_check
:no_highlight
	
string text
= text "High Scores"
number tw
number th
font_width font tw text
font_height font th
/ tw 2
number dx
number dy
= dx 320
- dx tw
= dy 50
font_draw font 223 113 38 255 text dx dy

number dy
= dy 50
+ dy th
+ dy 20
number w
number h
= w 256
= h 32
number rect_x1
= rect_x1 320
- rect_x1 128

number i
= i 0
:draw_next_score2
number m
= m i
% m 2
? m 1
jne no_bg2
filled_rectangle 32 32 32 255 32 32 32 255 32 32 32 255 32 32 32 255 rect_x1 dy w h
:no_bg2
string text
number sc
vector_get names text i
vector_get scores sc i
string text2
number seconds
= seconds sc
/ seconds 60
floor seconds

number hundredths
= hundredths sc
/ hundredths 60
- hundredths seconds
* hundredths 100
floor hundredths

number minutes
= minutes seconds
/ minutes 60
floor minutes
number tmp
= tmp minutes
* tmp 60
- seconds tmp

string ms
string ss
string hs
call_result ms zero_padded_string_from_number minutes
call_result ss zero_padded_string_from_number seconds
call_result hs zero_padded_string_from_number hundredths

string_format text2 "%:%.%" ms ss hs
number tr
number tg
number tb
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
number i_x
number i_y
number small_h
font_height small_font small_h
number pad
= pad h
- pad small_h
/ pad 2
/ small_h 2
= i_x rect_x1
+ i_x pad
= i_y dy
number half
= half h
/ half 2
+ i_y half
- i_y small_h
;font_draw small_font tr tg tb 255 text i_x i_y
= i_x 320
+ i_x 128
- i_x pad
number sw
font_width small_font sw text2
- i_x sw
;font_draw small_font tr tg tb 255 text2 i_x i_y
+ dy h
+ i 1
? i 5
jl draw_next_score2

number dy
= dy 50
+ dy th
+ dy 20

number i
= i 0
:draw_next_score
number m
= m i
% m 2
? m 1
jne no_bg
;filled_rectangle 32 32 32 255 32 32 32 255 32 32 32 255 32 32 32 255 rect_x1 dy w h
:no_bg
string text
number sc
vector_get names text i
vector_get scores sc i
string text2
number seconds
= seconds sc
/ seconds 60
floor seconds

number hundredths
= hundredths sc
/ hundredths 60
- hundredths seconds
* hundredths 100
floor hundredths

number minutes
= minutes seconds
/ minutes 60
floor minutes
number tmp
= tmp minutes
* tmp 60
- seconds tmp

string ms
string ss
string hs
call_result ms zero_padded_string_from_number minutes
call_result ss zero_padded_string_from_number seconds
call_result hs zero_padded_string_from_number hundredths

string_format text2 "%:%.%" ms ss hs
number tr
number tg
number tb
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
number i_x
number i_y
number small_h
font_height small_font small_h
number pad
= pad h
- pad small_h
/ pad 2
/ small_h 2
= i_x rect_x1
+ i_x pad
= i_y dy
number half
= half h
/ half 2
+ i_y half
- i_y small_h
font_draw small_font tr tg tb 255 text i_x i_y
= i_x 320
+ i_x 128
- i_x pad
number sw
font_width small_font sw text2
- i_x sw
font_draw small_font tr tg tb 255 text2 i_x i_y
+ dy h
+ i 1
? i 5
jl draw_next_score

flip
delay 10000
cfg_set_number cfg "last_score" -1
number success
cfg_save cfg success "com.nooskewl.coinhunt"
reset "main.boo"
