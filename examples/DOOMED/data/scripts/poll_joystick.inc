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

= joy_x1 (joy_get_axis 0 JOY_LEFTX)
= joy_y1 (joy_get_axis 0 JOY_LEFTY)
= joy_x2 (joy_get_axis 0 JOY_RIGHTX)
= joy_y2 (joy_get_axis 0 JOY_RIGHTY)
= joy_x3 (joy_get_axis 0 JOY_TRIGGERLEFT)
= joy_y3 (joy_get_axis 0 JOY_TRIGGERRIGHT)

= joy_a (joy_get_button 0 JOY_A)
= joy_b (joy_get_button 0 JOY_B)
= joy_x (joy_get_button 0 JOY_X)
= joy_y (joy_get_button 0 JOY_Y)
= joy_u (joy_get_button 0 JOY_U)
= joy_d (joy_get_button 0 JOY_D)
= joy_l (joy_get_button 0 JOY_L)
= joy_r (joy_get_button 0 JOY_R)
= joy_ls (joy_get_button 0 JOY_LS)
= joy_rs (joy_get_button 0 JOY_RS)
= joy_lb (joy_get_button 0 JOY_LB)
= joy_rb (joy_get_button 0 JOY_RB)
= joy_back (joy_get_button 0 JOY_BACK)
= joy_start (joy_get_button 0 JOY_START)

; This provides some keyboard support mapped to joystick

number _key_l _key_r _key_u _key_d _key_a _key_b _key_back
= _key_l (key_get KEY_LEFT)
= _key_r (key_get KEY_RIGHT)
= _key_u (key_get KEY_UP)
= _key_d (key_get KEY_DOWN)
= _key_a (key_get KEY_Z)
= _key_b (key_get KEY_X)
= _key_back (key_get KEY_ESCAPE)

= joy_l (|| (== _key_l 1) (== joy_l 1))
= joy_r (|| (== _key_r 1) (== joy_r 1))
= joy_u (|| (== _key_u 1) (== joy_u 1))
= joy_d (|| (== _key_d 1) (== joy_d 1))
= joy_a (|| (== _key_a 1) (== joy_a 1))
= joy_b (|| (== _key_b 1) (== joy_b 1))
= joy_back (|| (== _key_back 1) (== joy_back 1))
