vector m1 m2 tmp
vector_init tmp 1 4
vector_add m1 tmp
vector_init tmp 2 5
vector_add m1 tmp
vector_init tmp 3 6
vector_add m1 tmp
vector_init tmp 7 9 11
vector_add m2 tmp
vector_init tmp 8 10 12
vector_add m2 tmp

= tmp (mul m1 m2)

print "%\t%\n" [tmp 0 0] [tmp 1 0]
print "%\t%\n" [tmp 0 1] [tmp 1 1]
