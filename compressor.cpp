#include <cstring>
#include <cstdio>
#include <utility>
#include <array>
#include <vector>

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

template <typename pixel>
struct tiledata {
	unsigned char tile;
};

bool first = true;

template <int pixelsize>
bool readpixels(const int width, const int height) {
	typedef std::array<char, pixelsize> pixel;
	const int symbolcolumns = width/symbolwidth;
	const int symbolrows = height/symbolheight;
	const int bufsize = width*symbolheight;
	static std::vector<pixel> symbolrow;
	symbolrow.resize(bufsize);
	static std::vector<tiledata<pixel> > output;
	output.resize(symbolcolumns);
	if (first) {
		printf("%d\n%d\n", symbolcolumns, symbolrows);
		first = false;
	}
	for (int y = 0; y < symbolrows; ++y) {
		fread(&symbolrow[0], sizeof(pixel), bufsize, stdin);
		auto buf = symbolrow.begin();
		for (int x = 0; x < symbolcolumns; ++x) {
			deinterlacer<pixel> d(&*buf, symbolcolumns);
			auto c = binarydecode(d);
			output[x].tile = c.second;
			buf += symbolwidth;
		}
		fwrite(&output[0], sizeof(tiledata<pixel>), symbolcolumns, stdout);
	}
	return 1;
}

int main() {
	while (readframe());
	return 0;
}
// vim:set ts=4 sts=4 sw=4 noet:
