number f
file_open f "rounds.txt" "r"
string s
file_read f s
file_close f
number rounds
= rounds s

number x
= x 1
number pi
= pi 1
number i
= i 3

number l
= l 0
for l rounds loop_end
neg x
+ pi (/ x i)
+ i 2
:loop_end

* pi 4

print "%(3.16g)\n" pi
