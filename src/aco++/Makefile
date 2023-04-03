# Makefile for ACO++

CPP= g++
CFLAGS= -O3 -w -std=c++11
EXE=acothop

all: thop.o utilities.o ants.o inout.o unix_timer.o ls.o parse.o acothop.o hill_climbing.o
	$(CPP) $(CFLAGS) -g -o $(EXE) thop.o utilities.o ants.o inout.o unix_timer.o ls.o parse.o acothop.o hill_climbing.o

thop.o: thop.c thop.h
	$(CPP) $(CFLAGS) -g -c thop.c

utilities.o: utilities.c utilities.h
	$(CPP) $(CFLAGS) -g -c utilities.c

ants.o: ants.c ants.h
	$(CPP) $(CFLAGS) -g -c ants.c

inout.o: inout.c inout.h
	$(CPP) $(CFLAGS) -g -c inout.c

unix_timer.o: unix_timer.c timer.h
	$(CPP) $(CFLAGS) -g -c unix_timer.c

ls.o: ls.c ls.h
	$(CPP) $(CFLAGS) -g -c ls.c

parse.o: parse.c parse.h
	$(CPP) $(CFLAGS) -g -c parse.c

hill_climbing.o: hill_climbing.c hill_climbing.h
	$(CPP) $(CFLAGS) -g -c hill_climbing.c

acothop.o: ants.h utilities.h inout.h thop.h timer.h ls.h acothop.c hill_climbing.c
	$(CPP) $(CFLAGS) -g -c acothop.c


clean:
	rm -f $(EXE) *.o
