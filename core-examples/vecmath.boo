vector v1 v2
vector_init v1 1 0 0
vector_init v2 1 1 0

print "v1="
call printv v1
print "v2="
call printv v2

number a
= a (vangle v1 v2)
print "Angle=%\n" a

= a (vlen v2)
print "Length of v2=%\n" a

vector c
= c (cross v1 v2)
print "Cross="
call printv c

= a (dot v1 v2)
print "Dot product=%\n" a

= c (vmul v2 5)
print "v2 * 5="
call printv c

function printv v
{
	print "[% % %]\n" [v 0] [v 1] [v 2]
}
