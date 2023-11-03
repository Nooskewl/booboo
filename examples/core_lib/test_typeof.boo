pointer p
number n
= n 100
address p n
string s
typeof s p
print "p=%\n" s
typeof s n
print "n=%\n" s

typeof s label
print "label=%\n" s

typeof s foo
print "foo=%\n" s

:label

function foo
{
}

vector v
vector tmp
vector_add tmp 0
vector_add tmp 1
vector_add tmp 2
vector_add v tmp
vector_clear tmp
vector_add tmp 3
vector_add tmp 4
vector_add tmp "foobar"
vector_add v tmp

typeof s v 0 0
print "vector[0][0]: %\n" s
typeof s v 1 2
print "vector[1][2]: %\n" s
typeof s tmp 0
print "tmp[0]: %\n" s

map m
map tmpm
map_set tmpm "foo" "bar"
map_set tmpm "baz" "bazoo"
map_set m "first" tmpm
map_clear tmpm
map_set tmpm "foo" 1
map_set tmpm "baz" 2
map_set m "second" tmpm

typeof s m "first" "foo"
print "m[first][foo]: %\n" s
typeof s m "second" "foo"
print "m[second][foo]: %\n" s
