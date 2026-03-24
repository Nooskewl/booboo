g++ -shared -o libhsv.dll hsv.cpp -I../include ../dll/libBooBooCore.dll
g++ -shared -o libbooboojpeg.dll jpeg.cpp -I../include -I../include/SDL3 ../dll/libBooBooCore.dll ../dll/libshim5.dll libjpeg-9.dll
