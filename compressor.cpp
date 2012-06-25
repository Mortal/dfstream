#include <cstring>
#include <cstdio>

unsigned char lineardecode(const char * input) {
#include "linear_decider.inl"
    return 255;
}
unsigned char binarydecode(const char * input) {
#include "binary_decider.inl"
}

const int symbolwidth = 8; // pixel width of single symbol; must be a power of two
const int symbolheight = 12;
const int indexoffsetmask = symbolwidth-1;

inline int translate(const int index, const int symbolcolumns) {
    return (index & indexoffsetmask)
        + symbolwidth*(index/(symbolcolumns*symbolwidth))
        + (symbolwidth*symbolheight)*((index % (symbolcolumns*symbolwidth))/symbolwidth);
}

int main() {
    int format, width, height, what;
    if (4 != fscanf(stdin, "P%d %d %d %d\n", &format, &width, &height, &what)) {
        printf("Couldn't read header\n");
        return 1;
    }
    if (format != 5) {
        printf("Unknown format %d\n", format);
        return 1;
    }
    const int symbolcolumns = width/symbolwidth;
    const int bufsize = width*symbolheight;
    char * symbolrow = new char[bufsize];
    char * encodedinput = new char[bufsize];
    for (int y = 0; y < height; ++y) {
        fread(symbolrow, sizeof(char), bufsize, stdin);
        int bgindex = 7;
        int maxindex = 7+(symbolcolumns-1)*symbolwidth;
        for (int i = 0; i < bufsize; ++i) {
            if (0 != i && 0 == (i % symbolwidth))
                bgindex = (bgindex == maxindex) ? 7 : bgindex+symbolwidth;
            encodedinput[translate(i, symbolcolumns)] = (symbolrow[i] != symbolrow[bgindex]);
        }
        for (char * buf = encodedinput; buf != (encodedinput+bufsize); buf += symbolwidth*symbolheight) {
            putchar(binarydecode(buf));
        }
    }
}
// vim:set ts=4 sts=4 sw=4:
