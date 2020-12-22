CC = g++
CPPFLAG = -Og -Os -std=c++2a -Wall -lpthread -latomic -g
LD = /usr/bin/ld

all: clean dep 
	$(CC) *.o -lpthread -latomic -lstdc++ -o main.elf -fsanitize=address

dep:
	$(CC) $(CPPFLAG) -c map.cpp
	$(CC) $(CPPFLAG) -c ant.cpp
	$(CC) $(CPPFLAG) -c driver/main.cpp

plot:
	for x in *.elf ; do ( ./$$x &); done
	gnuplot live.gp

clean:
	rm -rf *.o *.elf output.txt