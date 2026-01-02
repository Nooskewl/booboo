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
= vertex_buffer (create_vertex_buffer (@ verts) (@ faces) (@ colours) (@ normals) (@ texcoords) (/ (vector_size faces) 3))

var rx ry
= rx 0
= ry 0

set_3d
enable_depth_test TRUE
enable_depth_write TRUE

function draw
{
	clear 100 100 255

	draw_vertex_buffer vertex_buffer
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
