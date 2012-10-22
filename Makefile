CXX=clang++
CXXFLAGS=-O3 -Wall -Wextra -std=c++0x -static
all: patterntrainer compressor uncompress

clean:
	$(RM) patterntrainer compressor uncompress uncompress.o ncurses.o compressor.o patterntrainer.o

patterntrainer: patterntrainer.o
	$(CXX) $(CXXFLAGS) -o $@ $^

compressor: compressor.o
	$(CXX) $(CXXFLAGS) -o $@ $^

uncompress: uncompress.o ncurses.o
	$(CXX) $(CXXFLAGS) -o $@ $^ -lncursesw

compressor.o: compressor.cpp types.h dataequal.h

ncurses.o: ncurses.cpp ncurses.h cp437.h types.h

uncompress.o: uncompress.cpp ncurses.h types.h
