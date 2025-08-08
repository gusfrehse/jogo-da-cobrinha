CFLAGS=-Wall -Wextra -O3 -g `sdl2-config --cflags`
LDLIBS=`sdl2-config --libs` -lGL

main : main.c
