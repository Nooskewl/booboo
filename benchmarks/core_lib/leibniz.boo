number f
file_open f "rounds.txt" "r"
string s
file_read f s
file_close f
number rounds
= rounds (+ 3 (* s 2))

number x
= x 1
number pi
= pi 1
number i

for i 3 rounds 2 loop_end
neg x
+ pi (/ x i)
:loop_end

* pi 4

print "%(3.16g)\n" pi
