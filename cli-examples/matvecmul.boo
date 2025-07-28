vector mat vec tmp
vector_init vec 1 2 3
vector_init tmp 1 2 3
vector_add mat tmp
vector_init tmp 3 4 5
vector_add mat tmp
vector_init tmp 6 7 8
vector_add mat tmp
= tmp (mul mat vec)

number sz
vector_size tmp sz

number i

for i 0 (< i sz) 1 loop
	string s
	typeof s [tmp 0]
	if (== "vector" s) do_mat do_vec
		number sz2
		vector_size [tmp 0] sz2
		number j
		for j 0 (< j sz2) 1 loop2
			print "% " [tmp i j]
		:loop2
		print "\n"
	:do_mat
	print "% " [tmp i]
	:do_vec
:loop
print "\n"
