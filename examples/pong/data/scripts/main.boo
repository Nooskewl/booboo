; Get the default screen size - that's what we'll use
number SCR_W SCR_H
get_buffer_size SCR_W SCR_H

; All of these numbers define the game, from sizes of ball and paddles, to positions and speeds
number PAD_H PAD_W BALL_SIZE BOTTOM score1 score2
number pad1y pad2y ballx bally balldx balldy
number BALL_SPEED PAD_SPEED GUTTER
; Set these values to defaults
= BOTTOM 50 ; 50 pixels at the bottom of the screen for score
= PAD_H (/ (- SCR_H BOTTOM) 8) ; paddle height (1/8th of the screen)
= PAD_W (/ PAD_H 3) ; paddle width
= BALL_SIZE (/ PAD_H 2.5) ; ball size
= score1 0 ; default scores
= score2 0
= pad1y (- (/ (- SCR_H BOTTOM) 2) (/ PAD_H 2)) ; position paddles in the middle
= pad2y pad1y
= BALL_SPEED 2.0 ; default speeds
= PAD_SPEED 3.5
= GUTTER 15 ; space between edge and paddle

; Set the ball moving left/right randomly
call randballdir

; Load a font
number font
font_load font "font.ttf" 24 1

; This places the ball in the middle and sets it moving randomly up/down/left/right
function randballdir
{
	; Position the ball in the centre
	= ballx (- (/ SCR_W 2) (/ BALL_SIZE 2))
	= bally (- (/ (- SCR_H BOTTOM) 2) (/ BALL_SIZE 2))

	; Generate random numbers between 0 and 1
	number r1 r2
	rand r1 0 1
	rand r2 0 1
	; Changes 0s to -1s
	if (== r1 0) negr1
		= r1 -1
	:negr1
	if (== r2 0) negr2
		= r2 -1
	:negr2
	; Set the ball velocity
	= balldx (* r1 BALL_SPEED)
	= balldy (* r2 BALL_SPEED)
}

function run
{
	; Move the ball
	+ ballx balldx
	+ bally balldy

	; Bounce the ball off walls and assign score if it hits the far left/right
	if (< ballx 0) posx (< bally 0) posy (>= (+ ballx BALL_SIZE) SCR_W) negx (>= (+ bally BALL_SIZE) (- SCR_H BOTTOM)) negy
		neg balldx
		+ score2 1
	:posx
		neg balldy
	:posy
		neg balldx
		+ score1 1
	:negx
		neg balldy
	:negy

	; Read the keyboard
	number kq ka ko kl
	key_get kq KEY_Q
	key_get ka KEY_A
	key_get ko KEY_O
	key_get kl KEY_L

	; Move paddles based on keys
	if (&& (> pad1y 0) (== kq TRUE)) p1up (&& (< pad1y (- SCR_H BOTTOM PAD_H)) (== ka TRUE)) p1dn (&& (> pad2y 0) (== ko TRUE)) p2up (&& (< pad2y (- SCR_H BOTTOM PAD_H)) (== kl TRUE)) p2dn
		- pad1y PAD_SPEED
	:p1up
		+ pad1y PAD_SPEED
	:p1dn
		- pad2y PAD_SPEED
	:p2up
		+ pad2y PAD_SPEED
	:p2dn

	; Change ball direction based on paddles
	number collides
	call_result collides ball_collides GUTTER pad1y
	if (== TRUE collides) hitpad1
		neg balldx
		= ballx (+ GUTTER PAD_W)
	:hitpad1
	call_result collides ball_collides (- SCR_W GUTTER PAD_W) pad2y
	if (== TRUE collides) hitpad2
		neg balldx
		= ballx (- SCR_W GUTTER PAD_W BALL_SIZE)
	:hitpad2
}

function draw
{
	; Draw left paddle
	filled_rectangle 255 255 255 255 255 255 255 255 255 255 255 255 255 255 255 255
		GUTTER pad1y PAD_W PAD_H
	; Draw right paddle
	filled_rectangle 255 255 255 255 255 255 255 255 255 255 255 255 255 255 255 255
		(- SCR_W GUTTER PAD_W) pad2y PAD_W PAD_H
	; Draw ball
	filled_rectangle 255 255 255 255 255 255 255 255 255 255 255 255 255 255 255 255
		ballx bally BALL_SIZE BALL_SIZE

	; Draw a line and the score below
	line 255 255 255 255 0 (- SCR_H BOTTOM) SCR_W (- SCR_H BOTTOM) 2

	string sc1 sc2
	number sc2w fh
	= sc1 score1
	= sc2 score2
	font_width font sc2w sc2
	font_height font fh
	font_draw font 255 255 255 255 sc1 GUTTER (- (+ (- SCR_H BOTTOM) (/ BOTTOM 2)) (/ fh 2))
	font_draw font 255 255 255 255 sc2 (- SCR_W GUTTER sc2w) (- (+ (- SCR_H BOTTOM) (/ BOTTOM 2)) (/ fh 2))
}

; Returns TRUE/FALSE depending if the ball collides with paddle at padx/pady
function ball_collides padx pady
{
	number bx2 by2
	= bx2 (+ ballx BALL_SIZE)
	= by2 (+ bally BALL_SIZE)
	number px2 py2
	= px2 (+ padx PAD_W)
	= py2 (+ pady PAD_H)
	if (|| (>= ballx px2) (>= bally py2) (< bx2 padx) (< by2 pady)) nope ; basic bounding box collision detection
		return FALSE
	:nope
	return TRUE
}
