CXX=clang++
CXXFLAGS=-O3 -Wall -Wextra -std=c++0x -static
all: patterntrainer compressor uncompress

uncompress: uncompress.o ncurses.o
	$(CXX) $(CXXFLAGS) -o $@ $^ -lncursesw
