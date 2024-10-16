vector v1 v2 v3
vector_init v1 1 0 0
vector_init v2 1 1 0
vector_init v3 3 3 0

print "v1="
call printv v1
print "v2="
call printv v2
print "v3="
call printv v3

number a
= a (angle v1 v2)
print "Angle=%\n" a

= a (length v2)
print "Length of v2=%\n" a

vector c
= c (cross v1 v2)
print "Cross="
call printv c

= a (dot v1 v2)
print "Dot product=%\n" a

= c (mul v2 5)
print "v2 * 5="
call printv c

= c (normalize v3)
print "V3 normalized="
call printv c

= c (add v1 v2)
print "v1 + v2="
call printv c

= c (sub v3 v2)
print "v3 - v2="
call printv c

function printv v
{
	print "[% % %]\n" [v 0] [v 1] [v 2]
}
