all: test_c_index test_skiplist2block  block_file_manager_test
LIBS = -L.. -lhistory -lrt -lmemp
INCS = -I../../util -I..
CFLAGS = -g -O3
test_c_index:test_c_index.o 
	gcc -o $@ $? $(LIBS)
test_skiplist2block: test_skiplist2block.o 
	g++ -o $@ $? $(LIBS)
block_file_manager_test: block_file_manager_test.o
	gcc -o $@ $? $(LIBS)
.c.o:
	gcc  $< -c -Wall $(CFLAGS) $(INCS) -std=gnu99
.cpp.o:
	g++  $< -c -Wall $(CFLAGS) $(INCS)
clean:
	rm -f test_c_index test_skiplist2block block_file_manager_test *.o
