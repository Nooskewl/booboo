g++ -shared -o libhsv.dll hsv.cpp -I../include ../dll/libBooBooCore.dll
g++ -shared -o libbooboojpeg.dll jpeg.cpp -I../include -I../include/SDL3 ../dll/libBooBooCore.dll ../dll/libshim5.dll libjpeg-9.dll
g++ -shared -o libbooboomp3.dll mpg123.cpp -I../include -I../include/SDL3 ../dll/libBooBooCore.dll ../dll/libshim5.dll ../dll/SDL3.dll libmpg123-0.dll
