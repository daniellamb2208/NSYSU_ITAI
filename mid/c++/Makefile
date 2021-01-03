CC = g++
CPPFLAG = -Ofast -Os -std=c++2a -Wall -lpthread -latomic -g -fconcepts
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

interface: clean dep
	$(CC) $(CPPFLAG) -fPIC -shared driver/interfaces.cpp -o interfaces.so

clean:
	rm -rf *.o *.elf *.so output.txt