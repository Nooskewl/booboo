number tilemap
tilemap_load tilemap "map.wm2"

number w h layers
tilemap_size tilemap w h
tilemap_num_layers tilemap layers

number dx dy
= dx 0
= dy 0

function draw
{
	tilemap_draw tilemap 0 (- layers 1) dx dy
}
