BooBoo is a mini high-level assembly-like language. There is a library
and command line interpreter that are standard C++, and the game API and
game launcher that uses SDL2 and some of my game engine libraries.

BooBooGame is the game launcher and BooBoo is the one without the game
library syntax.

To run the examples, run ./BooBoo <path/to/file.boo> (or use
./BooBooGame to run game library examples)

If you wish to distribute the binary (or your own compiled binaries) as
a standalone app (with your main.boo,) you are allowed to do that.

Your data/ directory can be compressed into data.cpa with compress_dir
from shim4. Just enter data/ and run it, and it will create ../data.cpa.
The shaders you'll find in the examples' data/ directories are part of
shim4 and are all or partly required to make BooBooGame work.
