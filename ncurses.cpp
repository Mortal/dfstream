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

// vim:set ts=4 sts=4 sw=4 noet:
