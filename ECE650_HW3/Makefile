CC=g++
CFLAGS=-std=c++11 -pedantic -Werror -Wall
all: ringmaster player
ringmaster: ringmaster.cpp potato.h
	$(CC) $(CFLAGS) -o $@ ringmaster.cpp

player: player.cpp potato.h
	$(CC) $(CFLAGS) -o $@ player.cpp

clean:
	rm -rf ringmaster player *.o *~
