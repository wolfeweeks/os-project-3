GCC = gcc
.SUFFIXES: .o

all: master slave

slave: slave.o union.h
	$(GCC) slave.o -o slave

master: master.o union.h
	$(GCC) master.o -o master

.c.o:
	$(GCC) -c $<

clean:
	rm -rf *.o ./master ./slave ./logfile.*