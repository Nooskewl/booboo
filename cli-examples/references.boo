function foo a ~b c
{
	= a 1
	= b 2
	= c 3
}

var a b c
= a 5
= b 10
= c 15

call foo a b c

print "% % %\n" a b c
