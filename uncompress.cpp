#include <iostream>
#include <vector>

using namespace std;

int main() {
	vector<string> lines;
	for (int i = 0; i < 256; ++i) {
		string line;
		getline(cin, line);
		lines.push_back(line);
	}
	char c;
	while (cin.get(c)) {
		cout << lines[static_cast<unsigned char>(c)] << '\n';
	}
	return 0;
}
// vim:set ts=4 sts=4 sw=4:
