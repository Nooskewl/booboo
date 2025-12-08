var cfg
= cfg (cfg_load "com.nooskewl.cfg")
var val exists
= val "default"
= exists (cfg_exists cfg "foobar")
if (== TRUE exists) read
	= val (cfg_get_string cfg "foobar")
:read
print "The secret value is: %\n" val
print "Enter a new secret value:\n"
= val (input)
cfg_set_string cfg "foobar" val
var success
= success (cfg_save cfg "com.nooskewl.cfg")
