function a
{
	print "a\n"
}

function b
{
	print "b\n"
}

function local_mod x
{
	= x 1
}

pointer p
address p b
call p
address p a
call p
call p
call p

vector v
vector_add v b
vector_add v a
vector_add v a
vector_add v a

number sz
vector_size v sz
number i
= i 0
:next_loop
vector_get v p i
call p
+ i 1
? i sz
jl next_loop

string foobar
= foobar "foobar\n"
address p foobar
= p "baz\n"
print foobar

number test
= test 0
call local_mod test
print "%\n" test
address p test
call local_mod p
print "%\n" test
