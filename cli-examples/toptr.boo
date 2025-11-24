number f
= f (file_open "word.txt" "r")
string s
= s (file_read f)
file_close f
pointer p
= p (toptr s)
if (!= p NULL) go nope
	call `p
:go
	print "NOPE!\n"
:nope
function foo
{
	print "foo!\n"
}
function bar
{
	print "bar!\n"
}
function baz
{
	print "baz!"
}
