function fib n
{
	if (<= n 1) return_it
		return n
	:return_it
	return (+ (fib (- n 1)) (fib (- n 2)))
}

var i
for i 1 (< i 36) 1 loop
	print "% %\n" i (fib i)
:loop
