#include "cp437.h"
#include <cstdlib>
#include <cstdio>
#include <cstring>

int main(int argc, char ** argv) {
	if (argc < 2) {
		printf("Usage: %s width\n", argv[0]);
		return 1;
	}

	int width;
	if (!strcmp(argv[1], "auto")) {
		scanf("%d\n", &width);
	} else {
		width = atoi(argv[1]);
	}

	int left = width;
	while (1) {
		int r = getc(stdin);
		if (r == EOF) break;
		fputs(cp437[r], stdout);
		if (!--left) {
			left = width;
			putchar('\n');
		}
	}
	return 0;
}
// vim:set ts=4 sts=4 sw=4 noet:
