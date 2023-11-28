number music
mml_load music "music/town.mml"
mml_play music 0.5 1

number W H
= W 240
= H 135
resize W H

number sprite
sprite_load sprite "pleasant"

number tilemap
tilemap_load tilemap "map.wm2"

number TILE_SIZE px py dir_x dir_y moving move_count MOVE_TIME
= TILE_SIZE 16
= px 19
= py 11
= moving 0
= MOVE_TIME 20
= dir_x 0
= dir_y 1

function suffix_from_dirs dx dy
{
	if (&& (== dx 0) (== dy 1)) south (&& (== dx 0) (== dy -1)) north (&& (== dx 1) (== dy 0)) east west
	return "_s"
:south
	return "_n"
:north
	return "_e"
:east
	return "_w"
:west
}

function draw
{
	number ox oy sx sy

	= ox (- (+ (/ TILE_SIZE 2) (* px TILE_SIZE)) (/ W 2))
	= oy (- (+ (/ TILE_SIZE 2) (* py TILE_SIZE)) (/ H 2))

	= sx (* px TILE_SIZE)
	= sy (* py TILE_SIZE)

	if (== moving 1) draw_moving
	number dx dy
	= dx dir_x
	= dy dir_y
	number inc
	= inc TILE_SIZE
	number p
	= p move_count
	/ p MOVE_TIME
	* inc p
	* dx inc
	* dy inc
	+ ox dx
	+ oy dy
	+ sx dx
	+ sy dy
:draw_moving

	number map_w map_h
	tilemap_size tilemap map_w map_h
	* map_w TILE_SIZE
	* map_h TILE_SIZE

	if (< ox 0) too_left (> ox (- map_w W)) too_right
	= ox 0
:too_left
	= ox (- map_w W)
:too_right

	if (< oy 0) too_up (> oy (- map_h H)) too_down
	= oy 0
:too_up
	= oy (- map_h H)
:too_down

	number layers
	tilemap_num_layers tilemap layers

	tilemap_draw tilemap 0 1 (* ox -1) (* oy -1)

	sprite_draw sprite 255 255 255 255 (- sx ox) (- sy oy) 0 0
	
	tilemap_draw tilemap 2 (- layers 1) (* ox -1) (* oy -1)
}

function run
{
	include "poll_joystick.inc"

	number map_w map_h
	tilemap_size tilemap map_w map_h

	if (== moving 1) do_moving not_moving
	+ move_count 1
	if (>= move_count MOVE_TIME) done_moving
	if (== dir_x -1) check_l (== dir_x 1) check_r (== dir_y -1) check_u (== dir_y 1) check_d
	if (!= joy_l 1) done_l
	= moving 0
	:done_l
	:check_l
	if (!= joy_r 1) done_r
	= moving 0
	:done_r
	:check_r
	if (!= joy_u 1) done_u
	= moving 0
	:done_u
	:check_u
	if (!= joy_d 1) done_d
	= moving 0
	:done_d
	:check_d
	+ px dir_x
	+ py dir_y
	= move_count 0
	if (== moving 1) check_solids2
	number xx
	number yy
	= xx px
	= yy py
	+ xx dir_x
	+ yy dir_y
	number s
	tilemap_is_solid tilemap s xx yy
	if (== s 1) stop
	= moving 0
	:stop
	:check_solids2
	if (== moving 0) stand
	string suffix
	call_result suffix suffix_from_dirs dir_x dir_y
	string a
	= a "stand"
	+ a suffix
	sprite_set_animation sprite a
	:stand
	:done_moving
:do_moving
	if (== joy_l 1) go_left (== joy_r 1) go_right (== joy_u 1) go_up (== joy_d 1) go_down
	= moving 1
	= move_count 0
	= dir_x -1
	= dir_y 0
:go_left
	= moving 1
	= move_count 0
	= dir_x 1
	= dir_y 0
:go_right
	= moving 1
	= move_count 0
	= dir_x 0
	= dir_y -1
:go_up
	= moving 1
	= move_count 0
	= dir_x 0
	= dir_y 1
:go_down
	if (== moving 1) check_solids
	number xx yy
	= xx px
	= yy py
	+ xx dir_x
	+ yy dir_y
	if (== 0 (&& (>= xx 0) (>= yy 0) (< xx map_w) (< yy map_h))) cant_move continue_check
	= moving 0
	:cant_move
	number solid
	tilemap_is_solid tilemap solid xx yy
	if (== 1 solid) cant_move2
	= moving 0
	:cant_move2
	:continue_check
	:check_solids
	string suffix
	call_result suffix suffix_from_dirs dir_x dir_y
	string a
	if (== moving 1) walk stand2
	= a "walk"
	+ a suffix
	sprite_set_animation sprite a
	:walk
	= a "stand"
	+ a suffix
	sprite_set_animation sprite a
	:stand2
:not_moving
}
