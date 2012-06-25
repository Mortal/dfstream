#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <algorithm>

using namespace std;

typedef vector<pair<string, size_t> > oclist;

struct oclist_partition {
	size_t index;
	oclist_partition(size_t index) : index(index) {}
	inline bool operator()(const oclist::value_type & a) const {
		return !a.first[index];
	}
};

struct oclist_bsearcher {
	inline bool operator()(const oclist::value_type & a, size_t index) const {
		return !a.first[index];
	}
};

string output(const string & from) {
	string res(from.size(), '0');
	for (size_t i = 0; i < from.size(); ++i) {
		if (from[i]) res[i] = '1';
	}
	return res;
}

void build_decision_tree(oclist::iterator a,
						 oclist::iterator b,
						 size_t totaloccurrences,
						 map<string, size_t> & lineids) {
	if (a == b) cout << "Error" << endl;
	if (1 == b-a) {
		cout << "return " << lineids[a->first] << "; // " << output(a->first) << endl;
		return;
	}
	size_t linelength = a->first.size();
	size_t bestdistance = -1;
	size_t bestindex = 0;
	for (size_t index = 0; index < linelength; ++index) {
		partition(a, b, oclist_partition(index));
		size_t unset = 0;
		for (auto i = a; i != b; ++i) {
			if (i->first[index]) break;
			unset += i->second;
		}
		size_t distance = ((unset+unset) > totaloccurrences) ? ((unset+unset)-totaloccurrences) : (totaloccurrences-(unset+unset));
		if (distance < bestdistance) {
			bestdistance = distance;
			bestindex = index;
			if (0 == distance) break;
		}
	}
	partition(a, b, oclist_partition(bestindex));
	size_t unset = 0;
	auto c = a;
	while (c != b) {
		if (c->first[bestindex]) break;
		unset += c->second;
		++c;
	}
	cout << "if (!input[" << bestindex << "]) {\n// " << unset << " out of " << totaloccurrences << endl;
	build_decision_tree(a, c, unset, lineids);
	cout << "} else {\n// " << (totaloccurrences-unset) << " out of " << totaloccurrences << endl;
	build_decision_tree(c, b, totaloccurrences-unset, lineids);
	cout << "}" << endl;
}

int main() {
	string line;
	map<string, size_t> lineoccurrences;
	map<string, size_t> lineids;
	size_t totaloccurrences = 0;
	size_t linelength = 0;
	vector<size_t> isone;
	vector<size_t> iszero;
	while (getline(cin, line)) {
		bool allones = true;
		bool allzeroes = true;
		for (size_t i = 0; i < line.size(); ++i) {
			if (line[i] == '0') {
				allones = false;
				line[i] = 0;
			} else {
				allzeroes = false;
				line[i] = 1;
			}
		}
		size_t id = lineids.insert(make_pair(line, totaloccurrences)).first->second;
		if (allones) cerr << "Line " << id << " is all ones" << endl;
		if (allzeroes) cerr << "Line " << id << " is all zeroes" << endl;
		if (!id) {
			linelength = line.size();
			isone.resize(linelength);
			iszero.resize(linelength);
		} else if (line.size() != linelength) {
			cerr << "Length mismatch: Expected " << linelength << ", got " << line.size() << ", exiting" << endl;
			return 1;
		}
		for (size_t i = 0; i < line.size(); ++i) {
			if (line[i]) ++isone[i];
			else ++iszero[i];
		}
		++(lineoccurrences.insert(make_pair(line, 0)).first->second);
		++totaloccurrences;
	}
	if (!totaloccurrences) {
		cerr << "No input" << endl;
		return 1;
	}

	{
		size_t least = -1;
		size_t pos = 0;
		cerr << "The following positions are all one: ";
		for (size_t i = 0; i < linelength; ++i) {
			if (0 == iszero[i]) cerr << i << ", ";
			if (iszero[i] < least) {least = iszero[i]; pos = i;}
		}
		if (least) cerr << pos << " (" << least << ')';
		cerr << endl;
	}

	{
		size_t least = -1;
		size_t pos = 0;
		cerr << "The following positions are all zero: ";
		for (size_t i = 0; i < linelength; ++i) {
			if (0 == isone[i]) cerr << i << ", ";
			if (isone[i] < least) {least = isone[i]; pos = i;}
		}
		if (least) cerr << pos << " (" << least << ')';
		cerr << endl;
	}

	oclist lineoccurrences_vec(lineoccurrences.begin(), lineoccurrences.end());

	build_decision_tree(lineoccurrences_vec.begin(), lineoccurrences_vec.end(), totaloccurrences, lineids);

	return 0;
}
// vim:set ts=4 sts=4 sw=4:
