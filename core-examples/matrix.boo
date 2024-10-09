vector m
= m (midentity 4)

call printm m

function printm m
{
	number nc nr
	vector_size m nc
	vector_size [m 0] nr

	number c r
	for c 0 (< c nc) 1 next_col
		for r 0 (< r nr) 1 next_row
			print "%\t" [m c r]
		:next_row
		print "\n"
	:next_col
}
