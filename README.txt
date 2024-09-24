Copyright 2022-2024 ILLUMINATI NORTH

BooBoo is a mini language with a simple, but powerful game API.

To run the examples, run ./BooBoo <path/to/data/> or just run BooBoo
to get a launcher (point the launcher to where there is a data/
directory from a BooBoo game or a data.cpa from one.) Press F12 to
exit.

Your data/ directory can be compressed into data.cpa with compress_dir.
Just enter data/ and run it, and it will create ../data.cpa.

Note: the examples and demos came about as the language evolved, so a
lot of "old" style and syntax may be found in some of them.

--

Direct3D can be used instead of OpenGL by supplying -opengl on the
commandline (- disables and + enables.) However you will need the
shaders in hlsl to be placed in each data/gfx/shaders directory.
pack_shader.exe in tools can be used to pack HLSL shaders for use
with BooBoo. However, you need to install the DirectX 9 SDK and run
pack_shader from the included DirectX SDK Command Prompt because it
needs fxc.exe.

Please visit https://cmykilluminati.net for updates!
