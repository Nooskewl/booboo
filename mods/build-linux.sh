g++ -fPIC -shared -o libhsv.so -I../include -L../so -lBooBooCore hsv.cpp
g++ -fPIC -shared -o libbooboojpeg.so -I../include -L../so -lBooBooCore -lshim5 jpeg.cpp -ljpeg -g
