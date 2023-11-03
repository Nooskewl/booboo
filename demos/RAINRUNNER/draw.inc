function draw
{
	clear 0 0 0

	line 0 255 0 255 0 ground 640 ground 2

	number flip_h
	= flip_h 0
	? dir 1
	je do_cx_cy
	= flip_h 1

:do_cx_cy
	number cx
	number cy
	number img
	? jumping 0
	je cy_bottom
	= img jump_img
	goto calc_cx_cy
:cy_bottom
	= img idle_img
:calc_cx_cy
	image_size img cx cy
	/ cx 2
	/ cy 2

	number dx
	number dy
	= dx px
	= dy py
	- dy cy

	? moving 0
	je draw_player
	? jumping 0
	jne draw_player
	number t
	get_ticks t
	- t start_ticks
	% t 1000
	? t 500
	jl use_run2
	= img run1_img
	goto draw_player
:use_run2
	= img run2_img

:draw_player
	number a
	= a angle
	? dir 1
	je do_draw_player
	neg a
:do_draw_player
	image_draw_rotated_scaled img 255 255 255 255 cx cy dx dy a 1 1 flip_h 0

	? have_umbrella 1
	jne dont_draw_umbrella_on_player

	number w
	number h
	image_size umbrella_img w h
	/ h 2
	- dy h
	image_draw_rotated_scaled umbrella_img 255 255 255 255 cx cy dx dy 0 1 1 0 0

:dont_draw_umbrella_on_player
	
	; draw drops

	number num_drops
	vector_size drops num_drops

	? num_drops 0
	je no_drops_to_draw

	number i
	= i 0
:draw_next_drop
	vector drop
	vector_get drops drop i
	number drop_x
	number drop_y
	vector_get drop drop_x 0
	vector_get drop drop_y 1
	number cx
	number cy
	image_size drop_img cx cy
	/ cx 2
	/ cy 2
	image_draw_rotated_scaled drop_img 255 255 255 255 cx cy drop_x drop_y 0 1 1 0 0
	+ i 1
	? i num_drops
	jl draw_next_drop

:no_drops_to_draw

	; draw umbrella item
	? umbrella 0
	je no_umbrella_item_to_draw
	number cx
	number cy
	image_size umbrella_img cx cy
	/ cx 2
	/ cy 2
	image_draw_rotated_scaled umbrella_img 255 255 255 255 cx cy umbrella_x umbrella_y 0 1 1 0 0
:no_umbrella_item_to_draw

	; numeral drawing

	number i
	= i 0
:next_numeral_draw
	vector numeral
	vector_get numerals numeral i
	number gotten
	number nx
	number ny
	vector_get numeral gotten 0
	vector_get numeral nx 1
	vector_get numeral ny 2
	? gotten 1
	je dont_process_numeral_draw
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
	jl dont_process_numeral_draw
	number nw
	number nh
	number nimg
	vector_get numeral_imgs nimg i
	image_size nimg nw nh
	/ nw 2
	/ nh 2
	image_draw_rotated_scaled nimg 255 255 255 255 nw nh nx ny 0 1 1 0 0
:dont_process_numeral_draw
	+ i 1
	? i 10
	jl next_numeral_draw

	; draw life

	number lr
	number lg
	number lb
	number l
	= l life
	? l 0
	jge life_ok
	= l 0
:life_ok
	vector_get life_r lr l
	vector_get life_g lg l
	vector_get life_b lb l

	number box_w
	number box_h
	= box_h 16
	+ box_h 6
	= box_w 7
	+ box_w 2
	* box_w max_life
	+ box_w 6
	- box_w 2 ; last bar doesn't need pad

	rectangle lr lg lb 255 10 319 box_w box_h 1

	? life 0
	jle no_life_to_draw

	number i
	= i 0
:draw_next_life
	number dx
	= dx 10
	+ dx 3
	number bar_w
	= bar_w 9
	* bar_w i
	+ dx bar_w
	number dy
	= dy 319
	+ dy 3
	filled_rectangle lr lg lb 255 lr lg lb 255 lr lg lb 255 lr lg lb 255 dx dy 7 16
	+ i 1
	? i life
	jl draw_next_life
:no_life_to_draw

	; draw score
	string text
	number score_int
	= score_int score
	floor score_int
	string_format text "%" score_int
	number text_w
	font_width font text_w text
	number text_x
	= text_x 640
	- text_x 15
	- text_x text_w
	font_draw font 0 255 0 255 text text_x 305

	? life 0
	jg dont_draw_flood
	number p
	= p 180
	- p exit_count
	/ p 180
	number h
	= h 360
	* h p
	number dy
	= dy 360
	- dy h
	filled_rectangle 0 108 128 128 0 108 128 128 0 108 128 128 0 108 128 128 0 dy 640 360

:dont_draw_flood
}
