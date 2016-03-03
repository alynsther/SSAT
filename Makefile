CC = /usr/bin/g++
CFLAGS = -Wall -std=c++11

ssat:
	$(CC) $(CFLAGS) -o ssat ssat.cc

ssat-generator:
	$(CC) $(CFLAGS) -o ssat-generator ssat-generator.cc

cleanssat:
	$(RM) -f ssat

cleanssatgen:
	$(RM) -f ssat-generator