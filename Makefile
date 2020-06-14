CXX = g++
CFLAGS = -Wall -g -std=c++11
LIB = capstone

.PHONY: all clean

all: countme capstone

countme: countme.o
	$(CXX) $< -o $@

capstone: capstone.o
	$(CXX) $< -l$(LIB) -o $@


countme.o: countme.cpp
	$(CXX) -c $< $(CFLAGS) -o $@

capstone.o: capstone.cpp
	$(CXX) -c $< $(CFLAGS) -o $@

clean:
	rm -rf *.o countme capstone