#include <string>
#include "types.h"
#include <vector>

struct ncurses {
	struct impl;
	impl * pimpl;
	ncurses();
	~ncurses();
	void setbuf(const std::vector<tile_type> & buf, int row, int col, int width, int height);
};
// vim:set ts=4 sts=4 sw=4 noet:
