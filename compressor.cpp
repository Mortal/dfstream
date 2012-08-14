#include <cstring>
#include <cstdio>
#include <utility>
#include <vector>
#include "dataequal.h"

const int symbolwidth = 8; // pixel width of single symbol; must be a power of two
const int symbolheight = 12;

template <int pixelsize>
struct pixel_type {
	unsigned char data[pixelsize];
	inline bool operator==(const pixel_type & other) const {
		return dataequal<unsigned char, pixelsize, pixelsize>::eq(data, other.data);
	}
	inline bool operator!=(const pixel_type & other) const {
		return !(*this == other);
	}
};

struct pixmap_header {
	int format, width, height, maxval;
	bool good;
	bool eof;

	pixmap_header() : good(false), eof(false) {}

	int symbolrows() const { return height/symbolheight; }
	int symbolcolumns() const { return width/symbolwidth; }

	bool operator==(const pixmap_header & other) const {
		if (eof != other.eof) return false;
		if (good != other.good) return false;
		if (!good) return true;
		return format == other.format
			&& width == other.width
			&& height == other.height
			&& maxval == other.maxval;
	}

	bool operator!=(const pixmap_header & other) const {
		return !(*this == other);
	}

	static pixmap_header read() {
		pixmap_header res;
		if (4 != fscanf(stdin, "P%d %d %d %d\n", &res.format, &res.width, &res.height, &res.maxval)) {
			if (feof(stdin)) {
				res.eof = true;
				return res;
			}
			printf("Couldn't read header\n");
			return res;
		}
		if (res.maxval > 255) {
			printf("Maxval too large: %d\n", res.maxval);
			return res;
		}
		res.good = true;
		return res;
	}
};

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
		const int indexoffsetmask = symbolwidth-1; // assume symbolwidth is a power of two
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

struct stdout_writer {
	typedef unsigned char tile_type;
	pixmap_header header;
	std::vector<tile_type> output;
	typename std::vector<tile_type>::iterator i;

	stdout_writer(const pixmap_header & header)
		: header(header)
	{
	}

	void begin_frame() {
		output.resize(header.symbolcolumns() * header.symbolrows());
		i = output.begin();
	}

	void write_row(const std::vector<tile_type> & row) {
		i = std::copy(row.begin(), row.end(), i);
	}

	void end_frame() {
		const int canvaswidth = header.symbolcolumns();
		const int canvasheight = header.symbolrows();
		const int outputcol = 0;
		const int outputrow = 0;
		const int outputwidth = canvaswidth;
		const int outputheight = canvasheight;
		printf("%d %d %d %d %d %d\n",
			   canvaswidth, canvasheight,
			   outputcol, outputrow,
			   outputwidth, outputheight);
		fwrite(&output[0], sizeof(tile_type), outputwidth*outputheight, stdout);
		fflush(stdout);
	}
};

template <int pixelsize>
struct stdin_reader {
	typedef pixel_type<pixelsize> pixel;
	typedef stdout_writer::tile_type tile_type;
	pixmap_header header;
	const int width;
	const int height;
	const int symbolcolumns;
	const int symbolrows;
	stdout_writer writer;

	stdin_reader(const pixmap_header & header)
		: header(header)
		, width(header.width)
		, height(header.height)
		, symbolcolumns(width/symbolwidth)
		, symbolrows(height/symbolheight)
		, writer(header)
	{
	}

bool readframes() {
	for (bool first = true;; first = false) {
		if (!first) {
			pixmap_header h = pixmap_header::read();
			if (h.eof) return true;
			if (h != header) return false;
		}
		if (!readpixels()) break;
	}
	return true;
}

bool readpixels() {
	const int symbolcolumns = width/symbolwidth;
	const int symbolrows = height/symbolheight;
	const int bufsize = width*symbolheight;
	static std::vector<pixel> symbolrow;
	symbolrow.resize(bufsize);
	static std::vector<tile_type> output;
	output.resize(symbolcolumns);
	writer.begin_frame();
	for (int y = 0; y < symbolrows; ++y) {
		fread(&symbolrow[0], sizeof(pixel), bufsize, stdin);
		auto buf = symbolrow.begin();
		for (int x = 0; x < symbolcolumns; ++x) {
			deinterlacer<pixel> d(&*buf, symbolcolumns);
			auto c = binarydecode(d);
			output[x] = c.second;
			buf += symbolwidth;
		}
		writer.write_row(output);
	}
	writer.end_frame();
	return 1;
}

};

template <int pixelsize>
bool readframes(pixmap_header & h) {
	stdin_reader<pixelsize> r(h);
	return r.readframes();
}

int main() {
	pixmap_header h = pixmap_header::read();
	if (!h.good) return false;
	if (h.format == 5) return readframes<1>(h) ? 0 : 1;
	if (h.format == 6) return readframes<3>(h) ? 0 : 1;
	printf("Unknown format %d\n", h.format);
	return 1;
}
// vim:set ts=4 sts=4 sw=4 noet:
