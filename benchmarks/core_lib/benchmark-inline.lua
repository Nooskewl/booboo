x = 0

for i=0,100000 do
	xx = 0

	for xx=0,640,2 do
		f = x / 640 * 3.14159 * 2
		--f = math.sin(f)
		f = f * 90 + 180
	end

	tmp = x % 640
	f = x / 640 * 3.14159 * 2
	--f = math.sin(f)
	f = f * 90 + 180
	x = x + 5
end
