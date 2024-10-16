number music
mml_load music "music/game.mml"
mml_play music 0.5 1

number ship_img
image_load ship_img "misc/ship.png"
number enemy_img
image_load enemy_img "misc/enemy.png"
number coin_img
image_load coin_img "misc/coin.png"
number bullet_img
image_load bullet_img "misc/bullet.png"
number coin_sfx
mml_load coin_sfx "sfx/coin.mml"
number hit_sfx
mml_load hit_sfx "sfx/hit.mml"
number shoot_sfx
mml_load shoot_sfx "sfx/shoot.mml"
number shot_coin_sfx
mml_load shot_coin_sfx "sfx/shot_coin.mml"
number count_sfx
mml_create count_sfx "A a32"
number small_font
number big_font
font_load small_font "font.ttf" 12 1
font_load big_font "font.ttf" 36 1

number pos_x
number pos_y
number ship_a
number real_a
number ship_acc
number ship_vel
number dead
number dead_time
number collected

number screens
= screens 4

number size_w
number size_h
= size_w screens
* size_w 640
= size_h screens
* size_h 360

number ox
number oy

number ticks
= ticks 0
number next_fire_ticks
= next_fire_ticks 0
number game_ticks

vector stars
vector coins
vector enemies
vector bullets
vector explosions

number got_coins_ticks
= got_coins_ticks 0
number killed_all_ticks
= killed_all_ticks 0
number got_coins
= got_coins 0
number killed_all
= killed_all 0
number won
= won 0
number total_ticks
number total_time
number after_win_delay
= after_win_delay 300

number NUM_COINS
= NUM_COINS 7
number NUM_ENEMIES
= NUM_ENEMIES 32
number num_stars
= num_stars 1024

call start_game

function start_game
{
	call start_player
	call start_stars
	call start_coins
	call start_enemies

	vector_clear bullets
	vector_clear explosions

	= game_ticks 0
	= got_coins_ticks 0
	= killed_all_ticks 0
}

function start_player
{
	= pos_x size_w
	/ pos_x 2
	= pos_y size_h
	/ pos_y 2

	= ship_a 0
	= ship_acc 0
	= ship_vel 0
	= dead 0
	= dead_time 0
	= collected 0
}

function start_stars
{
	vector_clear stars

	number i
	= i 0
:add_another_star
	vector star
	number star_w
	= star_w size_w
	- star_w 1
	number star_h
	= star_h size_h
	- star_h 1
	number star_x
	number star_y
	rand star_x 0 star_w
	rand star_y 0 star_h
	vector_add star star_x
	vector_add star star_y
	vector_add stars star
	+ i 1
	? i num_stars
	jl add_another_star
}

function start_coins
{
	vector possibilities
	call_result possibilities get_quadrants

	vector_clear coins

	number i
	= i 0
:another_coin
	number sz
	vector_size possibilities sz
	- sz 1
	number r
	rand r 0 sz
	vector tmp
	vector_get possibilities tmp r
	vector_erase possibilities r
	number x
	number y
	vector_get tmp x 0
	vector_get tmp y 1
	number coin_x
	number coin_y
	= coin_x x
	* coin_x 640
	= coin_y y
	* coin_y 360
	number f
	= f 640
	- f 64
	rand r 0 f
	+ r 32
	+ coin_x r
	= f 360
	- f 64
	rand r 0 f
	+ r 32
	+ coin_y r
	vector coin
	vector_add coin coin_x
	vector_add coin coin_y
	vector_add coin 1
	vector_add coins coin
	+ i 1
	? i NUM_COINS
	jl another_coin
}

function start_enemies
{
	vector possibilities
	call_result possibilities get_quadrants

	vector_clear enemies

	number i
	= i 0
:add_another_enemy
	number sz
	vector_size possibilities sz
	- sz 1
	number r
	rand r 0 sz
	vector tmp
	vector_get possibilities tmp r
	number x
	number y
	vector_get tmp x 0
	vector_get tmp y 1
	number ex
	number ey
	= ex x
	* ex 640
	= ey y
	* ey 360
	number f
	= f 640
	- f 64
	rand f 0 f
	+ f 32
	+ ex f
	= f 360
	- f 64
	rand f 0 f
	+ f 32
	+ ey f
	vector tmp
	vector_add tmp ex
	vector_add tmp ey
	vector_add tmp 1
	vector next
	call_result next get_next_enemy_pos ex ey
	number next_x
	number next_y
	vector_get next next_x 0
	vector_get next next_y 1
	vector_add tmp next_x
	vector_add tmp next_y
	vector_add enemies tmp
	+ i 1
	? i NUM_ENEMIES
	jl add_another_enemy
}

