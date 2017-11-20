GL_LIB = -lglfw3 -lGL -lGLEW
WINDOW_LIB = -lX11 -lXrandr -l Xxf86vm -lXcursor -lXi -lXinerama
COMMON_LIB = -lpthread -ldl
CFLAGS = -g
MAP = 
world:main.o $(MAP)
	g++ -o world main.o $(GL_LIB) $(WINDOW_LIB) $(COMMON_LIB)
.cpp.o:
	g++ -c $< $(CFLAGS)
clean:
	rm -f *.o
	rm -f world
