x = 0

function draw_at(r, g, b, x)
	f = x / 640 * 3.14159 * 2
	--f = math.sin(f)
	f = f * 90 + 180
end

function draw()
	xx = 0

	for xx=0,640,2 do
		draw_at(0, 255, 0, xx)
	end

	tmp = x % 640
	draw_at(128, 255, 128, tmp)
end

function run()
	x = x + 5
end

for i=0,100000 do
	draw()
	run()
end
