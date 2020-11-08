CC = g++
CPPFLAG = -Og -Os -std=c++2a -Wall -lpthread -latomic -g
LD = /usr/bin/ld

all: clean dep 
	$(CC) *.o -lpthread -latomic -o main.elf

dep:
	$(CC) $(CPPFLAG) -c map.cpp
	$(CC) $(CPPFLAG) -c ant.cpp
	$(CC) $(CPPFLAG) -c main.cpp

plot:
	for x in *.elf ; do ( ./$$x &); done
	gnuplot live.gp

clean:
	rm -rf *.o *.elf output.txt