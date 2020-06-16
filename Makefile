CXX = g++
CFLAGS = -Wall -g -std=c++11
LIB = capstone

.PHONY: all clean

all: countme capstone

countme: countme.o
	$(CXX) $< -o $@

capstone: capstone.o
	$(CXX) $< -l$(LIB) -o $@

count_syscall: count_syscall.o
	$(CXX) $< -o $@

no_more_traps: no_more_traps.o
	$(CXX) $< -o $@

countme.o: countme.cpp
	$(CXX) -c $< $(CFLAGS) -o $@

capstone.o: capstone.cpp
	$(CXX) -c $< $(CFLAGS) -o $@

count_syscall.o: count_syscall.cpp
	$(CXX) -c $< $(CFLAGS) -o $@

no_more_traps.o: no_more_traps.cpp
	$(CXX) -c $< $(CFLAGS) -o $@

clean:
	rm -rf *.o countme capstone count_syscall no_more_traps