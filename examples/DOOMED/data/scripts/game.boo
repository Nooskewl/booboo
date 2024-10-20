resize 240 135

number score
= score 0
number life
= life 100
number exit_count
= exit_count 180

number font
font_load font "font.ttf" 16 1

number eicon
image_load eicon "stand_s.png"
number hicon
image_load hicon "health.png"

number SPEED
= SPEED 0.01

number BULLET_SPEED
= BULLET_SPEED 0.2

number tree
image_load tree "tree.png"

number projectile
image_load projectile "projectile.png"

number hand_normal
image_load hand_normal "hand_normal.png"
number hand_fire
image_load hand_fire "hand_fire.png"
number fired
= fired 0

vector billboards

number num_billboards
= num_billboards 100

number hit
mml_create hit "@PO0 = { 0 1000 }\nA @TYPE1 @PO0 a8 @PO0"
number fire_sfx
mml_create fire_sfx "@PO0 = { 0 100 }\nA @TYPE0 o1 y16 @PO0 a8 @PO0"

number i
for i 0 (< i num_billboards) 1 loop
	number b
	number x
	rand x 0 1000
	- x 500
	/ x 500
	* x 10
	number z
	rand z 0 1000
	- z 500
	/ z 500
	* z 10
	billboard_create b tree x 0.4 z 0.55 0.8
	vector_add billboards b
:loop

vector enemies

call spawn_enemy

number SPAWN_TIME next_spawn
= SPAWN_TIME 100
= next_spawn 0

number texture
image_load texture "texture.png"

vector positions
vector_add positions -1000 -0.01 -1000
vector_add positions -1000 -0.01 1000
vector_add positions 1000 -0.01 -1000
vector_add positions -1000 -0.01 1000
vector_add positions 1000 -0.01 -1000
vector_add positions 1000 -0.01 1000
vector texcoords
vector_add texcoords 0 0
vector_add texcoords 0 10000
vector_add texcoords 10000 0
vector_add texcoords 0 10000
vector_add texcoords 10000 0
vector_add texcoords 10000 10000
vector faces
vector_add faces 0 1 2
vector_add faces 3 4 5
vector colours
	
number angle
= angle 0
number anglex dax
= anglex 0

number x y z
= x 0
= y 0
= z 0

mouse_set_relative TRUE

vector bullets

number fire_down
= fire_down FALSE

function spawn_enemy
{
	number x
	rand x 0 1000
	- x 500
	/ x 500
	* x 10

	number z
	rand z 0 1000
	- z 500
	/ z 500
	* z 10

	number sprite
	sprite_load sprite "pleasant"

	number pleasant
	billboard_from_sprite pleasant sprite x 0.25 z 0 0 32

	map enemy
	map_set enemy "sprite" sprite
	map_set enemy "billboard" pleasant
	map_set enemy "x" x
	map_set enemy "y" 0
	map_set enemy "z" z
	map_set enemy "dx" 0
	map_set enemy "dy" 0
	map_set enemy "dz" 0
	map_set enemy "dead" FALSE
	map_set enemy "dead_count" 0
	map_set enemy "attacking" FALSE
	map_set enemy "attack_count" 0

	vector_add enemies enemy

	number sz
	vector_size enemies sz
	- sz 1

	call set_dest [enemies sz]
}


function set_dest e
{
	number r
	rand r 0 1000
	- r 500
	/ r 500
	* r 10
	= [e "dx"] r
	rand r 0 1000
	- r 500
	/ r 500
	* r 10
	= [e "dz"] r
}

function move_enemy e
{
	if (== TRUE [e "dead"]) done2
		return
	:done2
	vector v1 v2 diff tmp
	vector_init v1 [e "x"] [e "y"] [e "z"]
	vector_init v2 [e "dx"] [e "dy"] [e "dz"]
	= diff (sub v2 v1)
	if (< (length diff) 0.1) done
		call set_dest e
		return
	:done
	= tmp (mul (normalize diff) SPEED)
	+ [e "x"] [tmp 0]
	+ [e "z"] [tmp 2]
	billboard_translate [e "billboard"] [tmp 0] 0 [tmp 2]
}

function spawn_bullet x y z dx dy dz friendly
{
	number billboard
	billboard_create billboard projectile x y z 0.25 0.25

	map bullet
	map_set bullet "x" x
	map_set bullet "y" y
	map_set bullet "z" z
	map_set bullet "dx" dx
	map_set bullet "dy" dy
	map_set bullet "dz" dz
	map_set bullet "friendly" friendly
	map_set bullet "billboard" billboard
	vector_add bullets bullet
}

