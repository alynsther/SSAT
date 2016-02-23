CC = /usr/bin/g++
CFLAGS = -Wall -std=c++11

ssat:
	$(CC) $(CFLAGS) -o ssat ssat.cc

clean:
	$(RM) -rm -f ssat