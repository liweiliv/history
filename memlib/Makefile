libmemp.so: MempRing.o
	g++ -o $@ MempRing.o -shared
MempRing.o: MempRing.cpp MempRing.h
	g++ -c MempRing.cpp -I .. -Wall -fPIC  -O3 -g
clean:
	rm -f MempRing.o
	rm -f libmemp.so