function normalize_angle a
{
	number tmp
	? a 0
	jge done_neg
	= tmp a
	neg tmp
	/ tmp (* PI 2)
	floor tmp
	+ tmp 1
	+ a (* tmp (* PI 2))
:done_neg
	? a (* PI 2)
	jl done_pos
	= tmp a
	/ tmp (* PI 2)
	floor tmp
	- a (* tmp (* PI 2))
:done_pos
	return a
}

function draw
{
	clear 100 100 255
	
	set_3d

	identity_3d
	rotate_3d anglex 1 0 0
	rotate_3d angle 0 1 0
	translate_3d x (- y 0.1) z

	draw_3d_textured texture positions faces colours texcoords 2

	number i
	for i 0 (< i num_billboards) 1 loop
		billboard_draw [billboards i] 255 255 255 255
	:loop

	number sz
	vector_size enemies sz

	for i 0 (< i sz) 1 loop2
		if (== TRUE [[enemies i] "dead"]) is_dead (== TRUE [[enemies i] "attacking"]) do_attacking not_dead
			sprite_set_animation_lazy [[enemies i] "sprite"] "dead"
		:is_dead
			sprite_set_animation_lazy [[enemies i] "sprite"] "fire"
		:do_attacking
			vector tmp1 tmp2 tmp3 tmp4
			number angle2

			vector_init tmp1 [[enemies i] "x"] [[enemies i] "y"] [[enemies i] "z"]
			vector_init tmp2 [[enemies i] "dx"] [[enemies i] "dy"] [[enemies i] "dz"]
			vector_init tmp3 0 0 1
			= tmp4 (sub tmp2 tmp1)
			
			= angle2 (angle tmp4 tmp3)

			vector cross plane
			= cross (cross tmp4 tmp3)
			vector_init plane 0 1 0
			if (< (dot plane cross) 0) negate
				neg angle2
			:negate

			call_result angle normalize_angle angle

			number a
			= a (- angle2 angle)

			call_result a normalize_angle a

			if (|| (< a (* PI 0.25)) (> a (* PI 1.75))) front (< a (* PI 0.75)) left (< a (* PI 1.25)) back right
				sprite_set_animation_lazy [[enemies i] "sprite"] "walk_s"
			:front
				sprite_set_animation_lazy [[enemies i] "sprite"] "walk_w"
			:left
				sprite_set_animation_lazy [[enemies i] "sprite"] "walk_n"
			:back
				sprite_set_animation_lazy [[enemies i] "sprite"] "walk_e"
			:right
		:not_dead
		billboard_draw [[enemies i] "billboard"] 255 255 255 255
	:loop2

	vector_size bullets sz
	for i 0 (< i sz) 1 next_bullet
		billboard_draw [[bullets i] "billboard"] 255 255 255 255
	:next_bullet

	set_2d

	image_draw hicon 255 255 255 255 20 10 0 0

	number r g b
	if (>= life 50) green (>= life 25) yellow red
		= r 0
		= g 255
		= b 0
	:green
		= r 255
		= g 255
		= b 0
	:yellow
		= r 255
		= g 0
		= b 0
	:red

	string healths
	string_format healths "%" life
	number xx
	= xx 20
	+ xx 16
	+ xx 10
	font_draw font 0 0 0 255 healths (- xx 1) 10
	font_draw font 0 0 0 255 healths (+ xx 1) 10
	font_draw font 0 0 0 255 healths xx 9
	font_draw font 0 0 0 255 healths xx 11
	font_draw font r g b 255 healths xx 10

	string scores
	string_format scores "%" score

	number xx
	= xx 240
	- xx 20
	number w
	font_width font w scores
	- xx w

	font_draw font 0 0 0 255 scores (- xx 1) 10
	font_draw font 0 0 0 255 scores (+ xx 1) 10
	font_draw font 0 0 0 255 scores xx 9
	font_draw font 0 0 0 255 scores xx 11
	font_draw font 255 255 255 255 scores xx 10

	- xx 10
	- xx 16

	image_draw eicon 255 255 255 255 xx 10 0 0

	number hand
	= hand hand_normal
	if (> fired 0) draw_hand
		= hand hand_fire
	:draw_hand
	image_draw hand 255 255 255 255 32 0 0 0
}

function eabs x
{
	if (< x 0) switch
		neg x
	:switch
	return x
}

