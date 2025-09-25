Copyright 2022-2025 Nooskewl

BooBoo is a mini language with a simple, but powerful game API.

To run the examples, run ./BooBoo <path/to/data/> or just run BooBoo
to get a launcher (point the launcher to where there is a data/
directory from a BooBoo game or a data.cpa from one.) Press F12 to
exit, F11 to toggle fullscreen.

Many of the games only read joystick input, but fudge some keys into
joystick events... so the keys are arrows and z/x.

Your data/ directory can be compressed into data.cpa with compress_dir.
Just enter data/ and run it, and it will create ../data.cpa.

--

Direct3D can be used instead of OpenGL by supplying -opengl on the
commandline (- disables and + enables.) However you will need the
shaders in hlsl to be placed in each data/gfx/shaders directory.
pack_shader.exe in tools can be used to pack HLSL shaders for use
with BooBoo. However, you need to install the DirectX 9 SDK and run
pack_shader from the included DirectX SDK Command Prompt because it
needs fxc.exe.

--

shim5.json can be put in any data/ directory or you can override
those by placing one in:

Windows:
	%USERPROFILE%\Saved Games\BooBoo


https://codeberg.org/nooskewl/shim5/src/branch/master/misc/io_scene_x_goobliata

is a model import/export for blender 2.73 which supports armatures/animations. Unfortunately I don't have one for recent Blender yet.

--

You can obfuscate code with the +obfuscate switch. Examples:

> ./BooBooCLI dragonslayer.boo +obfuscate
> ./BooBoo town +obfuscate > foo.boo

--

Please visit https://nooskewl.com for updates!
