function write_config
{
	number cfg
	cfg_load cfg "com.nooskewl.rainrunner"
	number exists
	cfg_exists cfg exists "last_score"
	? exists 0
	jne dont_set_defaults
	cfg_set_number cfg "score0" 2000
	cfg_set_number cfg "score1" 1500
	cfg_set_number cfg "score2" 1200
	cfg_set_number cfg "score3" 1100
	cfg_set_number cfg "score4" 1000
	cfg_set_string cfg "name0" "ILL"
	cfg_set_string cfg "name1" "ILL"
	cfg_set_string cfg "name2" "ILL"
	cfg_set_string cfg "name3" "ILL"
	cfg_set_string cfg "name4" "ILL"
:dont_set_defaults
	floor score
	cfg_set_number cfg "last_score" score
	number success
	cfg_save cfg success "com.nooskewl.rainrunner"
}

function run
{
	? won 1
	jne check_if_dead_exit
	- exit_count 1
	? exit_count 0
	jg check_if_dead_exit
	call write_config
	reset "win.boo"

:check_if_dead_exit
	? life 0
	jg check_all_numerals_collected
	- exit_count 1
	? exit_count 0
	jg check_all_numerals_collected
	call write_config
	reset "enter_score.boo"

:check_all_numerals_collected
	; check if all numerals collected

	number collected
	= collected 0
	number i
	= i 0
:check_if_next_collected
	vector numeral
	vector_get numerals numeral i
	number gotten
	vector_get numeral gotten 0
	? gotten 0
	je next_numeral_collected_loop
	+ collected 1
:next_numeral_collected_loop
	+ i 1
	? i 10
	jl check_if_next_collected
	? collected 10
	jne do_lr

	= moving 0
	? won 0
	jne after_player_input
	= won 1
	= exit_count 180
	+ score 1000
	mml_play win_sfx 1 0
	goto after_player_input

:do_lr
	include "poll_joystick.inc"

	? joy_back 1
	jne dont_bail
	reset "main.boo"

:dont_bail

	; l/r handling

	? life 0
	jg do_input
	= moving 0
	jle after_player_input

:do_input
	= moving 0
	number x_inc
	= x_inc 0
	? joy_l 0
	je test_joy_r
	= dir -1
	= moving 1
	= x_inc move_speed
	neg x_inc
	goto done_x_inc
:test_joy_r
	? joy_r 0
	je done_x_inc
	= dir 1
	= moving 1
	= x_inc move_speed
:done_x_inc
	? jumping 0
	jne lr_jumping
	goto done_lr
:lr_jumping
	* x_inc 0.75
:done_lr
	+ px x_inc
	? px 20
	jge check_right
	= px 20
	goto done_x_bounds
:check_right
	? px 619
	jle done_x_bounds
	= px 619
:done_x_bounds
	
	; jumping

	? joy_a old_joy_a
	je no_jump
	= old_joy_a joy_a
	? joy_a 0
	je no_jump
	? jumps 2
	jge no_jump
	= jumping 1
	+ jumps 1
	= angle 0
	mml_play jump_sfx 1 0
	+ vy jump_accel
	? vy jump_accel
	jge no_accel_fix
	= vy jump_accel
:no_accel_fix
	? jumps 1
	jne no_jump
	- py 30 ; sprite height difference
:no_jump

	? jumping 0
	je no_rotate
	+ angle 0.25
:no_rotate
:after_player_input

	; vertical movement
	? vy 0
	je y_ok
	+ vy jump_accel_damp
	? vy 0
	jle move_vertical
	= vy 0
:move_vertical
	+ py vy
	? py 25
	jge y_ok
	= py 25
:y_ok

	; gravity
	+ py gravity
	? jumping 0
	je gravity_on_ground
	number g
	= g ground
	- g 30
	? py g
	jle done_gravity
	= jumping 0
	= jumps 0
	= angle 0
	= py ground
	= vy 0
	goto done_gravity
:gravity_on_ground
	? py ground
	jle done_gravity
	= py ground
:done_gravity

	; scoring
	? jumping 0
	je no_score
	number p
	= p ground
	- p py
	/ p ground
	* p p
	+ score p
:no_score

	number num_drops
	vector_size drops num_drops

	? num_drops 0
	je no_drops_to_move

	; move drops
	number i
	= i 0
:move_next_drop
	vector drop
	vector_get drops drop i
	number drop_y
	vector_get drop drop_y 1
	+ drop_y drop_gravity
	vector_set drop 1 drop_y
	vector_set drops i drop
	? drop_y ground
	jl drop_y_ok
	vector_erase drops i
	mml_play drop_ground_sfx 1 0
	- num_drops 1
	goto check_if_drops_to_move
:drop_y_ok
	+ i 1
:check_if_drops_to_move
	? i num_drops
	jl move_next_drop
:no_drops_to_move

	; add drops
	- next_drop 1
	? next_drop 0
	jne no_drop
	vector drop
	number drop_x
	rand drop_x 20 619
	vector_add drop drop_x
	vector_add drop drop_start_y
	vector_add drops drop
	mml_play drip_sfx 1 0
	= next_drop drop_interval
:no_drop

	- speed_up 1
	? speed_up 0
	jne no_speed_up
	= speed_up 1800
	? drop_interval 20
	jle no_speed_up
	- drop_interval 5
:no_speed_up

	; check for collisions

	number num_drops
	vector_size drops num_drops

	? num_drops 0
	jle no_drops_for_cd

	number i
	= i 0
:check_for_next_hit
	vector drop
	vector_get drops drop i
	number drop_x
	number drop_y
	vector_get drop drop_x 0
	vector_get drop drop_y 1
	number drop_x1
	number drop_y1
	number drop_x2
	number drop_y2
	number drop_w
	number drop_h
	image_size drop_img drop_w drop_h
	* drop_w 0.75
	* drop_h 0.75
	/ drop_w 2
	/ drop_h 2
	= drop_x1 drop_x
	- drop_x1 drop_w
	= drop_x2 drop_x
	+ drop_x2 drop_w
	= drop_y1 drop_y
	- drop_y1 drop_h
	= drop_y2 drop_y
	+ drop_y2 drop_h
	number p_x1
	number p_y1
	number p_x2
	number p_y2
	number w
	number h
	? jumping 0
	je calc_bounds_no_jump
	image_size jump_img w h
	goto do_collision_check
:calc_bounds_no_jump
	image_size idle_img w h
:do_collision_check
	number half
	= half h
	/ half 2
	number player_y
	= player_y py
	- player_y half
	* w 0.75
	* h 0.75
	/ w 2
	/ h 2
	= p_x1 px
	- p_x1 w
	= p_x2 px
	+ p_x2 w
	= p_y1 player_y
	- p_y1 h
	= p_y2 player_y
	+ p_y2 h
	? have_umbrella 1
	number tmp
	number half_umbrella_h
	image_size umbrella_img tmp half_umbrella_h
	/ half_umbrella_h 2
	- p_y1 half_umbrella_h
	jne check_drop_collisions
:check_drop_collisions
	? drop_x1 p_x2
	jge after_collisions
	? drop_x2 p_x1
	jl after_collisions
	? drop_y1 p_y2
	jge after_collisions
	? drop_y2 p_y1
	jl after_collisions
	; it's colliding!
	? have_umbrella 1
	jne do_damage
	= have_umbrella 0
	mml_play hit_umbrella_sfx 1 0
	goto solved_collision
:do_damage
	- life 1
	mml_play hit_sfx 1 0
	? life 0
	jne solved_collision
	mml_play gameover_sfx 1 0
	= exit_count 180
:solved_collision
	- num_drops 1
	vector_erase drops i
	goto next_cd
:after_collisions
	+ i 1
:next_cd
	? i num_drops
	jl check_for_next_hit
:no_drops_for_cd

	? have_umbrella 1
	jne do_umbrella_gen
	= next_umbrella 1800
	goto after_umbrella_item

:do_umbrella_gen
	; making umbrellas appear and disappear
	- next_umbrella 1
	? next_umbrella -300
	jl erase_umbrella
	? next_umbrella 0
	je gen_umbrella
	jl keep_umbrella
	goto after_umbrella_item
:gen_umbrella
	rand umbrella_x 40 599
	rand umbrella_y 40 319
:keep_umbrella
	= umbrella 1
	goto after_umbrella_item
:erase_umbrella
	= umbrella 0
	= next_umbrella 1800
:after_umbrella_item

	? umbrella 0
	je no_umbrella_to_pick_up

	; check if you pick up an umbrella
	number u_x1
	number u_y1
	number u_x2
	number u_y2
	number p_x1
	number p_y1
	number p_x2
	number p_y2
	number pw
	number ph
	number uw
	number uh
	number pimg
	? jumping 1
	jne not_jumping
	= pimg jump_img
	goto get_sizes
:not_jumping
	= pimg idle_img
:get_sizes
	image_size pimg pw ph
	image_size umbrella_img uw uh
	* pw 0.75
	* ph 0.75
	* uw 0.75
	* uh 0.75
	/ pw 2
	/ ph 2
	/ uw 2
	/ uh 2
	= u_x1 umbrella_x
	- u_x1 uw
	= u_x2 umbrella_x
	+ u_x2 uw
	= u_y1 umbrella_y
	- u_y1 uh
	= u_y2 umbrella_y
	+ u_y2 uh
	= p_x1 px
	- p_x1 pw
	= p_x2 px
	+ p_x2 pw
	= p_y1 py
	- p_y1 ph
	= p_y2 py
	+ p_y2 ph
	? u_x1 p_x2
	jg not_touching_umbrella
	? u_x2 p_x1
	jle not_touching_umbrella
	? u_y1 p_y2
	jg not_touching_umbrella
	? u_y2 p_y1
	jle not_touching_umbrella
	; touching umbrella
	= have_umbrella 1
	+ score 100
	mml_play collect_sfx 1 0
	= next_umbrella 1800
	= umbrella 0
:not_touching_umbrella
:no_umbrella_to_pick_up

	; numeral collecting

	number i
	= i 0
:next_numeral_cd
	vector numeral
	vector_get numerals numeral i
	number gotten
	number nx
	number ny
	vector_get numeral gotten 0
	vector_get numeral nx 1
	vector_get numeral ny 2
	? gotten 1
	je dont_process_numeral_cd
	number instantiate_time
	= instantiate_time 15000
	number mul
	= mul i
	+ mul 1
	* instantiate_time mul
	number t
	get_ticks t
	- t start_ticks
	? t instantiate_time
	jl dont_process_numeral_cd
	number nw
	number nh
	number nimg
	vector_get numeral_imgs nimg i
	image_size nimg nw nh
	* nw 0.75
	* nh 0.75
	/ nw 2
	/ nh 2
	number pimg
	number pw
	number ph
	number player_y
	? jumping 0
	jne numeral_cd_jumping
	= pimg idle_img
	goto numeral_cd_get_player_size
:numeral_cd_jumping
	= pimg jump_img
:numeral_cd_get_player_size
	image_size pimg pw ph
	= player_y py
	/ ph 2
	- player_y ph
	* pw 0.75
	* ph 0.75
	/ pw 2
	number n_x1
	number n_y1
	number n_x2
	number n_y2
	number p_x1
	number p_y1
	number p_x2
	number p_y2
	= n_x1 nx
	- n_x1 nw
	= n_x2 nx
	+ n_x2 nw
	= n_y1 ny
	- n_y1 nh
	= n_y2 ny
	+ n_y2 nh
	= p_x1 px
	- p_x1 pw
	= p_x2 px
	+ p_x2 pw
	= p_y1 player_y
	- p_y1 ph
	= p_y2 player_y
	+ p_y2 ph
	? n_x1 p_x2
	jge dont_process_numeral_cd
	? n_x2 p_x1
	jl dont_process_numeral_cd
	? n_y1 p_y2
	jge dont_process_numeral_cd
	? n_y2 p_y1
	jl dont_process_numeral_cd
	vector_set numeral 0 1
	vector_set numerals i numeral
	+ score 100
	mml_play collect_sfx 1 0
:dont_process_numeral_cd
	+ i 1
	? i 10
	jl next_numeral_cd
}
