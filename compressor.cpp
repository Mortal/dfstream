#include <cstring>
#include <cstdio>

unsigned char lineardecode(const char * input) {
#include "linear_decider.inl"
    return 255;
}

const int symbolwidth = 8; // pixel width of single symbol; must be a power of two
const int symbolheight = 12;
const int indexoffsetmask = symbolwidth-1;

struct deinterlacer {
	const char * offset;
	const int symbolcolumns;
	inline deinterlacer(const char * offset, const int symbolcolumns)
		: offset(offset)
		, symbolcolumns(symbolcolumns)
	{
	}

	inline char operator[](const size_t i) const {
		return offset[7] != offset[(i & ~indexoffsetmask)*symbolcolumns + (i & indexoffsetmask)];
	}
};

unsigned char binarydecode(const deinterlacer & input) {
#include "binary_decider.inl"
}

inline int translate(const int index, const int symbolcolumns) {
    return (index & indexoffsetmask)
        + symbolwidth*(index/(symbolcolumns*symbolwidth))
        + (symbolwidth*symbolheight)*((index % (symbolcolumns*symbolwidth))/symbolwidth);
}

int readframe() {
    int format, width, height, what;
    if (4 != fscanf(stdin, "P%d %d %d %d\n", &format, &width, &height, &what)) {
        if (feof(stdin)) return 0;
        printf("Couldn't read header\n");
        return 0;
    }
    if (format != 5) {
        printf("Unknown format %d\n", format);
        return 0;
    }
    const int symbolcolumns = width/symbolwidth;
    const int symbolrows = height/symbolheight;
    const int bufsize = width*symbolheight;
    char * symbolrow = new char[bufsize];
    for (int y = 0; y < symbolrows; ++y) {
        fread(symbolrow, sizeof(char), bufsize, stdin);
        for (char * buf = symbolrow; buf != (symbolrow+width); buf += symbolwidth) {
            putchar(binarydecode(deinterlacer(buf, symbolcolumns)));
        }
    }
    return 1;
}

int main() {
	while (readframe());
	return 0;
}
// vim:set ts=4 sts=4 sw=4:
