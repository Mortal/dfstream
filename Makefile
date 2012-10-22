CXX := clang++
CXXFLAGS := -O3 -Wall -Wextra -std=c++0x -static
EXECS := patterntrainer compressor uncompress
OBJS := patterntrainer.o compressor.o uncompress.o ncurses.o

LIBS_uncompress := -lncursesw
DEPS_uncompress := ncurses.o

all: $(EXECS)

clean:
	$(RM) $(EXECS) $(OBJS)

$(EXECS): %: %.o
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LIBS_$@)

uncompress: ncurses.o

compressor.o: compressor.cpp types.h dataequal.h

ncurses.o: ncurses.cpp ncurses.h cp437.h types.h

uncompress.o: uncompress.cpp ncurses.h types.h
