var drop_img
var idle_img
var jump_img
var run1_img
var run2_img
var umbrella_img
= drop_img (image_load "game_gfx/drop.png")
= idle_img (image_load "game_gfx/idle.png")
= jump_img (image_load "game_gfx/jump.png")
= run1_img (image_load "game_gfx/run1.png")
= run2_img (image_load "game_gfx/run2.png")
= umbrella_img (image_load "game_gfx/umbrella.png")

var numeral_imgs ; I really should have named these 0-9...
var img
= img (image_load "game_gfx/i.png")
vector_add numeral_imgs img
= img (image_load "game_gfx/ii.png")
vector_add numeral_imgs img
= img (image_load "game_gfx/iii.png")
vector_add numeral_imgs img
= img (image_load "game_gfx/iv.png")
vector_add numeral_imgs img
= img (image_load "game_gfx/v.png")
vector_add numeral_imgs img
= img (image_load "game_gfx/vi.png")
vector_add numeral_imgs img
= img (image_load "game_gfx/vii.png")
vector_add numeral_imgs img
= img (image_load "game_gfx/viii.png")
vector_add numeral_imgs img
= img (image_load "game_gfx/ix.png")
vector_add numeral_imgs img
= img (image_load "game_gfx/x.png")
vector_add numeral_imgs img

var font
= font (font_load "font.ttf" 32 1)

var hit_sfx
var jump_sfx
var collect_sfx
var drip_sfx
var drop_ground_sfx
var hit_umbrella_sfx
var gameover_sfx
var win_sfx
= hit_sfx (mml_create "A @TYPE1 a16")
= jump_sfx (mml_create "@PO0 = { 0 100 }\nA @PO0 a16 @PO0")
= collect_sfx (mml_create "A @TYPE0 y32 c32g16")
= drip_sfx (mml_create "@PO0 = { 0 1000 }\nA o3 @TYPE3 @PO0 g32 @PO0")
= drop_ground_sfx (mml_create "@PO0 = { 0 -1000 0 1000 }\nA @TYPE3 o5 @PO0 a8 @PO0")
= hit_umbrella_sfx (mml_create "@PO0 = { 0 -100 }\nA @TYPE3 @PO0 g4 @PO0")
= gameover_sfx (mml_create "@PO0 = { 0 -500 100 -300 500 -500 0 -300 }\nA @TYPE1 o8 @PO0 c @PO0")
= win_sfx (mml_load "sfx/win.mml")

var music
= music (mml_load "music/music.mml")
mml_play music 0.5 1

var ground
= ground 300

var px
var py
= px 320
= py ground

var jumping
= jumping 0
var jumps
= jumps 0
var moving
= moving 1
var dir
= dir 1

var move_speed
= move_speed 5.0

var gravity
= gravity 20
var drop_gravity
= drop_gravity 10
var jump_accel
= jump_accel -31
var jump_accel_damp
= jump_accel_damp 0.5

var vy
= vy 0

include "poll_joystick.inc"
var old_joy_a
= old_joy_a joy_a

var angle
= angle 0

var score
= score 0

var drops
= drops (vector drops)
var next_drop
= next_drop 180
var drop_start_y
= drop_start_y -50
var drop_interval
= drop_interval 60
var speed_up
= speed_up 1800

var max_life
= max_life 10
var life
= life max_life

var next_umbrella
= next_umbrella 1800
var umbrella_x
var umbrella_y
var umbrella
= umbrella 0

var have_umbrella
= have_umbrella 0

var numerals
var i
= i 0
:add_next_numeral
var numeral
vector_add numeral 0
var r
= r (rand 50 589)
vector_add numeral r
= r (rand 50 289)
vector_add numeral r
vector_add numerals numeral
= i (+ i 1)
? i 10
jl add_next_numeral

var won
= won 0
var exit_count

var start_ticks
= start_ticks (get_ticks)

var life_r
var life_g
var life_b
vector_add life_r 0
vector_add life_r 255
vector_add life_r 255
vector_add life_r 255
vector_add life_r 255
vector_add life_r 255
vector_add life_r 0
vector_add life_r 0
vector_add life_r 0
vector_add life_r 0
vector_add life_r 0
vector_add life_g 0
vector_add life_g 0
vector_add life_g 0
vector_add life_g 255
vector_add life_g 255
vector_add life_g 255
vector_add life_g 255
vector_add life_g 255
vector_add life_g 255
vector_add life_g 255
vector_add life_g 255
vector_add life_b 0
vector_add life_b 0
vector_add life_b 0
vector_add life_b 0
vector_add life_b 0
vector_add life_b 0
vector_add life_b 0
vector_add life_b 0
vector_add life_b 0
vector_add life_b 0
vector_add life_b 0

include "run.inc"
include "draw.inc"
