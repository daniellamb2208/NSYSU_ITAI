CC = g++
CPPFLAG = -Og -Os -std=c++17 -Wall

all: clean dep 
	$(CC) $(CPPFLAG) *.o main.c -o main.elf

dep:
	$(CC) $(CPPFLAG) -c map.cpp
	$(CC) $(CPPFLAG) -c ant.cpp

plot:
	for x in *.elf ; do ( ./$$x &); done
	gnuplot live.gp

clean:
	rm -rf *.o *.elf output.txt