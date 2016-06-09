CC = gcc

CFLAGS := -Wall -Wextra -pedantic -g -Wwrite-strings
GRAPHICS_FLAGS  := -lGLEW -lglfw3 -lGL -lX11 -lXrandr -lXi -lXxf86vm -lm -ldl -lXinerama -lXcursor -lrt -lpthread
SOUND_FLAGS :=  -lportaudio -lasound -ljack

all:
	$(CC) pong.c -o pong $(GRAPHICS_FLAGS) $(CFLAGS) $(SOUND_FLAGS) libportaudio.a
