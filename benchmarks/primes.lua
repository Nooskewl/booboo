primes = {}

for i = 0, 15000 do
	factors = {}

	for j = 1, i do
		if i % j == 0 then
			table.insert(factors, j)
		end
	end

	if #factors == 2 then
		table.insert(primes, i)
	end
end
