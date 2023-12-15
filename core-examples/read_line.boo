number f
file_open f "file.txt" "r"

:next_loop
string s
file_read_line f s
? s ""
je done
string key
string value
number gotten
string_scan gotten "%=%" s key value
print "% are %\n" key value
goto next_loop

:done

file_close f
