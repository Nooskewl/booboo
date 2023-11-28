number min_wait
number max_wait
= min_wait 500
* min_wait 60
= max_wait 1000
* max_wait 60

number drop_img
number idle_img
number jump_img
number run1_img
number run2_img
number umbrella_img
image_load drop_img "game_gfx/drop.png"
image_load idle_img "game_gfx/idle.png"
image_load jump_img "game_gfx/jump.png"
image_load run1_img "game_gfx/run1.png"
image_load run2_img "game_gfx/run2.png"
image_load umbrella_img "game_gfx/umbrella.png"

vector numeral_imgs ; I really should have named these 0-9...
number img
image_load img "game_gfx/i.png"
vector_add numeral_imgs img
image_load img "game_gfx/ii.png"
vector_add numeral_imgs img
image_load img "game_gfx/iii.png"
vector_add numeral_imgs img
image_load img "game_gfx/iv.png"
vector_add numeral_imgs img
image_load img "game_gfx/v.png"
vector_add numeral_imgs img
image_load img "game_gfx/vi.png"
vector_add numeral_imgs img
image_load img "game_gfx/vii.png"
vector_add numeral_imgs img
image_load img "game_gfx/viii.png"
vector_add numeral_imgs img
image_load img "game_gfx/ix.png"
vector_add numeral_imgs img
image_load img "game_gfx/x.png"
vector_add numeral_imgs img

number font
font_load font "font.ttf" 48 1

number hit_sfx
number jump_sfx
number collect_sfx
number drip_sfx
number drop_ground_sfx
number hit_umbrella_sfx
number gameover_sfx
number win_sfx
mml_create hit_sfx "A @TYPE1 a16"
mml_create jump_sfx "@PO0 = { 0 100 }\nA @PO0 a16 @PO0"
mml_create collect_sfx "A @TYPE0 y32 c32g16"
mml_create drip_sfx "@PO0 = { 0 1000 }\nA o3 @TYPE3 @PO0 g32 @PO0"
mml_create drop_ground_sfx "@PO0 = { 0 -1000 0 1000 }\nA @TYPE3 o5 @PO0 a8 @PO0"
mml_create hit_umbrella_sfx "@PO0 = { 0 -100 }\nA @TYPE3 @PO0 g4 @PO0"
mml_create gameover_sfx "@PO0 = { 0 -500 100 -300 500 -500 0 -300 }\nA @TYPE1 o8 @PO0 c @PO0"
mml_load win_sfx "sfx/win.mml"

number music
mml_load music "music/music.mml"
mml_play music 0.5 1

number ground
= ground 300

number px
number py
= px 320
= py ground

number jumping
= jumping 0
number jumps
= jumps 0
number moving
= moving 1
number dir
= dir 1

number move_speed
= move_speed 5.0

number gravity
= gravity 20
number drop_gravity
= drop_gravity 10
number jump_accel
= jump_accel -31
number jump_accel_damp
= jump_accel_damp 0.5

number vy
= vy 0

include "poll_joystick.inc"
number old_joy_a
= old_joy_a joy_a

number angle
= angle 0

number score
= score 0

vector drops
number next_drop
= next_drop 180
number drop_start_y
= drop_start_y -50
number drop_interval
= drop_interval 60
number speed_up
= speed_up 1800

number max_life
= max_life 10
number life
= life max_life

number next_umbrella
= next_umbrella 1800
number umbrella_x
number umbrella_y
number umbrella
= umbrella 0

number have_umbrella
= have_umbrella 0

vector numerals
number i
= i 0
:add_next_numeral
vector numeral
vector_add numeral 0
number r
rand r 50 589
vector_add numeral r
rand r 50 289
vector_add numeral r
vector_add numerals numeral
+ i 1
? i 10
jl add_next_numeral

number won
= won 0
number exit_count

number start_ticks
get_ticks start_ticks

vector life_r
vector life_g
vector life_b
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
