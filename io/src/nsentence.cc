#include"nsentence.h"
#include"util.h"

using namespace std;
using namespace npbnlp;

static chunk eos;

nsentence::nsentence() {
}

/*
nsentence::nsentence(vector<word>& d, int head, int tail) {
}
*/

nsentence::~nsentence() {
}

int nsentence::operator[](int i) {
	if (i < 0 || i >= (int)c.size())
		return 0;
	return c[i].id;
}

chunk& nsentence::ch(int i) {
	if (i < 0 || i >= (int)c.size()) {
		return eos;
	}
	return c[i];
}

nsentence::nsentence(const nsentence& s) {
	for (auto it = s.c.begin(); it != s.c.end(); ++it)
		c.push_back(*it);
	for (auto it = s.n.begin(); it != s.n.end(); ++it)
		n.push_back(*it);
}

nsentence& nsentence::operator=(const nsentence& s) {
	c.clear();
	n.clear();
	for (auto it = s.c.begin(); it != s.c.end(); ++it)
		c.push_back(*it);
	for (auto it = s.n.begin(); it != s.n.end(); ++it)
		n.push_back(*it);
	return *this;
}

nsentence& nsentence::operator=(nsentence&& s) noexcept {
	if (this == &s)
		return *this;
	c = move(s.c);
	n = move(s.n);
	s.c.clear();
	s.n.clear();
	return *this;
}

int nsentence::size() {
	return c.size();
}
