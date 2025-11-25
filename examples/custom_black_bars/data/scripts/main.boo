resize 100 100

var iv ih it
= iv (image_load "vert.png")
= ih (image_load "horiz.png")
= it (image_load "tile.png")

var v_w v_h h_w h_h t_w t_h
explode (image_size iv) v_w v_h
explode (image_size ih) h_w h_h
explode (image_size it) t_w t_h

function draw_black_bar type x y w h
{
	var scale
	= scale 3

	var scr_w scr_h
	explode (get_screen_size) scr_w scr_h
	
	var xx yy

	if (== type BAR_TOP) top (== type BAR_BOTTOM) bottom (== type BAR_LEFT) left right
		= xx (neg (/ (- (* (+ (floor (/ w (* h_w scale))) 1) (* h_w scale) w) 2)))
		= yy (- (+ y h) (* h_h scale))
		call htile ih xx yy w h scale
		= xx (neg (/ (- (* (+ (floor (/ w (* t_w scale))) 1) (* t_w scale) w) 2)))
		if (> yy 0) tile
			:again
			= yy (- yy (* t_h scale))
			call htile it xx yy w h scale
			? yy 0
			jg again
		:tile
	:top
		= xx (neg (/ (- (* (+ (floor (/ w (* h_w scale))) 1) (* h_w scale) w) 2)))
		= yy y
		call htile ih xx yy w h scale
		= xx (neg (/ (- (* (+ (floor (/ w (* t_w scale))) 1) (* t_w scale) w) 2)))
		= yy (+ yy (* h_h scale))
		if (< yy scr_h) tile2
			:again2
			call htile it xx yy w h scale
			= yy (+ yy (* t_h scale))
			? yy scr_h
			jl again2
		:tile2
	:bottom
		= yy (neg (/ (- (* (+ (floor (/ h (* v_h scale))) 1) (* v_h scale) h) 2)))
		= xx (- (+ x w) (* v_w scale))
		call vtile iv xx yy w h scale
		= yy (neg (/ (- (* (+ (floor (/ h (* t_h scale))) 1) (* t_h scale) h) 2)))
		if (> xx 0) tile3
			:again3
			= xx (- xx (* t_w scale))
			call vtile it xx yy w h scale
			? xx 0
			jg again3
		:tile3
	:left
		= yy (neg (/ (- (* (+ (floor (/ h (* v_h scale))) 1) (* v_h scale) h) 2)))
		= xx x
		call vtile iv xx yy w h scale
		= yy (neg (/ (- (* (+ (floor (/ h (* t_h scale))) 1) (* t_h scale) h) 2)))
		= xx (+ xx (* v_w scale))
		if (< xx scr_w) tile4
			:again4
			call vtile it xx yy w h scale
			= xx (+ xx (* t_w scale))
			? xx scr_w
			jl again4
		:tile4
	:right	
}

function htile img x y w h scale
{
	var img_w img_h
	explode (image_size img) img_w img_h

:top
	image_stretch_region img 255 255 255 255 0 0 img_w img_h x y (* img_w scale) (* img_h scale) 0 0
	= x (+ x (* img_w scale))
	? x w
	jl top
}

function vtile img x y w h scale
{
	var img_w img_h
	explode (image_size img) img_w img_h

:top
	image_stretch_region img 255 255 255 255 0 0 img_w img_h x y (* img_w scale) (* img_h scale) 0 0
	= y (+ y (* img_h scale))
	? y h
	jl top
}
