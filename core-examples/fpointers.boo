function foo
{
	print "FOO\n"
}

pointer p
address p foo
call `p

call bar foo

function bar bazoo
{
	call bazoo
}
