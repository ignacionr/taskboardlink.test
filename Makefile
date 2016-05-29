all: srv

srv: srv.o mongoose.o
	g++ srv.o mongoose.o -ljpeg -lm -Wall -Wextra -std=c++11 -pedantic -O2 -mtune=generic -o srv

srv.o: srv.cpp
	g++ srv.cpp -c -Wall -Wextra -std=c++11 -pedantic -Dcimg_display=0 -O2 -mtune=generic -Dcimg_use_jpeg

mongoose.o: mongoose.c
	gcc mongoose.c -c

