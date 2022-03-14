GCC = gcc
.SUFFIXES: .o

all: master slave

slave: slave.o sharedMemory.o union.h
	$(GCC) slave.o sharedMemory.o -o slave

master: master.o sharedMemory.o union.h
	$(GCC) master.o sharedMemory.o -o master

.c.o:
	$(GCC) -c $<

clean:
	rm -rf *.o ./master ./slave ./logfile.*