CXX=clang++
CXXFLAGS=-O3 -Wall -Wextra -std=c++0x
all: patterntrainer compressor uncompress

uncompress: uncompress.o ncurses.o
	$(CXX) $(CXXFLAGS) -lncursesw -o $@ $^
