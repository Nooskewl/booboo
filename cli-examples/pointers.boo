number n
= n 10
pointer p
= p (@ n)
= `p 20
print "%\n" n
print "%\n" `p

map m
= [m "foo"] p
= `[m "foo"] 30
print "%\n" n
= [m "foo"] 40
print "%\n" [m "foo"]

= p (@ [m "foo"])
= `p 50
print "%\n" [m "foo"]
