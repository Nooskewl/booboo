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

set_3d

function draw
{
	clear 100 100 255

	draw_3d verts faces colours 12
}

function run
{
	rotate_3d 0.01 0.02 0.0
}