function get_next_enemy_pos x y
{
:try_again
	number x_dir
	number y_dir
	rand x_dir 0 2
	rand y_dir 0 2
	- x_dir 1
	- y_dir 1
	number r
	rand r 0 1
	? r 0
	je blank_x
	= y_dir 0
	goto done_blank
:blank_x
	= x_dir 0
:done_blank
	? x_dir 0
	jne got_one
	? y_dir 0
	jne got_one
	goto try_again

:got_one
	number len
	rand len 32 256
	number xinc
	number yinc
	= xinc x_dir
	* xinc len
	= yinc y_dir
	* yinc len
	number new_x
	number new_y
	= new_x x
	+ new_x xinc
	= new_y y
	+ new_y yinc

	number max_x
	= max_x size_w
	- max_x 1
	- max_x 32

	number max_y
	= max_y size_h
	- max_y 1
	- max_y 32

	? new_x 32
	jl try_again
	? new_y 32
	jl try_again
	? new_x max_x
	jge try_again
	? new_y max_y
	jge try_again

	vector result
	vector_add result new_x
	vector_add result new_y
	return result
}

function get_quadrants
{
	vector v

	number i
	= i 0
	number x
	number y
	= y 0
:repeat_y
	= x 0
:repeat_x
	vector tmp
	vector_add tmp x
	vector_add tmp y
	vector_add v tmp
	+ x 1
	? x screens
	jl repeat_x
	+ y 1
	? y screens
	jl repeat_y

	return v
}

function calc_offset
{
	= ox pos_x
	- ox 320
	? ox 0
	jge do_x_offset2
	= ox 0
:do_x_offset2
	number max_x
	= max_x size_w
	- max_x 640
	? ox max_x
	jle do_y_offset
	= ox max_x

:do_y_offset
	= oy pos_y
	- oy 180
	? oy 0
	jge do_y_offset2
	= oy 0
:do_y_offset2
	number max_y
	= max_y size_h
	- max_y 360
	? oy max_y
	jle done_offset
	= oy max_y

:done_offset
}

function calc_real_a
{
	= real_a 3.14159
	* real_a 3
	/ real_a 2
	+ real_a ship_a
}

function bullet_collide bx by
{
	; Check coins

	number i

	= i 0
:next_coin
	vector coin
	vector_get coins coin i
	number cx
	number cy
	number exists
	vector_get coin cx 0
	vector_get coin cy 1
	vector_get coin exists 2
	? exists 0
	je skip_it
	number dx
	number dy
	= dx bx
	- dx cx
	= dy by
	- dy cy
	* dx dx
	* dy dy
	+ dx dy
	sqrt dx
	? dx 18
	jge skip_it
	vector_set coin 2 0
	vector_set coins i coin
	mml_play hit_sfx 1 0
	mml_play shot_coin_sfx 1 0
	call explode cx cy 64 251 242 54
	? dead 1
	jne die
	return 1
:die
	= dead 1
	call explode pos_x pos_y 64 255 255 255
	return 1
:skip_it
	+ i 1
	? i NUM_COINS
	jl next_coin

	; Check enemies

	number num_enemies
	vector_size enemies num_enemies

	= i 0
:check_next_enemy
	vector e
	vector_get enemies e i
	number ex
	number ey
	number exists
	vector_get e ex 0
	vector_get e ey 1
	vector_get e exists 2
	? exists 0
	je not_a_hit2
	number dx
	number dy
	= dx bx
	- dx ex
	= dy by
	- dy ey
	* dx dx
	* dy dy
	+ dx dy
	sqrt dx
	? dx 14
	jge not_a_hit2
	vector_set e 2 0
	vector_set enemies i e
	mml_play hit_sfx 1 0
	call explode ex ey 48 223 113 38
	return 1
:not_a_hit2
	+ i 1
	? i num_enemies
	jl check_next_enemy

	return 0
}

function explode x y max_r r g b
{
	vector e
	vector_add e x
	vector_add e y
	vector_add e 0
	vector_add e max_r
	vector_add e r
	vector_add e g
	vector_add e b

	vector_add explosions e
}

function zero_padded_string_from_number n
{
	string s

	? n 10
	jl pad
	string_format s "%" n
	goto done_pad
:pad
	string_format s "0%" n
:done_pad
	return s
}

function draw_time_playing
{
	; Time

	number seconds
	= seconds killed_all_ticks
	/ seconds 60
	floor seconds

	number hundredths
	= hundredths killed_all_ticks
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

	string time
	string_format time "%:%.%" ms ss hs

	number tx
	= tx 639
	- tx 16
	number w
	font_width small_font w time
	- tx w

	font_draw small_font 223 113 38 255 time tx 0

	number seconds
	= seconds got_coins_ticks
	/ seconds 60
	floor seconds

	number hundredths
	= hundredths got_coins_ticks
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

	string time
	string_format time "%:%.%" ms ss hs

	number tx
	= tx 639
	- tx 16
	number w2
	font_width small_font w2 time
	- tx w
	- tx w2
	- tx 16

	font_draw small_font 251 242 54 255 time tx 0
}

function draw_time_won
{
	number tmp
	number tmp2
	= tmp2 killed_all_ticks
	number remain
	= tmp got_coins_ticks
	- tmp total_time
	? tmp 0
	jge still_counting_coins_time
	= remain tmp
	neg remain
	= tmp 0

	- tmp2 remain

:still_counting_coins_time

	number bak1
	number bak2
	= bak1 got_coins_ticks
	= bak2 killed_all_ticks
	= got_coins_ticks tmp
	= killed_all_ticks tmp2

	call draw_time_playing

	= got_coins_ticks bak1
	= killed_all_ticks bak2
}

function draw_time
{
	? won 1
	jne no_win_draw
	call draw_time_won
	goto draw_time_end
:no_win_draw
	call draw_time_playing
:draw_time_end
}

function draw
{
	call calc_offset

	; Draw stars

	number i
	= i 0
	start_primitives
:draw_next_star
	vector star
	vector_get stars star i
	number star_x
	number star_y
	vector_get star star_x 0
	vector_get star star_y 1
	- star_x ox
	- star_y oy
	+ star_y 16
	if (&& (>= star_x 0) (>= star_y 0) (< star_x 640) (< star_y 360)) draw_it
		filled_rectangle 255 255 255 255 255 255 255 255 255 255 255 255 255 255 255 255 star_x star_y 1 1
	:draw_it
	+ i 1
	? i num_stars
	jl draw_next_star

	end_primitives

	; Draw coins

	image_start coin_img
	
	number coin_x_scale

	number tmp
	= tmp ticks
	% tmp 10
	? tmp 5
	jge pingpong
	/ tmp 5
	= coin_x_scale tmp
	goto done_coin_scale
:pingpong
	- tmp 5
	number tmp2
	= tmp2 5
	- tmp2 tmp
	/ tmp2 5
	= coin_x_scale tmp2

:done_coin_scale

	= i 0
:draw_next_coin
	vector tmpv
	vector_get coins tmpv i
	number coin_x
	number coin_y
	number coin_exists
	vector_get tmpv coin_x 0
	vector_get tmpv coin_y 1
	vector_get tmpv coin_exists 2
	? coin_exists 0
	je skip_coin
	- coin_x ox
	- coin_y oy
	+ coin_y 16
	image_draw_rotated_scaled coin_img 255 255 255 255 16 16 coin_x coin_y 0 coin_x_scale 1 0 0
:skip_coin
	+ i 1
	? i NUM_COINS
	jl draw_next_coin

	image_end coin_img

	; Draw enemies

	image_start enemy_img

	number e_angle
	= e_angle ticks
	% e_angle 30
	/ e_angle 30
	* e_angle 3.14159
	* e_angle 2

	number num_enemies
	vector_size enemies num_enemies

	= i 0
:draw_next_enemy
	vector e
	vector_get enemies e i
	number x
	number y
	number exists
	vector_get e x 0
	vector_get e y 1
	vector_get e exists 2
	? exists 0
	je done_draw_enemy
	number xx
	number yy
	= xx x
	- xx ox
	= yy y
	- yy oy
	if (&& (>= xx -10) (>= yy -10) (< xx 650) (< yy 370)) draw_the_enemy
		+ yy 16
		image_draw_rotated_scaled enemy_img 255 255 255 255 12 12 xx yy e_angle 1 1 0 0
	:draw_the_enemy
:done_draw_enemy
	+ i 1
	? i num_enemies
	jl draw_next_enemy

	image_end enemy_img

	; Draw bullets

	image_start bullet_img

	number num_bullets
	vector_size bullets num_bullets
	? num_bullets 0
	je no_bullets

	= i 0
:draw_next_bullet
	vector b
	vector_get bullets b i
	number bx
	number by
	vector_get b bx 0
	vector_get b by 1
	- bx ox
	- by oy
	if (&& (>= bx -5) (>= by -5) (< bx 645) (< by 365)) draw_the_bullet
		+ by 16
		image_draw_rotated_scaled bullet_img 255 255 255 255 4 4 bx by 0 1 1 0 0
	:draw_the_bullet
	+ i 1
	? i num_bullets
	jl draw_next_bullet

:no_bullets

	image_end bullet_img

	; Draw player

	? dead 1
	je skip_draw_player	

	number xx
	= xx pos_x
	- xx ox
	number yy
	= yy pos_y
	- yy oy

	+ yy 16

	image_draw_rotated_scaled ship_img 255 255 255 255 8 8 xx yy ship_a 1 1 0 0

:skip_draw_player
	
	; Draw explosions

	start_primitives

	number ne
	vector_size explosions ne
	? ne 0
	je no_explosions2

	= i 0
:draw_next_explosion
	vector ex
	vector_get explosions ex i
	number x
	number y
	number r
	number cr
	number cg
	number cb
	vector_get ex x 0
	vector_get ex y 1
	vector_get ex r 2
	vector_get ex cr 4
	vector_get ex cg 5
	vector_get ex cb 6
	- x ox
	- y oy
	+ y 16
	circle cr cg cb 255 x y r 1 -1
	+ i 1
	? i ne
	jl draw_next_explosion

:no_explosions2

	end_primitives

	; Draw status bar

	filled_rectangle 32 32 32 255 32 32 32 255 32 32 32 255 32 32 32 255 0 0 640 16

	call draw_time
	; Coins

	image_start coin_img

	? collected 0
	je after_draw_coin_icons
	number loops
	= loops collected
	number cx
	= cx 16
:draw_next_coin_icon
	image_stretch_region coin_img 255 255 255 255 0 0 32 32 cx 2 12 12 0 0
	+ cx 16
	- loops 1
	? loops 0
	jg draw_next_coin_icon

:after_draw_coin_icons

	image_end coin_img

	; Draw total time if won
	? won 0
	je done_draw

	filled_rectangle 16 16 16 128 16 16 16 128 16 16 16 128 16 16 16 128 0 140 640 80

	number seconds
	= seconds total_time
	/ seconds 60
	floor seconds

	number hundredths
	= hundredths total_time
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

	string time
	string_format time "%:%.%" ms ss hs

	number tx
	= tx 320
	number w
	font_width big_font w time
	/ w 2
	- tx w
	number ty
	= ty 180
	number h
	font_height big_font h
	/ h 2
	- ty h

	font_draw big_font 255 255 255 255 time tx ty

:done_draw
}

function write_config score
{
	number cfg
	cfg_load cfg "com.illnorth.coinhunt"
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
	floor score
	cfg_set_number cfg "last_score" score
	number success
	cfg_save cfg success "com.illnorth.coinhunt"
}

function run
{
	? killed_all 1
	jne no_win
	? got_coins 1
	jne no_win
	? won 0
	jne no_win
	= won 1
	= total_time 0
	= total_ticks got_coins_ticks
	+ total_ticks killed_all_ticks

:no_win

	? won 1
	jne dont_count_score
	
	number i
	= i 0
:next_score_count
	? total_ticks 0
	jle dont_count_score
	+ total_time 1
	- total_ticks 1
	mml_play count_sfx 1 0
	+ i 1
	? i 20
	jl next_score_count

:dont_count_score

	? won 1
	jne dont_exit
	? total_ticks 0
	jne dont_exit
	- after_win_delay 1
	? after_win_delay 0
	jg dont_exit
	call write_config total_time
	reset "enter_score.boo"

:dont_exit
	+ ticks 1
	+ next_fire_ticks 1

	? got_coins 1
	je check_killed_all
	+ got_coins_ticks 1

:check_killed_all
	? killed_all 1
	je after_killed_all_check
	+ killed_all_ticks 1

:after_killed_all_check
	? dead 0
	je no_tick
	+ dead_time 1
	? dead_time 300
	jl no_tick
	call start_game

:no_tick

	? dead 1
	je skip_player_movement
	
	+ game_ticks 1

	+ ship_vel ship_acc
	? ship_vel 8
	jle done_clamp
	= ship_vel 8

:done_clamp

	? ship_acc 0
	jne done_vel_damp
	- ship_vel 0.1
	? ship_vel 0
	jge done_vel_damp
	= ship_vel 0

:done_vel_damp

	include "poll_joystick.inc"

	? joy_back 1
	jne check_if_firing
	reset "main.boo"

:check_if_firing

	; Check if firing

	? joy_x 0
	jne do_fire
	? joy_b 0
	jne do_fire
	goto do_thrust
:do_fire
	? next_fire_ticks 5
	jl do_thrust
	= next_fire_ticks 0
	
	vector b
	number bx
	number by
	number ba
	call calc_real_a
	= ba real_a
	= bx pos_x
	= by pos_y
	number f
	= f ba
	cos f
	* f 8
	+ bx f
	= f ba
	sin f
	* f 8
	+ by f
	vector_add b bx
	vector_add b by
	vector_add b ba
	vector_add bullets b
	mml_play shoot_sfx 1 0

	; Do thrust

:do_thrust
	= ship_acc 0
	? joy_a 0
	je done_thrust
	= ship_acc 0.1
:done_thrust

	call calc_real_a

	number f

	= f real_a
	cos f
	* f ship_vel
	+ pos_x f

	= f real_a
	sin f
	* f ship_vel
	+ pos_y f

	? pos_x 32
	jge x_ok1
	= pos_x 32
:x_ok1
	number max_x
	= max_x size_w
	- max_x 1
	- max_x 32
	? pos_x max_x
	jle x_ok2
	= pos_x max_x
:x_ok2

	? pos_y 32
	jge y_ok1
	= pos_y 32
:y_ok1
	number max_y
	= max_y size_h
	- max_y 1
	- max_y 32
	? pos_y max_y
	jle y_ok2
	= pos_y max_y
:y_ok2

	number a_inc
	= a_inc 0

	? joy_l 0
	je no_rot_l
	- a_inc 0.1
:no_rot_l
	? joy_r 0
	je no_rot_r
	+ a_inc 0.1
:no_rot_r

	+ ship_a a_inc

	; Check if colliding with any coins

	number i
	= i 0
:check_next_coin
	vector tmp
	vector_get coins tmp i
	number coin_x
	number coin_y
	number coin_exists
	vector_get tmp coin_x 0
	vector_get tmp coin_y 1
	vector_get tmp coin_exists 2
	? coin_exists 0
	je not_a_hit
	number xx
	number yy
	= xx pos_x
	= yy pos_y
	- xx coin_x
	- yy coin_y
	* xx xx
	* yy yy
	+ xx yy
	number dist
	= dist xx
	sqrt dist
	? dist 18
	jge not_a_hit
	vector_set tmp 2 0
	vector_set coins i tmp
	mml_play coin_sfx 1 0
	+ collected 1
	? collected NUM_COINS
	jl not_a_hit
	= got_coins 1
:not_a_hit
	+ i 1
	? i NUM_COINS
	jl check_next_coin

	; Check for player-enemy collisions

	number ne
	vector_size enemies ne

	= i 0
:check_next_enemy_with_player
	vector e
	vector_get enemies e i
	number ex
	number ey
	number exists
	vector_get e ex 0
	vector_get e ey 1
	vector_get e exists 2
	? exists 0
	je player_didnt_hit_enemy
	number dx
	number dy
	= dx ex
	= dy ey
	- dx pos_x
	- dy pos_y
	* dx dx
	* dy dy
	+ dx dy
	sqrt dx
	? dx 14
	jge player_didnt_hit_enemy
	= dead 1
	call explode ex ey 48 223 113 38
	call explode pos_x pos_y 64 255 255 255
	mml_play hit_sfx 1 0
	vector_set e 2 0
	vector_set enemies i e
:player_didnt_hit_enemy
	+ i 1
	? i ne
	jl check_next_enemy_with_player

:skip_player_movement

	; Move the enemies

	number num_enemies
	vector_size enemies num_enemies

	= i 0
:move_next_enemy
	vector e
	vector_get enemies e i
	number ex
	number ey
	number exists
	number dest_x
	number dest_y
	vector_get e ex 0
	vector_get e ey 1
	vector_get e exists 2
	vector_get e dest_x 3
	vector_get e dest_y 4
	? ex dest_x
	je move_y
	jl move_right
:move_left
	- ex 1
	goto done_x
:move_right
	+ ex 1
:done_x
	number diff
	= diff ex
	- diff dest_x
	abs diff
	? diff 2
	jl set_next_dest
	goto done_move
:move_y
	? ey dest_y
	jl move_down
:move_up
	- ey 1
	goto done_y
:move_down
	+ ey 1
:done_y
	number diff
	= diff ey
	- diff dest_y
	abs diff
	? diff 2
	jl set_next_dest
	goto done_move
:set_next_dest
	vector next_dest
	call_result next_dest get_next_enemy_pos ex ey
	vector_get next_dest dest_x 0
	vector_get next_dest dest_y 1
:done_move
	vector_set e 0 ex
	vector_set e 1 ey
	vector_set e 3 dest_x
	vector_set e 4 dest_y
	vector_set enemies i e
	+ i 1
	? i num_enemies
	jl move_next_enemy

	; Move the bullets

	number num_bullets
	vector_size bullets num_bullets
	? num_bullets 0
	je no_bullets2

	= i 0
:move_next_bullet
	number was_erased
	= was_erased 0
	number bx
	number by
	number ba
	= bx [b 0]
	= by [b 1]
	= ba [b 2]
	number c
	= c ba
	cos c
	number s
	= s ba
	sin s
	number speed
	= speed 16
	number step
	= step 8
:next_step
	+ bx (* c step)
	+ by (* s step)
	number hit_something
	call_result hit_something bullet_collide bx by
	? hit_something 0
	je hit_nothing
	goto erase_it
:hit_nothing
	- speed step
	? speed 0
	jg next_step
:done_steps
	? bx 0
	jl erase_it
	? by 0
	jl erase_it
	number max_
	= max_ size_w
	- max_ 1
	? bx max_
	jge erase_it
	= max_ size_h
	- max_ 1
	? by max_
	jge erase_it
	goto in_zone
:erase_it
	vector_erase bullets i
	= was_erased 1
	- i 1
	- num_bullets 1
:in_zone
	? was_erased 1
	je no_update
	vector_set b 0 bx
	vector_set b 1 by
	vector_set bullets i b
:no_update
	+ i 1
	? i num_bullets
	jl move_next_bullet

:no_bullets2

	; Check if all enemies are dead
	number num_enemies
	vector_size enemies num_enemies
	number all_dead
	= all_dead 1
	number i
	= i 0
:check_if_next_enemy_is_dead
	vector e
	vector_get enemies e i
	number exists
	vector_get e exists 2
	? exists 1
	jne its_dead
	= all_dead 0
:its_dead
	+ i 1
	? i num_enemies
	jl check_if_next_enemy_is_dead

	? all_dead 1
	jne update_explosions
	= killed_all 1

	; Update explosions

:update_explosions
	number ne
	vector_size explosions ne
	? ne 0
	je no_explosions

	= i 0
:update_next_explosion
	vector exv
	vector_get explosions exv i
	number r
	number max_r
	vector_get exv r 2
	vector_get exv max_r 3
	+ r 5
	? r max_r
	jle its_fine
	vector_erase explosions i
	- i 1
	- ne 1
	goto finished_explosion_update
:its_fine
	vector_set exv 2 r
	vector_set explosions i exv
:finished_explosion_update
	+ i 1
	? i ne
	jl update_next_explosion

:no_explosions
}
