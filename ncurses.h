#include <string>
struct ncurses {
	ncurses();
	~ncurses();
	void setbuf(const std::wstring & buf);
	void setbuf(const std::wstring & buf, int row, int col, int width, int height);
};
// vim:set ts=4 sts=4 sw=4 noet:
