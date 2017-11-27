GL_LIB = -lglfw3 -lGL -lGLEW
WINDOW_LIB = -lX11 -lXrandr -l Xxf86vm -lXcursor -lXi -lXinerama
COMMON_LIB = -lpthread -ldl
LOCAL_LIB = -L../util/memlib -lmemp
CFLAGS = -g
CINCS =  -I../util -I../util/memlib
MAP =  world.o
World:main.o $(MAP)
	g++ -o World $^  $(GL_LIB) $(WINDOW_LIB) $(COMMON_LIB) $(LOCAL_LIB)
.cpp.o:
	g++ -c $<  $(CINCS) $(CFLAGS) --std=c++11
clean:
	rm -f *.o
	rm -f world
