var c
= c 0
var old
= old 0

function draw
{
	clear c c c
}

function run
{
	var k
	= k (key_get KEY_RETURN)
	if (&& (== k 1) (== old 0)) change
		if (== c 0) white black
			= c 255
		:white
			= c 0
		:black
	:change
	= old k
}
