function addr ~v
{
	pointer p
	address p v
	return p
}
number n
= n 1
= `(addr n) 2
print "%\n" n
