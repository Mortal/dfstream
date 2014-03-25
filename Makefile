CXX := clang++
CXXFLAGS := -O3 -Wall -Wextra -std=c++0x -static
EXECS := patterntrainer compressor uncompress
OBJS := patterntrainer.o compressor.o uncompress.o ncurses_simple.o

LIBS_uncompress :=

all: $(EXECS)

clean:
	$(RM) $(EXECS) $(OBJS) $(OBJS:.o=.d)

$(EXECS): %: %.o
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LIBS_$@)

uncompress: ncurses_simple.o

%.d: %.cpp
	$(SHELL) -ec '$(CXX) -MM $(CPPFLAGS) $< \
		| sed "s/$*\\.o/& $@/g" > $@'

include $(OBJS:.o=.d)
