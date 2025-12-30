var shader
= shader (shader_load "lit_3d_vertex" "")

var tex
= tex (image_load "brick.png")

var verts faces colours normals texcoords

vector_add verts -0.5 0.5 0.5
vector_add verts 0.5 0.5 0.5
vector_add verts 0.5 0.5 -0.5
vector_add verts -0.5 0.5 -0.5

vector_add verts -0.5 -0.5 0.5
vector_add verts 0.5 -0.5 0.5
vector_add verts 0.5 -0.5 -0.5
vector_add verts -0.5 -0.5 -0.5

vector_add faces 2 1 0
vector_add faces 3 2 0
vector_add faces 4 5 6
vector_add faces 4 6 7

vector_add faces 7 2 3
vector_add faces 7 6 2
vector_add faces 0 1 4
vector_add faces 1 5 4

vector_add faces 4 3 0
vector_add faces 4 7 3
vector_add faces 5 1 2
vector_add faces 6 5 2

vector_add texcoords 3 3 3 0 0 0
vector_add texcoords 0 3 3 3 0 0
vector_add texcoords 0 0 3 0 3 3
vector_add texcoords 0 0 3 3 0 3

vector_add texcoords 0 3 3 0 0 0
vector_add texcoords 0 3 3 3 3 0
vector_add texcoords 3 0 0 0 3 3
vector_add texcoords 0 0 0 3 3 3

vector_add texcoords 0 3 3 0 0 0
vector_add texcoords 0 3 3 3 3 0
vector_add texcoords 3 3 3 0 0 0
vector_add texcoords 0 3 3 3 0 0

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

vector_add normals 0 0 -1
vector_add normals 0 0 -1
vector_add normals 0 0 -1
vector_add normals 0 0 -1
vector_add normals 0 0 -1
vector_add normals 0 0 -1

vector_add normals 0 0 1
vector_add normals 0 0 1
vector_add normals 0 0 1
vector_add normals 0 0 1
vector_add normals 0 0 1
vector_add normals 0 0 1

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

var rx ry
= rx 0
= ry 0

var vertex_buffer
= vertex_buffer (create_vertex_buffer (@ verts) (@ faces) (@ colours) (@ normals) (@ texcoords) 12)

set_3d

var light_pos
vector_init light_pos 0 100 0

function draw
{
	clear 100 100 255

	shader_use shader
	shader_set_float_vector shader "light_pos" light_pos
	shader_set_colour shader "ground_colour" 255 255 255 255

	draw_3d_textured vertex_buffer tex
}

function run
{
	= rx (+ rx 0.01)
	= ry (+ ry 0.02)
	identity_3d
	translate_3d 0 0 -5
	rotate_3d rx 1 0 0
	rotate_3d ry 0 1 0
}
