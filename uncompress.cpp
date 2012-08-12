#include "cp437.h"
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <sstream>
#include <unistd.h>

int main(int argc, char ** argv) {
	if (argc < 4) {
		printf("Usage: %s width height skip\n", argv[0]);
		return 1;
	}

	int width;
	int height;
	int skip;
	if (!strcmp(argv[1], "auto")) {
		scanf("%d\n", &width);
	} else {
		width = atoi(argv[1]);
	}
	if (!strcmp(argv[2], "auto")) {
		scanf("%d\n", &height);
	} else {
		height = atoi(argv[2]);
	}
	skip = atoi(argv[3]);

	int left = width;
	int linesleft = height;
	int skipleft = skip;
	std::stringstream outbuf;
	while (1) {
		int r = getc(stdin);
		if (r == EOF) break;
		if (!skipleft) outbuf << cp437[r];
		if (!--left) {
			left = width;
			if (!skipleft) outbuf << '\n';
			if (!--linesleft) {
				linesleft = height;
				if (skipleft--) continue;
				skipleft = skip;
				std::string out = outbuf.str();
				const char * outc = out.c_str();
				write(1, outc, out.size());
				fsync(1);
				outbuf.str("");
			}
		}
	}
	return 0;
}
// vim:set ts=4 sts=4 sw=4 noet:
