#include"tree.h"
#include"util.h"

using namespace std;
using namespace npbnlp;

static word eos;
static node blank;

node::node():k(-1), i(0), j(0), b(0) {
}

node::~node() {
}

tree::tree() {
}

tree::tree(sentence& s):s(s) {
	c.resize(s.size()*(1.+s.size())/2);
}

tree::~tree() {
}

node& tree::operator[](int i) {
	if (i < 0 || i >= c.size())
		return blank;
	return c[i];
}

word& tree::wd(int i) {
	if (i < 0 || i >= s.size())
		return eos;
	return s.wd(i);
}

tree::tree(const tree& t): c(t.c), s(t.s){
}

tree& tree::operator=(const tree& t) {
	for (auto it = t.c.begin(); it != t.c.end(); ++it)
		c.push_back(*it);
	s = t.s;
	return *this;
}

tree& tree::operator=(tree&& t) noexcept {
	if (this == &t)
		return *this;
	c = move(t.c);
	s = move(t.s);
	t.c.clear();
	t.s.w.clear();
	t.s.n.clear();

	return *this;
}
