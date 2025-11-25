function draw
{
	clear 0 0 0

	line 0 255 0 255 0 ground 640 ground 2

	var flip_h
	= flip_h 0
	? dir 1
	je do_cx_cy
	= flip_h 1

:do_cx_cy
	var cx
	var cy
	var img
	? jumping 0
	je cy_bottom
	= img jump_img
	goto calc_cx_cy
:cy_bottom
	= img idle_img
:calc_cx_cy
	explode (image_size img) cx cy
	= cx (/ cx 2)
	= cy (/ cy 2)

	var dx
	var dy
	= dx px
	= dy (- py cy)

	? moving 0
	je draw_player
	? jumping 0
	jne draw_player
	var t
	= t (% (- (get_ticks) start_ticks) 1000)
	? t 500
	jl use_run2
	= img run1_img
	goto draw_player
:use_run2
	= img run2_img

:draw_player
	var a
	= a angle
	? dir 1
	je do_draw_player
	= a (neg a)
:do_draw_player
	image_draw_rotated_scaled img 255 255 255 255 cx cy dx dy a 1 1 flip_h 0

	? have_umbrella 1
	jne dont_draw_umbrella_on_player

	var w
	var h
	explode (image_size umbrella_img) w h
	= h (/ h 2)
	= dy (- dy h)
	image_draw_rotated_scaled umbrella_img 255 255 255 255 cx cy dx dy 0 1 1 0 0

:dont_draw_umbrella_on_player
	
	; draw drops

	var num_drops
	if (!= (typeof drops) "vector") zero_drops set_it
		= num_drops 0
	:zero_drops
		= num_drops (vector_size drops)
	:set_it

	? num_drops 0
	je no_drops_to_draw

	var i
	= i 0
:draw_next_drop
	var drop
	= drop [drops i]
	var drop_x
	var drop_y
	explode drop drop_x drop_y
	var cx
	var cy
	explode (image_size drop_img) cx cy
	= cx (/ cx 2)
	= cy (/ cy 2)
	image_draw_rotated_scaled drop_img 255 255 255 255 cx cy drop_x drop_y 0 1 1 0 0
	= i (+ i 1)
	? i num_drops
	jl draw_next_drop

:no_drops_to_draw

	; draw umbrella item
	? umbrella 0
	je no_umbrella_item_to_draw
	var cx
	var cy
	explode (image_size umbrella_img) cx cy
	= cx (/ cx 2)
	= cy (/ cy 2)
	image_draw_rotated_scaled umbrella_img 255 255 255 255 cx cy umbrella_x umbrella_y 0 1 1 0 0
:no_umbrella_item_to_draw

	; numeral drawing

	var i
	= i 0
:next_numeral_draw
	var numeral
	= numeral [numerals i]
	var gotten
	var nx
	var ny
	explode numeral gotten nx ny
	? gotten 1
	je dont_process_numeral_draw
	var instantiate_time
	= instantiate_time 15000
	var mul
	= mul (+ i 1)
	= instantiate_time (* instantiate_time mul)
	var t
	= t (- (get_ticks) start_ticks)
	? t instantiate_time
	jl dont_process_numeral_draw
	var nw
	var nh
	var nimg
	= nimg [numeral_imgs i]
	explode (image_size nimg) nw nh
	= nw (/ nw 2)
	= nh (/ nh 2)
	image_draw_rotated_scaled nimg 255 255 255 255 nw nh nx ny 0 1 1 0 0
:dont_process_numeral_draw
	= i (+ i 1)
	? i 10
	jl next_numeral_draw

	; draw life

	var lr
	var lg
	var lb
	var l
	= l life
	? l 0
	jge life_ok
	= l 0
:life_ok
	= lr [life_r l]
	= lg [life_g l]
	= lb [life_b l]

	var box_w
	var box_h
	= box_h (+ 16 6)
	= box_w (- (+ (* (+ 7 2) max_life) 6) 2)

	rectangle lr lg lb 255 10 319 box_w box_h 1

	? life 0
	jle no_life_to_draw

	var i
	= i 0
:draw_next_life
	var dx
	= dx (+ 10 3)
	var bar_w
	= bar_w (* 9 i)
	= dx (+ dx bar_w)
	var dy
	= dy (+ 319 3)
	filled_rectangle lr lg lb 255 lr lg lb 255 lr lg lb 255 lr lg lb 255 dx dy 7 16
	= i (+ i 1)
	? i life
	jl draw_next_life
:no_life_to_draw

	; draw score
	var text
	var score_int
	= score_int (number (floor score))
	= text (string_format "%" score_int)
	var text_w
	= text_w (font_width font text)
	var text_x
	= text_x (- 640 15 text_w)
	font_draw font 0 255 0 255 text text_x 305

	? life 0
	jg dont_draw_flood
	var p
	= p (/ (- 180 exit_count) 180)
	var h
	= h (* 360 p)
	var dy
	= dy (- 360 h)
	filled_rectangle 0 108 128 128 0 108 128 128 0 108 128 128 0 108 128 128 0 dy 640 360

:dont_draw_flood
}
