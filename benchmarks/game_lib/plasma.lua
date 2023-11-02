TSZ = 16

palette = {}

for i = 0,84 do
	colour = {}
	p = math.floor((1.0 - (i / 85)) * 255)
	colour[0] = p
	colour[1] = math.floor((i / 85) * 255)
	colour[2] = 0
	palette[i] = colour
end

for i = 0,84 do
	colour = {}
	colour[0] = 0
	p = math.floor((1.0 - (i / 85)) * 255)
	colour[1] = p
	colour[2] = math.floor((i / 85) * 255)
	palette[i+85] = colour
end

for i = 0,85 do
	colour = {}
	colour[0] = math.floor((i / 85) * 255)
	colour[1] = 0
	p = math.floor((1.0 - (i / 85)) * 255)
	colour[2] = p
	palette[i+85+85] = colour
end


W = math.floor(640 / TSZ) + 1
H = math.floor(360 / TSZ) + 1

frame = {}

for y=0,H-1 do
	frame[y] = {}
	for x=0,W-1 do
		frame[y][x] = rand(0, 255)
	end
end

function divide(frame, w, h)
	new_w = w * 2
	new_h = h * 2

	v = {}

	for i=0,new_h-1 do
		v[i] = {}
		for j=0,new_w-1 do
			v[i][j] = 0
		end
	end

	for y=0,new_h-1 do
		for x=0,new_w-1 do
			r = 0
			g = 0
			b = 0
			valid = 0
			for yy=y-1,y+1 do
				for xx=x-1,x+1 do
					xx2 = math.floor(xx/2)
					yy2 = math.floor(yy/2)
					if (xx2 >= 0 and xx2 < w and yy2 >= 0 and yy2 < h) then
						valid = valid + 1
						row = frame[yy2]
						pal_index = row[xx2]
						r = r + palette[pal_index][0]
						g = g + palette[pal_index][1]
						b = b + palette[pal_index][2]
					end
				end
			end
			r = math.floor(r / valid)
			g = math.floor(g / valid)
			b = math.floor(b / valid)
			best_fit = find_closest(r, g, b)
			v[y][x] = best_fit
		end
	end

	return v
end

function find_closest(r, g, b)
	best_i = 0
	smallest_diff = 255
	for i=0,255 do
		rr = math.abs(palette[i][0] - r)
		gg = math.abs(palette[i][1] - g)
		bb = math.abs(palette[i][2] - b)
		rr = math.sqrt(rr*rr + gg*gg + bb*bb)
		if (rr < smallest_diff) then
			smallest_diff = rr
			best_i = i
		end
	end

	return best_i
end

frame = divide(frame, W, H)
W = W * 2
H = H * 2
frame = divide(frame, W, H)
W = W * 2
H = H * 2
--frame = divide(frame, W, H)
--W = W * 2
--H = H * 2
--frame = divide(frame, W, H)
--W = W * 2
--H = H * 2

ticks = 0

function draw()
	xsz = 640 / W
	ysz = 360 / H

	start_primitives()

	for y=0,H-1 do
		for x=0,W-1 do
			pal_index = frame[y][x]
			pal_index = (pal_index + ticks) % 256
			r = palette[pal_index][0]
			g = palette[pal_index][1]
			b = palette[pal_index][2]
			filled_rectangle(r, g, b, 255, r, g, b, 255, r, g, b, 255, r, g, b, 255, x*xsz, y*ysz, xsz, ysz)
		end
	end

	end_primitives()
	
	ticks = ticks + 1
end

function run()
end
