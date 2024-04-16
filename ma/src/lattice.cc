#include"lattice.h"
#include"chartype.h"
#include"wordtype.h"
#ifdef _OPENMP
#include<omp.h>
#endif

using namespace std;
using namespace npbnlp;

static word eos;
static vector<int> bos(1, 0);

lattice::lattice(io& f, int i) {
	int head = f.head[i];
	int tail = f.head[i+1];
	vector<type> ct;
	for (auto j = head; j < tail; ++j) {
		ct.push_back(chartype::get((*f.raw)[j]));
	}
	w.resize(ct.size());
	mu.resize(ct.size(), 0); // for slice
	//check.resize(ct.size()); // for slice
	k.resize(ct.size()); // for pos inference
	shared_ptr<wid> dic = wid::create();
#ifdef _OPENMP
#pragma omp parallel for
#endif
	for (auto j = 0; j < (int)ct.size(); ++j) {
		type t = ct[j];
		for (auto k = j; k >= 0 && j-k < _segsize(t, ct[k]); --k) {
			word wd(*f.raw, head+k, 1+j-k);
			wd.id = (*dic)[wd];
			w[j].push_back(wd);
		}
		k[j].resize(w[j].size());
		//check[j].resize(w[j].size(), 0);
	}
}

lattice::~lattice() {
}

word& lattice::wd(int i, int len) {
	if (i < 0 || i >= (int)w.size())
		return eos;
	if (len-1 >= (int)w[i].size())
		throw "invalid segment size";
	return w[i][len-1];
}

word* lattice::wp(int i, int len) {
	if (i < 0 || i >= (int)w.size())
		return &eos;
	if (len-1 >= (int)w[i].size())
		throw "invalid segment size";
	return &w[i][len-1];
}

int lattice::size(int i) {
	if (i < 0 || i >= (int)w.size())
		return 1;
	return w[i].size();
}

void lattice::slice(int i, double u) {
	mu[i] = u;
}

double lattice::u(int i) {
	if (i < 0 || i >= (int)mu.size())
		return 0;
	return mu[i];
}

vector<int>::iterator lattice::sbegin(int i, int j) {
	if (i < 0 || i >= (int)k.size())
		return bos.begin();
	return k[i][j].begin();
}

vector<int>::iterator lattice::send(int i, int j) {
	if (i < 0 || i >= (int)k.size())
		return bos.end();
	return k[i][j].end();
}

/*
bool lattice::skip(int i, int j) {
	if (i < 0 || i >= (int)check.size())
		return 0;
	return (check[i][j]);
}
*/

int lattice::_segsize(type& t, type& u) {
	switch (u) {
		case U_HIRAGANA:
			if (t == U_KATAKANA) {
				t = U_HIRA_KATA;
			} else if (t == U_HANJI) {
				t = U_HIRA_HANJI;
			} else if (t == U_KATA_HANJI) {
				t = U_HIRA_KATA_HANJI;
			} else if (t == U_KATA_OR_HIRA) {
				t = U_HIRAGANA;
			} else if (t == U_HIRA_HANJI || t == U_HIRA_KATA || t == U_HIRA_KATA_HANJI) {
			} else if (u != t) {
				t = U_MISC;
			}
			break;
		case U_KATAKANA:
			if (t == U_HIRAGANA) {
				t = U_HIRA_KATA;
			} else if (t == U_HANJI) {
				t = U_KATA_HANJI;
			} else if (t == U_HIRA_HANJI) {
				t = U_HIRA_KATA_HANJI;
			} else if (t == U_KATA_OR_HIRA) {
				t = U_KATAKANA;
			} else if (t == U_HIRA_KATA || t == U_KATA_HANJI || t == U_HIRA_KATA_HANJI) {
			} else if (u != t) {
				t = U_MISC;
			}
			break;
		case U_HANJI:
			if (t == U_HIRAGANA) {
				t = U_HIRA_HANJI;
			} else if (t == U_KATAKANA) {
				t = U_KATA_HANJI;
			} else if (t == U_HIRA_KATA) {
				t = U_HIRA_KATA_HANJI;
			} else if (t == U_KATA_OR_HIRA) {
				t = U_HIRA_KATA_HANJI;
			} else if (t == U_HIRA_HANJI || t == U_KATA_HANJI || t == U_HIRA_KATA_HANJI) {
			} else if (u != t) {
				t = U_MISC;
			}
			break;
		case U_KATA_OR_HIRA:
			if (t != U_HIRAGANA && t != U_KATAKANA && t != U_HIRA_KATA && t != U_KATA_HANJI && t != U_HIRA_HANJI && t != U_HIRA_KATA_HANJI)
				t = U_MISC;
		default:
			if (t != u) {
				t = U_MISC;
			}
	}
	switch (t) {
		case U_ARABIC:
			return S_ARABIC;
		case U_GREEK:
			return S_GREEK;
		case U_HANGUL:
			return S_HANGUL;
		case U_HEBREW:
			return S_HEBREW;
		case U_LATIN:
			return S_LATIN;
		case U_MYANMAR:
			return S_MYANMAR;
		case U_THAI:
			return S_THAI;
		case U_DIGIT:
			return S_DIGIT;
		case U_HIRAGANA:
			return S_HIRAGANA;
		case U_KATAKANA:
			return S_KATAKANA;
		case U_HANJI:
			return S_HANJI;
		case U_HIRA_KATA:
			return S_HIRA_KATA;
		case U_HIRA_HANJI:
			return S_HIRA_HANJI;
		case U_KATA_HANJI:
			return S_KATA_HANJI;
		case U_HIRA_KATA_HANJI:
			return S_HIRA_KATA_HANJI;
		case U_SYNBOL:
			return S_SYNBOL;
		default:
			return S_MISC;
	}
}
