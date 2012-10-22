CXX=clang++
CXXFLAGS=-O3 -Wall -Wextra -std=c++0x -static
all: patterntrainer compressor uncompress

clean:
	$(RM) patterntrainer compressor uncompress uncompress.o ncurses.o

uncompress: uncompress.o ncurses.o
	$(CXX) $(CXXFLAGS) -o $@ $^ -lncursesw
