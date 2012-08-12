#include "cp437.h"
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <sstream>
#include <unistd.h>

int main(int argc, char ** argv) {
	int width;
	int height;
	int skip = 0;
	scanf("%d\n", &width);
	scanf("%d\n", &height);
	if (argc >= 2) skip = atoi(argv[1]);

	int left = width;
	int linesleft = height;
	int skipleft = 0;
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
