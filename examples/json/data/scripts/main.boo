number json
json_load json "colours.json"

function draw
{
	number r1 g1 b1 r2 g2 b2 r3 g3 b3 r4 g4 b4
	json_as_number json r1 "topleft>r"
	json_as_number json g1 "topleft>g"
	json_as_number json b1 "topleft>b"
	json_as_number json r2 "topright>r"
	json_as_number json g2 "topright>g"
	json_as_number json b2 "topright>b"
	json_as_number json r3 "bottomright>r"
	json_as_number json g3 "bottomright>g"
	json_as_number json b3 "bottomright>b"
	json_as_number json r4 "bottomleft>[0]"
	json_as_number json g4 "bottomleft>[1]"
	json_as_number json b4 "bottomleft>[2]"

	filled_rectangle r1 g1 b1 255 r2 g2 b2 255 r3 g3 b3 255 r4 g4 b4 255 0 0 640 360
}
