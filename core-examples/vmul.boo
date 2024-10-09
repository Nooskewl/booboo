vector v mat tmp
vector_init v 1 2 3
vector_init tmp 1 2 3
vector_add mat tmp
vector_init tmp 4 5 6
vector_add mat tmp
vector_init tmp 7 8 9
vector_add mat tmp

= tmp (vmul v mat)

print "% % %\n" [tmp 0] [tmp 1] [tmp 2]
