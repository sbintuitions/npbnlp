#include"tree.h"
#include"util.h"

using namespace std;
using namespace npbnlp;

static tree root;

tree::tree():k(0),left(NULL),right(NULL),_head(0),_len(0) {
}

tree::tree(int k, tree *l, tree *r):k(k),left(l),right(r),_head(0),_len(0) {
}

tree::~tree() {
}

tree::tree(const tree& t): k(t.k), left(t.left), right(t.right), _head(t._head), _len(t._len) {
}

tree::tree(tree&& t): k(t.k), left(t.left), right(t.right), _head(t._head), _len(t._len) {
	t.k = 0;
	t.left = NULL;
	t.right = NULL;
	t._head = 0;
	t._len = 0;
}

tree& tree::operator=(const tree& t) {
	k = t.k;
	left = t.left;
	right = t.right;
	_head = t._head;
	_len = t._len;
	return *this;
}

tree& tree::operator=(tree&& t) noexcept {
	if (this == &t)
		return *this;
	k = t.k;
	left = t.left;
	right = t.right;
	_head = t._head;
	_len = t._len;
	t.k = 0;
	t.left = NULL;
	t.right = NULL;
	t._head = 0;
	t._len = 0;
	return *this;
}
