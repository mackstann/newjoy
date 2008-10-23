# Written by Nick Welch in the year 2008.  Author disclaims copyright.

newjoy: main.c
	gcc -o newjoy -std=gnu99 -pedantic -Wall -lm -lSDL -lX11 -lXtst main.c
