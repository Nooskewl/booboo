number rounds
= rounds 100000000
* rounds 2
+ rounds 3

number x
= x 1
number pi
= pi 1.0
number a
number b

number i
= i 3
:next_loop
neg x
= a 1
= b i
/ a b
* a x
+ pi a
+ i 2
? i rounds
jl next_loop

* pi 4

print "%\n" pi
