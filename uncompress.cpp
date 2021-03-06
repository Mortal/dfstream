#include <cstdlib>
#include <cstring>
#include <sstream>
#include <unistd.h>
#include <iostream>
#include <vector>
#include "ncurses.h"
#include "types.h"

int canvaswidth;
int canvasheight;
int outputcol;
int outputrow;
int outputwidth;
int outputheight;
const int maxsize = 4096;

inline bool readheader() {
	if (!std::cin.ignore(4)) // ignore chunk size
		return false;
	if (!(std::cin >> canvaswidth >> canvasheight >> outputcol >> outputrow >> outputwidth >> outputheight))
		return false;
	std::string s;
	std::getline(std::cin, s);
	return true;
}

const char * tr(int & var) {
	if (&var == &canvaswidth) return "canvaswidth";
	if (&var == &canvasheight) return "canvasheight";
	if (&var == &outputcol) return "outputcol";
	if (&var == &outputrow) return "outputrow";
	if (&var == &outputwidth) return "outputwidth";
	if (&var == &outputheight) return "outputheight";
	return "?";
}

void ensure_complain(int & var, int min, int max) {
	std::cerr << "Invalid input; " << tr(var) << " = " << var << ", expected range [" << min << ", " << max << "]" << std::endl;
	std::abort();
}

inline void ensure(int & var, int min, int max) {
	if (var < min || var > max) {
		ensure_complain(var, min, max);
	}
}

int main(int argc, char ** argv) {
	ncurses sc;
	int skip = 0;
	if (argc >= 2) skip = atoi(argv[1]);

	int skipleft = 0;
	std::vector<tile_type> frame;
	int prevwidth = 0;
	int prevheight = 0;
	while (readheader()) {
		ensure(canvaswidth, 0, 4096);
		ensure(canvasheight, 0, 4096);
		if (prevwidth != canvaswidth || prevheight != canvasheight) {
			frame.clear();
			frame.resize(canvaswidth*canvasheight);
			prevwidth = canvaswidth;
			prevheight = canvasheight;
		}
		ensure(outputcol, 0, canvaswidth);
		ensure(outputrow, 0, canvasheight);
		ensure(outputwidth, 0, canvaswidth-outputcol);
		ensure(outputheight, 0, canvasheight-outputrow);

		if (outputwidth == 0 || outputheight == 0) {
			// do nothing!
		} else if (outputcol == 0 && outputwidth == canvaswidth) {
			std::cin.read(reinterpret_cast<char*>(&*(frame.begin()+(outputrow*canvaswidth))),
						  sizeof(tile_type)*outputheight*outputwidth);
		} else {
			std::vector<tile_type> input(outputwidth*outputheight);
			std::cin.read(reinterpret_cast<char*>(&input[0]), sizeof(tile_type)*outputheight*outputwidth);
			int inputidx = 0;
			int idx = canvaswidth * outputrow + outputcol;
			for (int r = 0; r < outputheight; ++r) {
				std::copy(input.begin()+inputidx, input.begin()+(inputidx+outputwidth), frame.begin()+idx);
				inputidx += outputwidth;
				idx += canvaswidth;
			}
		}
		if (!skipleft--) {
			skipleft = skip;
			sc.setbuf(frame, 0, 0, canvaswidth, canvasheight);
		}
	}
	return 0;
}
// vim:set ts=4 sts=4 sw=4 noet:
