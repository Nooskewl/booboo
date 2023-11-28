number joy_x1
number joy_y1
number joy_x2
number joy_y2
number joy_x3
number joy_y3
number joy_l
number joy_r
number joy_u
number joy_d
number joy_a
number joy_b
number joy_x
number joy_y
number joy_lb
number joy_rb
number joy_ls
number joy_rs
number joy_back
number joy_start

joystick_poll 0 joy_x1 joy_y1 joy_x2 joy_y2 joy_x3 joy_y3 joy_l joy_r joy_u joy_d joy_a joy_b joy_x joy_y joy_lb joy_rb joy_ls joy_rs joy_back joy_start

; This provides some keyboard support mapped to joystick

include "keycodes.inc"

number _key_l _key_r _key_u _key_d _key_a _key_b _key_back
key_get _key_l KEY_LEFT
key_get _key_r KEY_RIGHT
key_get _key_u KEY_UP
key_get _key_d KEY_DOWN
key_get _key_a KEY_z
key_get _key_b KEY_x
key_get _key_back KEY_ESCAPE

= joy_l (|| (== _key_l 1) (== joy_l 1))
= joy_r (|| (== _key_r 1) (== joy_r 1))
= joy_u (|| (== _key_u 1) (== joy_u 1))
= joy_d (|| (== _key_d 1) (== joy_d 1))
= joy_a (|| (== _key_a 1) (== joy_a 1))
= joy_b (|| (== _key_b 1) (== joy_b 1))
= joy_back (|| (== _key_back 1) (== joy_back 1))
