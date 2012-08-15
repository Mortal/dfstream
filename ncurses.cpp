#include "ncurses.h"
#include <ncurses.h>
#include <unistd.h>
#include <locale.h>

ncurses::ncurses() {
	setlocale(LC_ALL, "");
	initscr();
}

ncurses::~ncurses() {
	endwin();
}

void ncurses::setbuf(const std::wstring & buf) {
	mvaddnwstr(0,0, buf.c_str(), buf.size());
	refresh();
}

void ncurses::setbuf(const std::wstring & buf, int row, int col, int width, int height) {
	const wchar_t * cbuf = buf.c_str();
	int off = 0;
	for (int y = 0; y < height; ++y) {
		mvaddnwstr(row+y, col, cbuf+off, width);
		off += width;
	}
	refresh();
}

// vim:set ts=4 sts=4 sw=4 noet:
