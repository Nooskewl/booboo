var music
= music (mml_load "music/game.mml")
mml_play music 0.5 1

var ship_img
= ship_img (image_load "misc/ship.png")
var enemy_img
= enemy_img (image_load "misc/enemy.png")
var coin_img
= coin_img (image_load "misc/coin.png")
var bullet_img
= bullet_img (image_load "misc/bullet.png")
var coin_sfx
= coin_sfx (mml_load "sfx/coin.mml")
var hit_sfx
= hit_sfx (mml_load "sfx/hit.mml")
var shoot_sfx
= shoot_sfx (mml_load "sfx/shoot.mml")
var shot_coin_sfx
= shot_coin_sfx (mml_load "sfx/shot_coin.mml")
var count_sfx
= count_sfx (mml_create "A a32")
var small_font
var big_font
= small_font (font_load "font.ttf" 12 1)
= big_font (font_load "font.ttf" 36 1)

var pos_x
var pos_y
var ship_a
var real_a
var ship_acc
var ship_vel
var dead
var dead_time
var collected

var screens
= screens 4

var size_w
var size_h
= size_w (* screens 640)
= size_h (* screens 360)

var ox
var oy

var ticks
= ticks 0
var next_fire_ticks
= next_fire_ticks 0
var game_ticks

var stars
var coins
var enemies
var bullets
var explosions

var got_coins_ticks
= got_coins_ticks 0
var killed_all_ticks
= killed_all_ticks 0
var got_coins
= got_coins 0
var killed_all
= killed_all 0
var won
= won 0
var total_ticks
var total_time
var after_win_delay
= after_win_delay 300

var NUM_COINS
= NUM_COINS 7
var NUM_ENEMIES
= NUM_ENEMIES 32
var num_stars
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
	= pos_x (/ size_w 2)
	= pos_y (/ size_h 2)

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

	var i
	= i 0
:add_another_star
	var star
	var star_w
	= star_w (- size_w 1)
	var star_h
	= star_h (- size_h 1)
	var star_x
	var star_y
	= star_x (rand 0 star_w)
	= star_y (rand 0 star_h)
	vector_add star star_x
	vector_add star star_y
	vector_add stars star
	= i (+ i 1)
	? i num_stars
	jl add_another_star
}

function start_coins
{
	var possibilities
	call_result possibilities get_quadrants

	vector_clear coins

	var i
	= i 0
:another_coin
	var sz
	= sz (- (vector_size possibilities) 1)
	var r
	= r (rand 0 sz)
	var tmp
	= tmp [possibilities r]
	vector_erase possibilities r
	var x
	var y
	explode tmp x y
	var coin_x
	var coin_y
	= coin_x (* x 640)
	= coin_y (* y 360)
	var f
	= f (- 640 64)
	= r (+ (rand 0 f) 32)
	= coin_x (+ coin_x r)
	= f (- 360 64)
	= r (+ (rand 0 f) 32)
	= coin_y (+ coin_y r)
	var coin
	vector_add coin coin_x
	vector_add coin coin_y
	vector_add coin 1
	vector_add coins coin
	= i (+ i 1)
	? i NUM_COINS
	jl another_coin
}

function start_enemies
{
	var possibilities
	call_result possibilities get_quadrants

	vector_clear enemies

	var i
	= i 0
:add_another_enemy
	var sz
	= sz (- (vector_size possibilities) 1)
	var r
	= r (rand 0 sz)
	var tmp
	= tmp [possibilities r]

	var x
	var y
	explode tmp x y
	var ex
	var ey
	= ex (* x 640)
	= ey (* y 360)
	var f
	= f (- 640 64)
	= f (+ (rand 0 f) 32)
	= ex (+ ex f)
	= f (- 360 64)
	= f (+ (rand 0 f) 32)
	= ey (+ ey f)
	var tmp
	vector_add tmp ex
	vector_add tmp ey
	vector_add tmp 1
	var next
	call_result next get_next_enemy_pos ex ey
	var next_x
	var next_y
	explode next next_x next_y
	vector_add tmp next_x
	vector_add tmp next_y
	vector_add enemies tmp
	= i (+ i 1)
	? i NUM_ENEMIES
	jl add_another_enemy
}

function get_next_enemy_pos x y
{
:try_again
	var x_dir
	var y_dir
	= x_dir (- (rand 0 2) 1)
	= y_dir (- (rand 0 2) 1)
	var r
	= r (rand 0 1)
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
	var len
	= len (rand 32 256)
	var xinc
	var yinc
	= xinc (* x_dir len)
	= yinc (* y_dir len)
	var new_x
	var new_y
	= new_x (+ x xinc)
	= new_y (+ y yinc)

	var max_x
	= max_x (- size_w 1 32)

	var max_y
	= max_y (- size_h 1 32)

	? new_x 32
	jl try_again
	? new_y 32
	jl try_again
	? new_x max_x
	jge try_again
	? new_y max_y
	jge try_again

	var result
	vector_add result new_x
	vector_add result new_y
	return result
}

function get_quadrants
{
	var v

	var i
	= i 0
	var x
	var y
	= y 0
:repeat_y
	= x 0
:repeat_x
	var tmp
	vector_add tmp x
	vector_add tmp y
	vector_add v tmp
	= x (+ x 1)
	? x screens
	jl repeat_x
	= y (+ y 1)
	? y screens
	jl repeat_y

	return v
}

function calc_offset
{
	= ox (- pos_x 320)
	? ox 0
	jge do_x_offset2
	= ox 0
:do_x_offset2
	var max_x
	= max_x (- size_w 640)
	? ox max_x
	jle do_y_offset
	= ox max_x

:do_y_offset
	= oy (- pos_y 180)
	? oy 0
	jge do_y_offset2
	= oy 0
:do_y_offset2
	var max_y
	= max_y (- size_h 360)
	? oy max_y
	jle done_offset
	= oy max_y

:done_offset
}

function calc_real_a
{
	= real_a (+ (/ (* PI 3) 2) ship_a)
}

function bullet_collide bx by
{
	; Check coins

	var i

	= i 0
:next_coin
	var coin
	= coin [coins i]
	var cx
	var cy
	var exists
	explode coin cx cy exists
	? exists 0
	je skip_it
	var dx
	var dy
	= dx (- bx cx)
	= dy (- by cy)
	= dx (* dx dx)
	= dy (* dy dy)
	= dx (sqrt (+ dx dy))
	? dx 18
	jge skip_it
	= [coin 2] 0
	= [coins i] coin
	mml_play hit_sfx 1 0
	mml_play shot_coin_sfx 1 0
	call do_explode cx cy 64 251 242 54
	? dead 1
	jne die
	return 1
:die
	= dead 1
	call do_explode pos_x pos_y 64 255 255 255
	return 1
:skip_it
	= i (+ i 1)
	? i NUM_COINS
	jl next_coin

	; Check enemies

	var num_enemies
	= num_enemies (vector_size enemies)

	= i 0
:check_next_enemy
	var e
	= e [enemies i]
	var ex
	var ey
	var exists
	explode e ex ey exists
	? exists 0
	je not_a_hit2
	var dx
	var dy
	= dx (- bx ex)
	= dy (- by ey)
	= dx (* dx dx)
	= dy (* dy dy)
	= dx (sqrt (+ dx dy))
	? dx 14
	jge not_a_hit2
	= [e 2] 0
	= [enemies i] e
	mml_play hit_sfx 1 0
	call do_explode ex ey 48 223 113 38
	return 1
:not_a_hit2
	= i (+ i 1)
	? i num_enemies
	jl check_next_enemy

	return 0
}

function do_explode x y max_r r g b
{
	var e
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
	var s

	? n 10
	jl pad
	= s (string_format "%" n)
	goto done_pad
:pad
	= s (string_format "0%" n)
:done_pad
	return s
}

function draw_time_playing
{
	; Time

	var seconds
	= seconds (floor (/ killed_all_ticks 60))

	var hundredths
	= hundredths (floor (* (- (/ killed_all_ticks 60) seconds) 100))

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

	var _time
	= _time (string_format "%:%.%" ms ss hs)

	var tx
	= tx (- 639 16)
	var w
	= w (font_width small_font _time)
	= tx (- tx w)

	font_draw small_font 223 113 38 255 _time tx 0

	var seconds
	= seconds (floor (/ got_coins_ticks 60))

	var hundredths
	= hundredths (floor (* (- (/ got_coins_ticks 60) seconds) 100))

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

	var _time
	= _time (string_format "%:%.%" ms ss hs)

	var tx
	= tx (- 639 16)
	var w2
	= w2 (font_width small_font _time)
	= tx (- tx w w2 16)

	font_draw small_font 251 242 54 255 _time tx 0
}

function draw_time_won
{
	var tmp
	var tmp2
	= tmp2 killed_all_ticks
	var remain
	= tmp (- got_coins_ticks total_time)
	? tmp 0
	jge still_counting_coins_time
	= remain tmp
	= remain (neg remain)
	= tmp 0

	= tmp2 (- tmp2 remain)

:still_counting_coins_time

	var bak1
	var bak2
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

	var i
	= i 0
	start_primitives
:draw_next_star
	var star
	= star [stars i]
	var star_x
	var star_y
	explode star star_x star_y
	= star_x (- star_x ox)
	= star_y (+ (- star_y oy) 16)
	if (&& (>= star_x 0) (>= star_y 0) (< star_x 640) (< star_y 360)) draw_it
		filled_rectangle 255 255 255 255 255 255 255 255 255 255 255 255 255 255 255 255 star_x star_y 1 1
	:draw_it
	= i (+ i 1)
	? i num_stars
	jl draw_next_star

	end_primitives

	; Draw coins

	image_start coin_img
	
	var coin_x_scale

	var tmp
	= tmp ticks
	= tmp (% tmp 10)
	? tmp 5
	jge pingpong
	= tmp (/ tmp 5)
	= coin_x_scale tmp
	goto done_coin_scale
:pingpong
	= tmp (- tmp 5)
	var tmp2
	= tmp2 (/ (- 5 tmp) 5)
	= coin_x_scale tmp2

:done_coin_scale

	= i 0
:draw_next_coin
	var tmpv
	= tmpv [coins i]
	var coin_x
	var coin_y
	var coin_exists
	explode tmpv coin_x coin_y coin_exists
	? coin_exists 0
	je skip_coin
	= coin_x (- coin_x ox)
	= coin_y (+ (- coin_y oy) 16)
	image_draw_rotated_scaled coin_img 255 255 255 255 16 16 coin_x coin_y 0 coin_x_scale 1 0 0
:skip_coin
	= i (+ i 1)
	? i NUM_COINS
	jl draw_next_coin

	image_end coin_img

	; Draw enemies

	image_start enemy_img

	var e_angle
	= e_angle ticks
	= e_angle (% e_angle 30)
	= e_angle (* (/ e_angle 30) PI 2)

	var num_enemies
	= num_enemies (vector_size enemies)

	= i 0
:draw_next_enemy
	var e
	= e [enemies i]
	var x
	var y
	var exists
	explode e x y exists
	? exists 0
	je done_draw_enemy
	var xx
	var yy
	= xx (- x ox)
	= yy (- y oy)
	if (&& (>= xx -10) (>= yy -10) (< xx 650) (< yy 370)) draw_the_enemy
		= yy (+ yy 16)
		image_draw_rotated_scaled enemy_img 255 255 255 255 12 12 xx yy e_angle 1 1 0 0
	:draw_the_enemy
:done_draw_enemy
	= i (+ i 1)
	? i num_enemies
	jl draw_next_enemy

	image_end enemy_img

	; Draw bullets

	image_start bullet_img

	var num_bullets
	= num_bullets (vector_size bullets)
	? num_bullets 0
	je no_bullets

	= i 0
:draw_next_bullet
	var b
	= b [bullets i]
	var bx
	var by
	explode b bx by
	= bx (- bx ox)
	= by (- by oy)
	if (&& (>= bx -5) (>= by -5) (< bx 645) (< by 365)) draw_the_bullet
		= by (+ by 16)
		image_draw_rotated_scaled bullet_img 255 255 255 255 4 4 bx by 0 1 1 0 0
	:draw_the_bullet
	= i (+ i 1)
	? i num_bullets
	jl draw_next_bullet

:no_bullets

	image_end bullet_img

	; Draw player

	? dead 1
	je skip_draw_player	

	var xx
	= xx (- pos_x ox)
	var yy
	= yy (- pos_y oy)

	= yy (+ yy 16)

	image_draw_rotated_scaled ship_img 255 255 255 255 8 8 xx yy ship_a 1 1 0 0

:skip_draw_player
	
	; Draw explosions

	start_primitives

	var ne
	= ne (vector_size explosions)
	? ne 0
	je no_explosions2

	= i 0
:draw_next_explosion
	var ex
	= ex [explosions i]
	var x
	var y
	var r
	var cr
	var cg
	var cb
	explode ex x y r cr cg cb
	= x (- x ox)
	= y (+ (- y oy) 16)
	circle cr cg cb 255 x y r 1 -1
	= i (+ i 1)
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
	var loops
	= loops collected
	var cx
	= cx 16
:draw_next_coin_icon
	image_stretch_region coin_img 255 255 255 255 0 0 32 32 cx 2 12 12 0 0
	= cx (+ cx 16)
	= loops (- loops 1)
	? loops 0
	jg draw_next_coin_icon

:after_draw_coin_icons

	image_end coin_img

	; Draw total time if won
	? won 0
	je done_draw

	filled_rectangle 16 16 16 128 16 16 16 128 16 16 16 128 16 16 16 128 0 140 640 80

	var seconds
	= seconds (floor (/ total_time 60))

	var hundredths
	= hundredths (floor (* (- (/ total_time 60) seconds) 100))

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

	var _time
	= _time (string_format "%:%.%" ms ss hs)

	var tx
	= tx 320
	var w
	w = (/ (font_width big_font _time) 2)
	= tx (- tx w)
	var ty
	= ty 180
	var h
	= h (/ (font_height big_font) 2)
	= ty (- ty h)

	font_draw big_font 255 255 255 255 _time tx ty

:done_draw
}

function write_config score
{
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
	= score (floor score)
	cfg_set_number cfg "last_score" score
	var success
	= success (cfg_save cfg "com.nooskewl.coinhunt")
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
	= total_ticks (+ got_coins_ticks killed_all_ticks)

:no_win

	? won 1
	jne dont_count_score
	
	var i
	= i 0
:next_score_count
	? total_ticks 0
	jle dont_count_score
	= total_time (+ total_time 1)
	= total_ticks (- total_ticks 1)
	mml_play count_sfx 1 0
	= i (+ i 1)
	? i 20
	jl next_score_count

:dont_count_score

	? won 1
	jne dont_exit
	? total_ticks 0
	jne dont_exit
	= after_win_delay (- after_win_delay 1)
	? after_win_delay 0
	jg dont_exit
	call write_config total_time
	reset "enter_score.boo"

:dont_exit
	= ticks (+ ticks 1)
	= next_fire_ticks (+ next_fire_ticks 1)

	? got_coins 1
	je check_killed_all
	= got_coins_ticks (+ got_coins_ticks 1)

:check_killed_all
	? killed_all 1
	je after_killed_all_check
	= killed_all_ticks (+ killed_all_ticks 1)

:after_killed_all_check
	? dead 0
	je no_tick
	= dead_time (+ dead_time 1)
	? dead_time 300
	jl no_tick
	call start_game

:no_tick

	? dead 1
	je skip_player_movement
	
	= game_ticks (+ game_ticks 1)

	= ship_vel (+ ship_vel ship_acc)
	? ship_vel 8
	jle done_clamp
	= ship_vel 8

:done_clamp

	? ship_acc 0
	jne done_vel_damp
	= ship_vel (- ship_vel 0.1)
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
	? next_fire_ticks 7
	jl do_thrust
	= next_fire_ticks 0
	
	var b
	var bx
	var by
	var ba
	call calc_real_a
	= ba real_a
	= bx pos_x
	= by pos_y
	var f
	= f (* (cos ba) 8)
	= bx (+ bx f)
	= f (* (sin ba) 8)
	= by (+ by f)
	vector_add b bx
	vector_add b by
	vector_add b ba
	vector_add bullets b
	mml_play shoot_sfx 1 0

	if (>= (vector_size bullets) 7) erase_one
		vector_erase bullets 0
	:erase_one

	; Do thrust

:do_thrust
	= ship_acc 0
	? joy_a 0
	je done_thrust
	= ship_acc 0.1
:done_thrust

	call calc_real_a

	var f

	= f (* (cos real_a) ship_vel)
	= pos_x (+ pos_x f)

	= f (* (sin real_a) ship_vel)
	= pos_y (+ pos_y f)

	? pos_x 32
	jge x_ok1
	= pos_x 32
:x_ok1
	var max_x
	= max_x (- size_w 1 32)
	? pos_x max_x
	jle x_ok2
	= pos_x max_x
:x_ok2

	? pos_y 32
	jge y_ok1
	= pos_y 32
:y_ok1
	var max_y
	= max_y (- size_h 1 32)
	? pos_y max_y
	jle y_ok2
	= pos_y max_y
:y_ok2

	var a_inc
	= a_inc 0

	? joy_l 0
	je no_rot_l
	= a_inc (- a_inc 0.1)
:no_rot_l
	? joy_r 0
	je no_rot_r
	= a_inc (+ a_inc 0.1)
:no_rot_r

	= ship_a (+ ship_a a_inc)

	; Check if colliding with any coins

	var i
	= i 0
:check_next_coin
	var tmp
	= tmp [coins i]
	var coin_x
	var coin_y
	var coin_exists
	explode tmp coin_x coin_y coin_exists
	? coin_exists 0
	je not_a_hit
	var xx
	var yy
	= xx (- pos_x coin_x)
	= yy (- pos_y coin_y)
	= xx (* xx xx)
	= yy (* yy yy)
	= xx (+ xx yy)
	var dist
	= dist (sqrt xx)
	? dist 18
	jge not_a_hit
	= [tmp 2] 0
	= [coins i] tmp
	mml_play coin_sfx 1 0
	= collected (+ collected 1)
	? collected NUM_COINS
	jl not_a_hit
	= got_coins 1
:not_a_hit
	= i (+ i 1)
	? i NUM_COINS
	jl check_next_coin

	; Check for player-enemy collisions

	var ne
	= ne (vector_size enemies)

	= i 0
:check_next_enemy_with_player
	var e
	= e [enemies i]
	var ex
	var ey
	var exists
	explode e ex ey exists
	? exists 0
	je player_didnt_hit_enemy
	var dx
	var dy
	= dx (- ex pos_x)
	= dy (- ey pos_y)
	= dx (* dx dx)
	= dy (* dy dy)
	= dx (sqrt (+ dx dy))
	? dx 14
	jge player_didnt_hit_enemy
	= dead 1
	call do_explode ex ey 48 223 113 38
	call do_explode pos_x pos_y 64 255 255 255
	mml_play hit_sfx 1 0
	= [e 2] 0
	= [enemies i] e
:player_didnt_hit_enemy
	= i (+ i 1)
	? i ne
	jl check_next_enemy_with_player

:skip_player_movement

	; Move the enemies

	var num_enemies
	= num_enemies (vector_size enemies)

	= i 0
:move_next_enemy
	var e
	= e [enemies i]
	var ex
	var ey
	var exists
	var dest_x
	var dest_y
	explode e ex ey exists dest_x dest_y
	? ex dest_x
	je move_y
	jl move_right
:move_left
	= ex (- ex 1)
	goto done_x
:move_right
	= ex (+ ex 1)
:done_x
	var diff
	= diff (abs (- ex dest_x))
	? diff 2
	jl set_next_dest
	goto done_move
:move_y
	? ey dest_y
	jl move_down
:move_up
	= ey (- ey 1)
	goto done_y
:move_down
	= ey (+ ey 1)
:done_y
	var diff
	= diff (abs (- ey dest_y))
	? diff 2
	jl set_next_dest
	goto done_move
:set_next_dest
	var next_dest
	call_result next_dest get_next_enemy_pos ex ey
	explode next_dest dest_x dest_y
:done_move
	= [e 0] ex
	= [e 1] ey
	= [e 3] dest_x
	= [e 4] dest_y
	= [enemies i] e
	= i (+ i 1)
	? i num_enemies
	jl move_next_enemy

	; Move the bullets

	var num_bullets
	= num_bullets (vector_size bullets)
	? num_bullets 0
	je no_bullets2

	= i 0
:move_next_bullet
	var was_erased
	= was_erased 0
	var bx
	var by
	var ba
	= bx [b 0]
	= by [b 1]
	= ba [b 2]
	var c
	= c (cos ba)
	var s
	= s (sin ba)
	var speed
	= speed 16
	var step
	= step 1
:next_step
	= bx (+ bx (* c step))
	= by (+ by (* s step))
	var hit_something
	call_result hit_something bullet_collide bx by
	? hit_something 0
	je hit_nothing
	goto erase_it
:hit_nothing
	= speed (- speed step)
	? speed 0
	jg next_step
:done_steps
	? bx 0
	jl erase_it
	? by 0
	jl erase_it
	var max_
	= max_ (- size_w 1)
	? bx max_
	jge erase_it
	= max_ (- size_h 1)
	? by max_
	jge erase_it
	goto in_zone
:erase_it
	vector_erase bullets i
	= was_erased 1
	= i (- i 1)
	= num_bullets (- num_bullets 1)
:in_zone
	? was_erased 1
	je no_update
	= [b 0] bx
	= [b 1] by
	= [bullets i] b
:no_update
	= i (+ i 1)
	? i num_bullets
	jl move_next_bullet

:no_bullets2

	; Check if all enemies are dead
	var num_enemies
	= num_enemies (vector_size enemies)
	var all_dead
	= all_dead 1
	var i
	= i 0
:check_if_next_enemy_is_dead
	var e
	= e [enemies i]
	var exists
	= exists [e 2]
	? exists 1
	jne its_dead
	= all_dead 0
:its_dead
	= i (+ i 1)
	? i num_enemies
	jl check_if_next_enemy_is_dead

	? all_dead 1
	jne update_explosions
	= killed_all 1

	; Update explosions

:update_explosions
	var ne
	= ne (vector_size explosions)
	? ne 0
	je no_explosions

	= i 0
:update_next_explosion
	var exv
	= exv [explosions i]
	var r
	var max_r
	= r [exv 2]
	= max_r [exv 3]
	= r (+ r 5)
	? r max_r
	jle its_fine
	vector_erase explosions i
	= i (- i 1)
	= ne (- ne 1)
	goto finished_explosion_update
:its_fine
	= [exv 2] r
	= [explosions i] exv
:finished_explosion_update
	= i (+ i 1)
	? i ne
	jl update_next_explosion

:no_explosions
}
