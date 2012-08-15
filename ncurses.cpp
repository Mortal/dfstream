#include "ncurses.h"
#include <ncurses.h>
#include <unistd.h>
#include <locale.h>
#include "cp437.h"
#include <limits>
#include <map>

struct ncurses::impl {
	std::vector<int> pairusage;
	std::map<int, short> colors;
	std::vector<std::vector<short> > tilecolors;

	impl() : pairusage(COLOR_PAIRS, 0) {
		pairusage.resize(COLOR_PAIRS, 0);
	}

	short use_pair(int r, int c, int color) {
		if (tilecolors.size() <= r) tilecolors.resize(r+1);
		if (tilecolors[r].size() <= c) tilecolors[r].resize(c+1);
		if (tilecolors[r][c]) down(tilecolors[r][c]);
		short p = this->get_pair(color);
		up(p);
		return p;
	}

	short get_pair(int color) {
		std::pair<std::map<int, short>::iterator, bool> i = colors.insert(std::make_pair(color, 0));
		if (!i.second) return i.first->second;
		short p = alloc_pair(color);
		i.first->second = p;
		return p;
	}

	short alloc_pair(int color) {
		int least = std::numeric_limits<int>::max();
		short leastp = 0;
		for (short i = 1; i < pairusage.size(); ++i) {
			if (pairusage[i] <= least) {
				least = pairusage[i];
				leastp = i;
				if (!least) break;
			}
		}
		init_pair(leastp, color & 0x7, (color & 0x70) >> 4);
		return leastp;
	}

	void up(short pair) {
		++pairusage[pair];
	}

	void down(short pair) {
		if (pairusage[pair]) --pairusage[pair];
	}
};

ncurses::ncurses() {
	setlocale(LC_ALL, "");
	initscr();
	start_color();
	pimpl = new ncurses::impl();
}

ncurses::~ncurses() {
	delete pimpl;
	endwin();
}

void ncurses::setbuf(const std::vector<tile_type> & buf, int row, int column, int width, int height) {
	int off = 0;
	for (int y = 0; y < height; ++y) {
		for (int x = 0; x < width; ++x) {
			color_type col = buf[off].color();
			move(row+y, column+x);
			attrset(((col & 0x8) ? A_BOLD : 0)
				   | COLOR_PAIR(pimpl->use_pair(row+y, column+x, col & 0x77)));
			addnwstr(cp437 + buf[off].tile_index(), 1);
			++off;
		}
	}
	refresh();
}

// vim:set ts=4 sts=4 sw=4 noet:
