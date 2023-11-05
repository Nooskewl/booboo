string s
= s "23:59:00"
number gotten
number hours
number minutes
number seconds
string_scan gotten "%:%:%" s hours minutes seconds
print "Got % results\n" gotten
print "% hours, % minutes and % seconds\n" hours minutes seconds

= s "apples oranges   	bananas"
string a
string b
string c
string_scan gotten " % % % " s a b c
print "Got % results\n" gotten
print "% % %\n" a b c
