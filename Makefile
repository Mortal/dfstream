CXX := clang++
CXXFLAGS := -O3 -Wall -Wextra -std=c++0x -static
EXECS := patterntrainer compressor uncompress
OBJS := patterntrainer.o compressor.o uncompress.o ncurses.o

LIBS_uncompress := -lncursesw
DEPS_uncompress := ncurses.o

all: $(EXECS)

clean:
	$(RM) $(EXECS) $(OBJS) $(OBJS:.o=.d)

$(EXECS): %: %.o
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LIBS_$@)

uncompress: ncurses.o

%.d: %.cpp
	$(SHELL) -ec '$(CXX) -MM $(CPPFLAGS) $< \
		| sed "s/$*\\.o/& $@/g" > $@'

include $(OBJS:.o=.d)
