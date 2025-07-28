function test a
{
	= a 10
}

function test2 ~a
{
	= a 10
}

vector tmp
vector_init tmp 1 2 3

call test [tmp 1]

print "% % %\n" [tmp 0] [tmp 1] [tmp 2]

call test2 [tmp 1]

print "% % %\n" [tmp 0] [tmp 1] [tmp 2]

number tmp2
= tmp2 4
call test tmp2
print "%\n" tmp2
call test2 tmp2
print "%\n" tmp2
