number f
= f (file_open "out.txt" "w")
file_print f "How much are burgers? %?\n" 1.99
file_print f "Burgers are %!\n" 2.99
file_close f
