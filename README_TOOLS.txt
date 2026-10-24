compress_dir can compress a data/ directory into a data.cpa file. Just enter the
data/ directory and run it, it will create ../data.cpa. BooBoo can launch a
data.cpa in a directory by pointing BooBoo to the directory of the data.cpa
file.

uncompress_cpa unpacks a CPA archive into a data/ directory.

prune_sprite_json keeps only the required info in Aseprite sprite exports

convert_model converts ascii .x models to binary .nsm models which are faster to
load and smaller

io_scene_goobliata_x - Blender 2.78 model import/exporter

play_mml is a player for MML audio files. It supports all the flags from shim5.json
as well as +loop.

Some of these tools need SDL3.dll and SDL3_ttf.dll and shim5.dll, located in the
root of the BooBoo installation.

mkpixelfont can get you started creating a pixel font. Using the 'events' example
as a template, you'd put a font.ttf in the font directory and run mkpixelfont to
create font.png. You need a file called glyphs.utf8 which has each character you
want in the font, and a file called info.ini which has two possible lines, size=
and wide= (for wide fonts.) If you specify a size of 10, it'll reserve 10x10 per
glyph or 20x10 if wide=1.
