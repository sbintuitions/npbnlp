#include"cyk.h"

using namespace std;
using namespace npbnlp;

static word eos;
static vector<int> root(1, 0);

cyk::cyk(io& f, int i):s(*f.raw, f.head[i], f.head[i+1]) {
	//int size = s.size();
	k.resize(s.size(), vector<set<int> >(s.size()));
	mu.resize(s.size(), vector<double>(s.size(), 0));
}

cyk::~cyk() {
}

word& cyk::wd(int i) {
	if (i < 0 || i >= s.size())
		return eos;
	return s.wd(i);
}

word* cyk::wp(int i) {
	if (i < 0 || i >= s.size())
		return &eos;
	return &s.wd(i);
}

int cyk::size(int i, int j) {
	if (i < 0 || i >= s.size() || j < i || j >= s.size()) {
		return 0;
	}
	return k[i][j].size();
}

set<int>::iterator cyk::begin(int i, int j) {
	if (i < 0 || i >= s.size() || j < i || j >= s.size()) {
		return k[0][s.size()-1].begin();
	}
	return k[i][j].begin();
}

set<int>::iterator cyk::end(int i, int j) {
	if (i < 0 || i >= s.size() || j < i || j >= s.size()) {
		return k[0][s.size()-1].end();
	}
	return k[i][j].end();
}
