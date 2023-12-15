number font
font_load font "font.ttf" 96 1
number small_font
font_load small_font "font.ttf" 48 1

number drip_sfx
number drop_ground_sfx
mml_create drip_sfx "@PO0 = { 0 1000 }\nA o3 @TYPE3 @PO0 g32 @PO0"
mml_create drop_ground_sfx "@PO0 = { 0 -1000 0 1000 }\nA @TYPE3 o5 @PO0 a8 @PO0"

number cfg
cfg_load cfg "com.illnorth.rainrunner"
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
jge scored
reset "main.boo"
:scored
number place
number i
= i 0
:check_next_score
number s
vector_get scores s i
? last_score s
jl on_to_the_next_one
= place i
goto have_highscore
:on_to_the_next_one
+ i 1
? i 5
jl check_next_score
reset "main.boo"

:have_highscore
? place 4
jge no_shift
number i
= i 4
:next_shift
number j
= j i
- j 1
number s
string n
vector_get scores s j
vector_get names n j
vector_set scores i s
vector_set names i n
- i 1
? i place
jg next_shift
:no_shift
vector_set scores place last_score

number num_set
= num_set 0
number sel
= sel 0
string curr
= curr "A"

number old_joy_a
= old_joy_a 0
number old_joy_u
= old_joy_u 0
number old_joy_d
= old_joy_d 0

vector initials
vector_add initials " "
vector_add initials " "
vector_add initials " "

function write_scores name
{
	vector_set names place name

	number i
	= i 0
	:write_next_score
	number s
	string n
	vector_get names n i
	vector_get scores s i
	string str
	string_format str "score%" i
	cfg_set_number cfg str s
	string_format str "name%" i
	cfg_set_string cfg str n
	+ i 1
	? i 5
	jl write_next_score

	number success
	cfg_save cfg success "com.illnorth.rainrunner"
	reset "highscores.boo"
}

function draw
{
	string text
	= text "HIGH SCORE"
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
	font_draw font 0 216 255 255 text dx dy

	number dx
	= dx 320
	- dx 72

	= dy 50
	+ dy th
	+ dy 30

	? num_set 3
	jge done_draw_initials

	? num_set 0
	jle draw_curr

	number i
	= i 0
:draw_next_initial
	string s
	vector_get initials s i
	number dx2
	= dx2 dx
	number w
	font_width small_font w s
	/ w 2
	- dx2 w
	font_draw small_font 216 255 0 255 s dx2 dy
	+ dx 50
	+ i 1
	? i num_set
	jl draw_next_initial

:draw_curr
	number t
	get_ticks t
	% t 500
	? t 250
	jl done_draw_initials
	number dx2
	= dx2 dx
	number w
	font_width small_font w curr
	/ w 2
	- dx2 w
	font_draw small_font 216 255 0 255 curr dx2 dy

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
	vector_set initials num_set curr
	+ num_set 1
	mml_play drop_ground_sfx 1 0
	? num_set 3
	jl no_press
	string name
	string c1
	string c2
	string c3
	vector_get initials c1 0
	vector_get initials c2 1
	vector_get initials c3 2
	= name c1
	+ name c2
	+ name c3
	call write_scores name

:no_press

	? joy_d old_joy_d
	je check_joy_u
	= old_joy_d joy_d
	? joy_d 1
	jne check_joy_u
	mml_play drip_sfx 1 0
	- sel 1
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
	mml_play drip_sfx 1 0
	+ sel 1
	? sel 36
	jl no_change
	= sel 0
:no_change

	number ch
	? sel 26
	jge is_digit
	= ch sel
	+ ch 65
	string_from_number curr ch
	goto done_set_curr

:is_digit
	= ch sel
	- ch 26
	+ ch 48
	string_from_number curr ch

:done_set_curr
}
