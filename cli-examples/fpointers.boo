function foo
{
	print "FOO\n"
}

pointer p
= p (@ foo)
call `p

call bar foo

function bar bazoo
{
	call bazoo
}
