#include"vtable.h"

using namespace std;
using namespace npbnlp;

vt::vt():v(0), n(0), id(0), _parent(NULL), _init(false), _t(new vtable) {
}

vt::vt(int n, int id, vt *p):v(0), n(n), id(id), _parent(p), _init(false), _t(new vtable) {
}

vt::~vt() {
}

vt& vt::operator[](int i) {
	auto it = _t->find(i);
	if (it == _t->end()) {
		lock_guard<mutex> m(_mutex);
		(*_t)[i] = shared_ptr<vt>(new vt(n+1, i, this));
	}
	return *((*_t)[i]);
}

bool vt::is_init() {
	return _init;
}

void vt::set(bool f) {
	_init = f;
	if (_parent)
		_parent->set(f);
}
