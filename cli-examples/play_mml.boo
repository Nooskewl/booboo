var args
= args (get_args)
if (< (vector_size args) 3) usage
	print "Usage: BooBooCLI play_mml.boo <mml> [+loop]\n"
	exit 0
:usage

var mml
= mml (mml_load [args 2] TRUE)

var loop
= loop FALSE
if (> (vector_size args) 3) check
	if (== "+loop" [args 3]) ok
		= loop TRUE
	:ok
:check

var done
= done FALSE
= VOID (mml_play mml 1.0 loop 0.0 callback)

:top
if (== done TRUE) out
	goto fini
:out
delay 10
goto top
:fini

function callback inst
{
	= done TRUE
}
