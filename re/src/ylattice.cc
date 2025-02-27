#include"ylattice.h"

using namespace std;
using namespace npbnlp;

static word w_eos;
static vector<unsigned int> p_eos(1, 0);
static ynode eos(w_eos, p_eos);

ynode::ynode(word& w, vector<unsigned int>& p):w(w),phonetic(p),prod(0),bos(0),eos(0) {
}

ynode::~ynode() {
}

ynode::ynode(const ynode& n):w(n.w),phonetic(n.phonetic),prod(n.prod),bos(n.bos),eos(n.eos) {
}

ynode& ynode::operator=(const ynode& n) {
	w = n.w;
	if (!phonetic.empty())
		phonetic.clear();
	for (auto& i : n.phonetic)
		phonetic.emplace_back(i);
	bos = n.bos;
	prod = n.prod;
	eos = n.eos;
	return *this;
}

ylattice::ylattice(io& f, int i, trie& t) {
	int head = f.head[i];
	int tail = f.head[i+1];
	for (auto j = head; j < tail; ++j) {
		query.emplace_back((*f.raw)[j]);
	}
	y.resize(query.size());
	shared_ptr<wid> d = wid::create();
	for (auto j = 0; j < (int)query.size(); ++j) {
		vector<unsigned int> key(query.begin()+j, query.end());
		key.emplace_back(3);
		auto r = t.cp_search(key);
		for (auto& n : r) {
			word w(*f.raw, head+j, n.first);
			w.id = d->index(w);
			auto v = t.getval(n.second);
			if (v) {
				for (auto& m : v.value()) {
					y[j+n.first-1].emplace_back(ynode(w, m));
				}
			}
		}
		if (y[j].empty()) {
			word w(*f.raw, head+j, 1);
			w.id = 1; // unk
			vector<unsigned int> n(1, 0);
			y[j].emplace_back(ynode(w, n));
		}
	}
}

ylattice::~ylattice() {
}

int ylattice::len(int i, int j) {
	if (i < 0 || i >= (int)y.size())
		return 1;
	if (j >= (int)y[i].size())
		throw "found invalid node id in len()";
	return y[i][j].w.len;
}

ynode& ylattice::get(int i, int j) {
	if (i < 0 || i >= (int)y.size())
		return eos;
	if (j >= (int)y[i].size())
		throw "found invalid node id in get()";
	return y[i][j];
}

ynode* ylattice::getp(int i, int j) {
	if (i < 0 || i >= (int)y.size())
		return &eos;
	if (j >= (int)y[i].size())
		throw "found invalid node id in get()";
	return &y[i][j];
}

int ylattice::size() {
	return y.size();
}

int ylattice::size(int i) {
	if (i < 0 || i >= (int)y.size())
		return 1;
	return y[i].size();
}
