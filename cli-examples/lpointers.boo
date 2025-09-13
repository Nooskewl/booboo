pointer p
= p (@ bar)
goto `p

:foo
print "FOO\n"
goto done
:bar
print "BAR\n"
goto done
:baz
print "BAZ\n"
goto done

:done
