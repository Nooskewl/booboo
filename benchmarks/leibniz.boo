var f
= f (file_open "rounds.txt" "r")
var s
= s (file_read f)
file_close f
var rounds
= rounds (+ 4 (* s 2))

var x
= x 1
var pi
= pi 1
var i

for i 3 (<= i rounds) 2 loop_end
	= x (neg x)
	= pi (+ pi (/ x i))
:loop_end

= pi (* pi 4)

print "%(3.16g)\n" pi
