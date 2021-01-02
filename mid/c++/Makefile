CC = g++
CPPFLAG = -Og -Os -std=c++17 -Wall -lpthread -latomic -g
LD = /usr/bin/ld

all: clean dep main
	$(CC) *.o -lpthread -latomic -lstdc++ -o main.elf -fsanitize=address

main: 
	$(CC) $(CPPFLAG) -c driver/main.cpp

dep:
	$(CC) $(CPPFLAG) -c map.cpp
	$(CC) $(CPPFLAG) -c ant.cpp

test: clean dep
	$(CC) $(CPPFLAG) -c driver/testMap.cpp
	$(CC) *.o -lpthread -latomic -lstdc++ -o testMap.elf -fsanitize=address

clean:
	rm -rf *.o *.elf output.txt