map m
map tmpm
map_set tmpm "foo" "bar"
map_set tmpm "baz" "bazoo"
map_set m "first" tmpm
map_clear tmpm
map_set tmpm "foo" 1
map_set tmpm "baz" 2
map_set m "second" tmpm

vector v
map_keys m v

number sz
vector_size v sz

number i
= i 0
:next_loop
string key
vector_get v key i
print "m[%] exists\n" key
+ i 1
? i sz
jl next_loop

vector v
map m2
map_get m m2 "first"
map_keys m2 v

number sz
vector_size v sz

number i
= i 0
:next_loop2
string key
vector_get v key i
print "m2[%] exists\n" key
+ i 1
? i sz
jl next_loop2
