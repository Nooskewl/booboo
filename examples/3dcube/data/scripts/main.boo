set_cull_mode NO_FACE

var verts faces colours normals texcoords
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

vector_add faces 0
vector_add faces 1
vector_add faces 2
vector_add faces 1
vector_add faces 2
vector_add faces 3

vector_add faces 4
vector_add faces 5
vector_add faces 6
vector_add faces 5
vector_add faces 6
vector_add faces 7

vector_add faces 0
vector_add faces 2
vector_add faces 4
vector_add faces 2
vector_add faces 4
vector_add faces 6

vector_add faces 1
vector_add faces 3
vector_add faces 5
vector_add faces 3
vector_add faces 5
vector_add faces 7

vector_add faces 0
vector_add faces 1
vector_add faces 4
vector_add faces 1
vector_add faces 4
vector_add faces 5

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

var vertex_buffer
= vertex_buffer (create_vertex_buffer (@ verts) (@ faces) (@ colours) (@ normals) (@ texcoords) 12)

var rx ry
= rx 0
= ry 0

set_3d

function draw
{
	clear 100 100 255

	draw_3d vertex_buffer
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