function run
{
	include "poll_joystick.inc"

	number b1 b2 b3 wheel
	mouse_get_buttons b1 b2 b3 wheel

	= joy_a (|| (!= b1 0) (>= joy_y3 0.5))

	number dx dy
	mouse_get_delta dx dy
	if (!= 0 dx) set_x2
		= joy_x2 (/ dx 10)
	:set_x2
	if (!= 0 dy) set_y2
		= joy_y2 (/ dy 10)
	:set_y2

	number w a s d
	key_get w KEY_w
	key_get a KEY_a
	key_get s KEY_s
	key_get d KEY_d
	if (!= 0 w) set_forward
		= joy_y1 -1
	:set_forward
	if (!= 0 s) set_backward
		= joy_y1 1
	:set_backward
	if (!= 0 a) set_strafe_left
		= joy_x1 -1
	:set_strafe_left
	if (!= 0 d) set_strafe_right
		= joy_x1 1
	:set_strafe_right

	if (< (eabs joy_x1) 0.1) clip_x1
		= joy_x1 0
	:clip_x1
	if (< (eabs joy_x2) 0.1) clip_x2
		= joy_x2 0
	:clip_x2
	if (< (eabs joy_y1) 0.1) clip_y1
		= joy_y1 0
	:clip_y1
	if (< (eabs joy_y2) 0.1) clip_y2
		= joy_y2 0
	:clip_y2

	neg joy_x1
	neg joy_y1

	/ joy_x1 10
	/ joy_x2 10
	/ joy_y1 10
	/ joy_y2 10

	+ angle joy_x2
	+ anglex joy_y2

	if (< anglex (* -1 (/ PI 2))) cliplow (> anglex (/ PI 2)) cliphigh
		= anglex (* -1 (/ PI 2))
	:cliplow
		= anglex (/ PI 2)
	:cliphigh

	number xi yi zi
	= xi (+ angle (/ PI 2))
	cos xi
	= xi (* xi joy_y1)
	= yi 0
	= zi (+ angle (/ PI 2))
	sin zi
	= zi (* zi joy_y1)

	number xi2 yi2 zi2
	= xi2 angle
	cos xi2
	= xi2 (* xi2 joy_x1)
	= yi2 0
	= zi2 angle
	sin zi2
	= zi2 (* zi2 joy_x1)

	+ x xi xi2
	+ y yi yi2
	+ z zi zi2

	if (< x -10) incx
		= x -10
	:incx
	if (> x 10) decx
		= x 10
	:decx
	if (< z -10) incz
		= z -10
	:incz
	if (> z 10) decz
		= z 10
	:decz

	number sz i
	vector_size enemies sz
	for i 0 (< i sz) 1 loop
		vector v1 v2
		vector_init v1 [[enemies i] "x"] [[enemies i] "y"] [[enemies i] "z"]
		vector_init v2 (* x -1) 0 (* z -1)
		number dist
		= dist (length (sub v1 v2))
		number close
		if (< dist 2.5) is_close not_close
			= close TRUE
		:is_close
			= close FALSE
		:not_close
		if (== TRUE [[enemies i] "attacking"]) attacking not_attacking
			if (== FALSE close) stop_attacking
				= [[enemies i] "attacking"] FALSE
				call set_dest [enemies i]
			:stop_attacking
		:attacking
			if (== TRUE close) start_attacking move
				= [[enemies i] "attacking"] TRUE
				= [[enemies i] "attack_count"] 0
			:start_attacking
				call move_enemy [enemies i]
			:move
		:not_attacking
	:loop

	+ next_spawn 1
	if (>= next_spawn SPAWN_TIME) spawn
		call spawn_enemy
		= next_spawn 0
	:spawn

	vector_size bullets sz
	? sz 0
	jle done_bullets
	= i 0
:again
	vector v
	vector_init v [[bullets i] "dx"] [[bullets i] "dy"] [[bullets i] "dz"]
	= v (mul v BULLET_SPEED)
	+ [[bullets i] "x"] [v 0]
	+ [[bullets i] "y"] [v 1]
	+ [[bullets i] "z"] [v 2]
	billboard_translate [[bullets i] "billboard"] [v 0] [v 1] [v 2]
	number del
	= del FALSE
	if (|| (< [[bullets i] "x"] -10) (> [[bullets i] "x"] 10)) delete_it
		= del TRUE
	:delete_it
	if (|| (< [[bullets i] "z"] -10) (> [[bullets i] "z"] 10)) delete_it2
		= del TRUE
	:delete_it2
	? del TRUE
	jne bottom
	vector_erase bullets i
	- sz 1
	goto next
:bottom
	+ i 1
:next
	? i sz
	jl again
:done_bullets

	if (&& (== fire_down FALSE) (== joy_a 1)) fire
		number a
		= a (+ angle (* PI 1.5))
		number c s c2
		= c a
		cos c
		= s a
		sin s
		= c2 anglex
		neg c2
		sin c2
		vector_init v c c2 s
		= v (normalize v)
		call spawn_bullet (* x -1) 0.2 (* z -1) [v 0] [v 1] [v 2] TRUE
		mml_play fire_sfx 1 0
		= fired 30
	:fire

	= fire_down (== joy_a 1)

	number ne nb j
	vector_size enemies ne
	vector_size bullets nb

	= i 0
:next_e
	= j 0
:next_b
	? i ne
	jge no_bullets
	? j nb
	jge no_bullets
	if (== FALSE [[bullets j] "friendly"]) next_b2
		goto no_collide
	:next_b2
	number col
	cd_sphere_sphere col [[enemies i] "x"] [[enemies i] "y"] [[enemies i] "z"] 0.25 [[bullets j] "x"] [[bullets j] "y"] [[bullets j] "z"] 0.25
	? col 0
	je no_collide
	= [[enemies i] "dead"] TRUE
	= [[enemies i] "dead_count"] 30
	- nb 1
	vector_erase bullets j
	mml_play hit 1 0
	+ score 100
	goto next_e2
:no_collide
	+ j 1
	? j nb
	jl next_b
:next_e2
	+ i 1
	? i ne
	jl next_e
:no_bullets

	= i 0
:next_dead
	? i ne
	jge killed_enemies
	? [[enemies i] "dead"] FALSE
	je not_dead
	- [[enemies i] "dead_count"] 1
	? [[enemies i] "dead_count"] 0
	jge not_dead
	vector_erase enemies i
	- ne 1
	goto next_dead
:not_dead
	+ i 1
	? i ne
	jl next_dead
:killed_enemies

	for i 0 (< i ne) 1 enemy_attack
		if (== TRUE [[enemies i] "attacking"]) ok
			- [[enemies i] "attack_count"] 1
			if (<= [[enemies i] "attack_count"] 0) enemy_fire
				= [[enemies i] "attack_count"] 60
				call do_enemy_attack [enemies i]
			:enemy_fire
		:ok
	:enemy_attack

	- fired 1

	; check if bullets hit player
	? life 0
	jle done_checking_player
	= i 0
:check_next_bullet
	? i nb
	jge done_checking_player
	? [[bullets i] "friendly"] TRUE
	je another
	vector v1 v2
	vector_init v1 [[bullets i] "x"] [[bullets i] "y"] [[bullets i] "z"]
	vector_init v2 (* x -1) 0.2 (* z -1)
	number col
	cd_sphere_sphere col [v1 0] [v1 1] [v1 2] 0.125 [v2 0] [v2 1] [v2 2] 0.5
	if (== TRUE col) hit_player
		- life 1
		? life 0
		jle done_checking_player
		if (<= life 0) player_dead player_not_dead
			screen_shake 3 2500
			rumble 2500
			number a
			number da
			= da (/ PI 4)
			= a (- da anglex)
			/ a 180
			= dax a
		:player_dead
			screen_shake 1.5 500
			rumble 500
		:player_not_dead
		mml_play hit 1 0
		vector_erase bullets i
		- nb 1
		goto check_next_bullet
	:hit_player
:another
	+ i 1
	? i nb
	jl check_next_bullet
:done_checking_player

	? life 0
	jg no_ending
	+ angle 0.025
	+ anglex dax
	- y 0.01
	- exit_count 1
	? exit_count 0
	jge no_ending
	resize 640 360
	call write_config
	reset "enter_score.boo"

:no_ending
}

function do_enemy_attack e
{
	vector v1 v2 diff
	vector_init v1 [e "x"] [e "y"] [e "z"]
	vector_init v2 (* x -1) 0 (* z -1)
	= diff (normalize (sub v2 v1))
	call spawn_bullet [e "x"] 0.2 [e "z"] [diff 0] [diff 1] [diff 2] 0
	mml_play fire_sfx 1 0
}

function write_config
{
	number cfg
	cfg_load cfg "com.illnorth.doomed"
	number exists
	cfg_exists cfg exists "last_score"
	? exists 0
	jne dont_set_defaults
	cfg_set_number cfg "score0" 5000
	cfg_set_number cfg "score1" 4000
	cfg_set_number cfg "score2" 3000
	cfg_set_number cfg "score3" 2000
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
	cfg_save cfg success "com.illnorth.doomed"
}

