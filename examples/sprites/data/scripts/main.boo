resize 320 180

number sprite
sprite_load sprite "pleasant"

vector names
vector_add names "stand_s"
vector_add names "walk_s"
vector_add names "stand_n"
vector_add names "walk_n"
vector_add names "stand_e"
vector_add names "walk_e"
vector_add names "stand_w"
vector_add names "walk_w"
vector_add names "stick"
vector_add names "throw_stick"
vector_add names "threw_w"
vector_add names "stand_w_exclamation"
vector_add names "drag_tom"
vector_add names "stand_e_exclamation"
vector_add names "attack_punch"
vector_add names "mouth_open"
vector_add names "use"
vector_add names "dead"
vector_add names "hit"
vector_add names "stand_n_exclamation"
vector_add names "attack_uppercut"
vector_add names "fall"
vector_add names "throw_item"
vector_add names "attack_fierce_punch"
vector_add names "attack_combo_punch"
vector_add names "victory"
vector_add names "attack_axe_hammer"
vector_add names "attack_cyclone"
vector_add names "attack_seering_slap"
vector_add names "attack_trout_slap"
vector_add names "throw_axe"
vector_add names "idle"
vector_add names "attack_nose_breaker"
vector_add names "attack_limbs_a_flailin"
vector_add names "attack_defend"
vector_add names "defend"

call callback sprite

function callback id
{
	number sz
	vector_size names sz
	number r
	rand r 0 (- sz 1)
	sprite_set_animation id [names r] callback
}

function draw
{
	number w h
	sprite_current_frame_size sprite w h

	number dx dy
	= dx (- 160 (/ w 2))
	= dy (- 90 (/ h 2))

	sprite_draw sprite 255 255 255 255 dx dy 0 0
}
