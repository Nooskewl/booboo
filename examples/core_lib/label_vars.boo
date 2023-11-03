label l
string value
print "Print 2? (y/n)\n"
input value

call_result l set_label value

goto l

function set_label value
{
	? value "y"
	je two
	return label1
:two
	return label2
}

:label1
print "1\n"
exit 0

:label2
print "2\n"
exit 0
