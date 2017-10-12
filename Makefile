CFLAG = -g -O3  -Wall -fPIC 
INCS = -I util
LIBS = -L util/memlib -lmemp
OBJS = c_index.o c_block.o  arena.o file_opt.o skiplist2index.o block_file_manager.o page_pool.o
libhistory.so :$(OBJS)
	g++ -o $@ $(OBJS) $(LIBS)  -shared 
.cpp.o:
	g++ -c $(CFLAG) $(INCS) $<
.cc.o:
	g++ -c $(CFLAG) $(INCS) $<
.c.o:
	gcc -c $(CFLAG) $(INCS) $< --std=gnu99
clean :
	rm -f *.o libhistory.so 
