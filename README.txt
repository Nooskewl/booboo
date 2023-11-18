BooBoo is a mini high-level assembly-like language. There is a library and
command line interpreter that are standard C++, and the game API and game
launcher that are an addon.

To run the examples, run ./BooBoo <path/to/file.boo> (or use ./BooBooGame
to run game library examples)

If you wish to distribute BooBooGame or BooBoo as a standalone app (with your
main.boo and other files,) you can, but please be thoughtful and donate if you find it useful.

Your data/ directory can be compressed into data.cpa with compress_dir.
Just enter data/ and run it, and it will create ../data.cpa. The shaders
you'll find in the examples' data/ directories are all or partly required
to make BooBooGame work.
