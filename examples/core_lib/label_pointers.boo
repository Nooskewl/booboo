pointer l
string value
print "Print 2? (y/n)\n"
input value

call_result l set_label value

goto l

function set_label value
{
	pointer p
	? value "y"
	je two
	address p label1
	return p
:two
	address p label2
	return p
}

:label1
print "1\n"
exit 0

:label2
print "2\n"
exit 0
