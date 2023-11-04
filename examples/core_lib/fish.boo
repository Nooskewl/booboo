map m

map m2
map m3

map m4

map_set m2 "foo" "bar"
map_set m2 "baz" "bazoo"

map_set m3 "number" 10
map_set m3 "string" "foobarbaz"

map_set m4 "turkey" "sandwich"
map_set m4 "boneless" "ribs"
map_set m4 "index" 1

map_set m2 "recipes" m4

map_set m "0" m2
map_set m "1" m3

vector v
vector_add v 0
vector_add v 1
vector_add v 3

print "%\n" (+ 10 [v (+ 1 [m "0" "recipes" "index"])])
