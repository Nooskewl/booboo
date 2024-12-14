resize 100 100

number iv ih it
image_load iv "vert.png"
image_load ih "horiz.png"
image_load it "tile.png"

number v_w v_h h_w h_h t_w t_h
image_size iv v_w v_h
image_size ih h_w h_h
image_size it t_w t_h

function draw_black_bar type x y w h
{
	number scale
	= scale 3

	number scr_w scr_h
	get_screen_size scr_w scr_h
	
	number xx yy

	if (== type BAR_TOP) top (== type BAR_BOTTOM) bottom (== type BAR_LEFT) left right
		= xx w
		/ xx (* h_w scale)
		floor xx
		+ xx 1
		* xx (* h_w scale)
		- xx w
		/ xx 2
		neg xx
		= yy (- (+ y h) (* h_h scale))
		call htile ih xx yy w h scale
		= xx w
		/ xx (* t_w scale)
		floor xx
		+ xx 1
		* xx (* t_w scale)
		- xx w
		/ xx 2
		neg xx
		if (> yy 0) tile
			:again
			- yy (* t_h scale)
			call htile it xx yy w h scale
			? yy 0
			jg again
		:tile
	:top
		= xx w
		/ xx (* h_w scale)
		floor xx
		+ xx 1
		* xx (* h_w scale)
		- xx w
		/ xx 2
		neg xx
		= yy y
		call htile ih xx yy w h scale
		= xx w
		/ xx (* t_w scale)
		floor xx
		+ xx 1
		* xx (* t_w scale)
		- xx w
		/ xx 2
		neg xx
		+ yy (* h_h scale)
		if (< yy scr_h) tile2
			:again2
			call htile it xx yy w h scale
			+ yy (* t_h scale)
			? yy scr_h
			jl again2
		:tile2
	:bottom
		= yy h
		/ yy (* v_h scale)
		floor yy
		+ yy 1
		* yy (* v_h scale)
		- yy h
		/ yy 2
		neg yy
		= xx (- (+ x w) (* v_w scale))
		call vtile iv xx yy w h scale
		= yy h
		/ yy (* t_h scale)
		floor yy
		+ yy 1
		* yy (* t_h scale)
		- yy h
		/ yy 2
		neg yy
		if (> xx 0) tile3
			:again3
			- xx (* t_w scale)
			call vtile it xx yy w h scale
			? xx 0
			jg again3
		:tile3
	:left
		= yy h
		/ yy (* v_h scale)
		floor yy
		+ yy 1
		* yy (* v_h scale)
		- yy h
		/ yy 2
		neg yy
		= xx x
		call vtile iv xx yy w h scale
		= yy h
		/ yy (* t_h scale)
		floor yy
		+ yy 1
		* yy (* t_h scale)
		- yy h
		/ yy 2
		neg yy
		+ xx (* v_w scale)
		if (< xx scr_w) tile4
			:again4
			call vtile it xx yy w h scale
			+ xx (* t_w scale)
			? xx scr_w
			jl again4
		:tile4
	:right	
}

function htile img x y w h scale
{
	number img_w img_h
	image_size img img_w img_h

:top
	image_stretch_region img 255 255 255 255 0 0 img_w img_h x y (* img_w scale) (* img_h scale) 0 0
	+ x (* img_w scale)
	? x w
	jl top
}

function vtile img x y w h scale
{
	number img_w img_h
	image_size img img_w img_h

:top
	image_stretch_region img 255 255 255 255 0 0 img_w img_h x y (* img_w scale) (* img_h scale) 0 0
	+ y (* img_h scale)
	? y h
	jl top
}
