#include <cstring>
#include <cstdio>
#include <utility>
#include <array>

const int symbolwidth = 8; // pixel width of single symbol; must be a power of two
const int symbolheight = 12;
const int indexoffsetmask = symbolwidth-1;

template <typename pixel>
struct deinterlacer {
	const pixel * offset;
	const int symbolcolumns;
	inline deinterlacer(const pixel * offset, const int symbolcolumns)
		: offset(offset)
		, symbolcolumns(symbolcolumns)
	{
	}

	inline pixel getcol(const size_t i) const {
		return offset[(i & ~indexoffsetmask)*symbolcolumns + (i & indexoffsetmask)];
	}

	inline bool operator[](const size_t i) const {
		return getcol(7) != getcol(i);
	}
};

template <typename pixel>
std::pair<unsigned char, unsigned char> binarydecode(const deinterlacer<pixel> & input) {
#include "binary_decider.inl"
}

template <int pixelsize>
bool readpixels(const int width, const int height);

bool readframe() {
	int format, width, height, maxval;
	if (4 != fscanf(stdin, "P%d %d %d %d\n", &format, &width, &height, &maxval)) {
		if (feof(stdin)) return false;
		printf("Couldn't read header\n");
		return false;
	}
	if (maxval > 255) {
		printf("Maxval too large: %d\n", maxval);
		return false;
	}
	if (format == 5) return readpixels<1>(width, height);
	if (format == 6) return readpixels<3>(width, height);
	printf("Unknown format %d\n", format);
	return false;
}

template <int pixelsize>
bool readpixels(const int width, const int height) {
	typedef std::array<char, pixelsize> pixel;
	const int symbolcolumns = width/symbolwidth;
	const int symbolrows = height/symbolheight;
	const int bufsize = width*symbolheight;
	pixel * symbolrow = new pixel[bufsize];
	for (int y = 0; y < symbolrows; ++y) {
		fread(symbolrow, sizeof(pixel), bufsize, stdin);
		for (pixel * buf = symbolrow; buf != (symbolrow+width); buf += symbolwidth) {
			deinterlacer<pixel> d(buf, symbolcolumns);
			auto c = binarydecode(d);
			//pixel fg = d.getcol(c.first);
			putchar(c.second);
		}
	}
	return 1;
}

int main() {
	while (readframe());
	return 0;
}
// vim:set ts=4 sts=4 sw=4 noet:
