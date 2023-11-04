number rounds
= rounds (+ 3 (* 100000000 2))

number x
= x 1
number pi
= pi 1.0

number i
= i 3
:next_loop
neg x
+ pi (* x (/ 1 i))
+ i 2
? i rounds
jl next_loop

* pi 4

print "%\n" pi
