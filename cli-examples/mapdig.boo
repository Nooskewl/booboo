var m m2 m3 m4

= [m2 "foo"] "bar"
= [m2 "baz"] "bazoo"

= [m3 "number"] 10
= [m3 "string"] "foobarbaz"

= [m4 "turkey"] "sandwich"
= [m4 "boneless"] "ribs"

= [m2 "recipes"] m4

= [m "0"] m2
= [m "1"] m3

var recipe
= recipe [m "0" "recipes" "boneless"]

print "We're having %!\n" recipe
