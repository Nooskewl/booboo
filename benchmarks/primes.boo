var primes i

for i 0 (< i 15000) 1 loop1
	var factors j
	for j 1 (< j (+ i 1)) 1 loop2
		if (== (% i j) 0) add
			vector_add factors j
		:add
	:loop2

	if (== (vector_size factors) 2) prime
		vector_add primes prime
	:prime
:loop1
