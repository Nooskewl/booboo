var num_stars
= num_stars 128
var max_trail
= max_trail 64
var velocity
= velocity 1

var stars

var i
for i 0 (< i num_stars) 1 add_another_star
	var v r x y
	= r (rand 0 639)
	= x r
	vector_add v r
	= r (rand 0 359)
	= y r
	vector_add v r
	var dx
	var dy
	= dx (- x 320)
	= dy (- y 180)
	var angle
	= angle (atan2 dy dx)
	var vx
	= vx (* (cos angle) velocity)
	var vy
	= vy (* (sin angle) velocity)
	vector_add v vx
	vector_add v vy
	vector_add stars v
:add_another_star

; cycle the animation so they don't all start in the centre
for i 0 (< i 120) 1 next_cycle
	call run
:next_cycle

function run
{
	var sz
	= sz (vector_size stars)

	for i 0 (< i num_stars) 1 next_update
		var v
		= v [stars i]
		var x y vx vy
		explode v x y vx vy
		= x (+ x vx)
		= y (+ y vy)
		? x 0
		jl regen
		? y 0
		jl regen
		? x 640
		jge regen
		? y 360
		jge regen
		goto done_regen

:regen
		= x 320
		= y 180
		goto done_regen

:done_regen
		= [v 0] x
		= [v 1] y
		= [stars i] v
:next_update
}

function draw
{
	clear 0 0 0

	var sz
	= sz (vector_size stars)

	for i 0 (< i num_stars) 1 next_draw
		var v
		= v [stars i]
		var x y vx vy
		explode v x y vx vy
		var cx
		var cy
		= cx 320
		= cy 180
		var dx
		var dy
		= dx (- cx x)
		= dy (- cy y)
		= dx (* dx dx)
		= dy (* dy dy)
		var dist
		= dist dx
		= dist (sqrt (+ dist dy))
		; 410px is about the max it can get from diagonal to center
		= dist (/ dist 410)
		= dist (* dist max_trail)
		var dx
		var dy
		= dx (- 320 x)
		= dy (- 180 y)
		var angle
		= angle (atan2 dy dx)
		= dx (* (cos angle) dist)
		= dy (* (sin angle) dist)
		var trail_x
		var trail_y
		= trail_x x
		= trail_x (- trail_x dx)
		= trail_y y
		= trail_y (- trail_y dy)
		line 255 255 255 255 x y trail_x trail_y 1
:next_draw

	filled_circle 0 0 0 255 320 180 16 -1
}
