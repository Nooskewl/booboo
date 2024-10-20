number cock fire
sample_load cock "gun-cocking-01.wav"
sample_load fire "pistol.ogg"

number font
font_load font "font.ttf" 32 1

function event type a b c d
{
	if
		(&& (== type EVENT_KEY_DOWN) (== a KEY_SPACE) (== b FALSE)) cock_it
		(&& (== type EVENT_KEY_UP) (== a KEY_SPACE)) fire_it
		sample_play cock 1 0
	:cock_it
		sample_play fire 1 0
	:fire_it
}

function draw
{
	font_draw font 50 255 100 255 "Press SPACE" 25 25
}
