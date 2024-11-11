number shader
shader_load shader "lit_3d_vertex" ""

vector verts faces colours
vector_add verts -0.5
vector_add verts 0.5
vector_add verts 0.5
vector_add verts 0.5
vector_add verts 0.5
vector_add verts 0.5
vector_add verts -0.5
vector_add verts 0.5
vector_add verts -0.5
vector_add verts 0.5
vector_add verts 0.5
vector_add verts -0.5

vector_add verts -0.5
vector_add verts -0.5
vector_add verts 0.5
vector_add verts 0.5
vector_add verts -0.5
vector_add verts 0.5
vector_add verts -0.5
vector_add verts -0.5
vector_add verts -0.5
vector_add verts 0.5
vector_add verts -0.5
vector_add verts -0.5

; top
vector_add faces 0
vector_add faces 1
vector_add faces 2
vector_add faces 1
vector_add faces 2
vector_add faces 3

; bottom
vector_add faces 4
vector_add faces 5
vector_add faces 6
vector_add faces 5
vector_add faces 6
vector_add faces 7

; left
vector_add faces 0
vector_add faces 2
vector_add faces 4
vector_add faces 2
vector_add faces 4
vector_add faces 6

; right
vector_add faces 1
vector_add faces 3
vector_add faces 5
vector_add faces 3
vector_add faces 5
vector_add faces 7

; back
vector_add faces 0
vector_add faces 1
vector_add faces 4
vector_add faces 1
vector_add faces 4
vector_add faces 5

; front
vector_add faces 2
vector_add faces 3
vector_add faces 6
vector_add faces 3
vector_add faces 6
vector_add faces 7

vector_add colours 255
vector_add colours 216
vector_add colours 0
vector_add colours 255
vector_add colours 255
vector_add colours 216
vector_add colours 0
vector_add colours 255
vector_add colours 255
vector_add colours 216
vector_add colours 0
vector_add colours 255

vector_add colours 255
vector_add colours 216
vector_add colours 0
vector_add colours 255
vector_add colours 255
vector_add colours 216
vector_add colours 0
vector_add colours 255
vector_add colours 255
vector_add colours 216
vector_add colours 0
vector_add colours 255

vector_add colours 255
vector_add colours 216
vector_add colours 0
vector_add colours 255
vector_add colours 255
vector_add colours 216
vector_add colours 0
vector_add colours 255
vector_add colours 255
vector_add colours 216
vector_add colours 0
vector_add colours 255

vector_add colours 255
vector_add colours 216
vector_add colours 0
vector_add colours 255
vector_add colours 255
vector_add colours 216
vector_add colours 0
vector_add colours 255
vector_add colours 255
vector_add colours 216
vector_add colours 0
vector_add colours 255

vector_add colours 255
vector_add colours 0
vector_add colours 216
vector_add colours 255
vector_add colours 255
vector_add colours 0
vector_add colours 216
vector_add colours 255
vector_add colours 255
vector_add colours 0
vector_add colours 216
vector_add colours 255

vector_add colours 255
vector_add colours 0
vector_add colours 216
vector_add colours 255
vector_add colours 255
vector_add colours 0
vector_add colours 216
vector_add colours 255
vector_add colours 255
vector_add colours 0
vector_add colours 216
vector_add colours 255

vector_add colours 255
vector_add colours 0
vector_add colours 216
vector_add colours 255
vector_add colours 255
vector_add colours 0
vector_add colours 216
vector_add colours 255
vector_add colours 255
vector_add colours 0
vector_add colours 216
vector_add colours 255

vector_add colours 255
vector_add colours 0
vector_add colours 216
vector_add colours 255
vector_add colours 255
vector_add colours 0
vector_add colours 216
vector_add colours 255
vector_add colours 255
vector_add colours 0
vector_add colours 216
vector_add colours 255

vector_add colours 0
vector_add colours 216
vector_add colours 255
vector_add colours 255
vector_add colours 0
vector_add colours 216
vector_add colours 255
vector_add colours 255
vector_add colours 0
vector_add colours 216
vector_add colours 255
vector_add colours 255

vector_add colours 0
vector_add colours 216
vector_add colours 255
vector_add colours 255
vector_add colours 0
vector_add colours 216
vector_add colours 255
vector_add colours 255
vector_add colours 0
vector_add colours 216
vector_add colours 255
vector_add colours 255

vector_add colours 0
vector_add colours 216
vector_add colours 255
vector_add colours 255
vector_add colours 0
vector_add colours 216
vector_add colours 255
vector_add colours 255
vector_add colours 0
vector_add colours 216
vector_add colours 255
vector_add colours 255

vector_add colours 0
vector_add colours 216
vector_add colours 255
vector_add colours 255
vector_add colours 0
vector_add colours 216
vector_add colours 255
vector_add colours 255
vector_add colours 0
vector_add colours 216
vector_add colours 255
vector_add colours 255

vector normals
vector_add normals 0 1 0
vector_add normals 0 1 0
vector_add normals 0 1 0
vector_add normals 0 1 0
vector_add normals 0 1 0
vector_add normals 0 1 0

vector_add normals 0 -1 0
vector_add normals 0 -1 0
vector_add normals 0 -1 0
vector_add normals 0 -1 0
vector_add normals 0 -1 0
vector_add normals 0 -1 0

vector_add normals -1 0 0
vector_add normals -1 0 0
vector_add normals -1 0 0
vector_add normals -1 0 0
vector_add normals -1 0 0
vector_add normals -1 0 0

vector_add normals 1 0 0
vector_add normals 1 0 0
vector_add normals 1 0 0
vector_add normals 1 0 0
vector_add normals 1 0 0
vector_add normals 1 0 0

vector_add normals 0 0 1
vector_add normals 0 0 1
vector_add normals 0 0 1
vector_add normals 0 0 1
vector_add normals 0 0 1
vector_add normals 0 0 1

vector_add normals 0 0 -1
vector_add normals 0 0 -1
vector_add normals 0 0 -1
vector_add normals 0 0 -1
vector_add normals 0 0 -1
vector_add normals 0 0 -1

number rx ry
= rx 0
= ry 0

set_3d

vector light_pos
vector_init light_pos 0 100 0

function draw
{
	clear 100 100 255

	shader_use shader
	shader_set_float_vector shader "light_pos" light_pos
	shader_set_colour shader "ground_colour" 255 255 255 255

	draw_3d verts faces colours normals 12
}

function run
{
	+ rx 0.01
	+ ry 0.02
	identity_3d
	translate_3d 0 0 -5
	rotate_3d rx 1 0 0
	rotate_3d ry 0 1 0
}
