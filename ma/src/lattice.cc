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
		int chn = 0;
		for (auto k = j; k >= 0 && j-k < _segsize(t, ct[k], chn); --k) {
			word wd(*f.raw, head+k, 1+j-k);
			//cout << wd << endl;
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

int lattice::_segsize(type& t, type& u, int& chn) {
	if (t != u)
		++chn;
	if (chn > 1)
		return 0;
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
			//cout << "ARABIC:" << S_ARABIC << endl;
			return S_ARABIC;
		case U_GREEK:
			//cout << "GREEK:" << S_GREEK << endl;
			return S_GREEK;
		case U_HANGUL:
			//cout << "HANGUL:" << S_HANGUL << endl;
			return S_HANGUL;
		case U_HEBREW:
			//cout << "HEBREW:" << S_HEBREW << endl;
			return S_HEBREW;
		case U_LATIN:
			//cout << "LATIN:" << S_LATIN << endl;
			return S_LATIN;
		case U_MYANMAR:
			//cout << "MYANMAR:" << S_MYANMAR << endl;
			return S_MYANMAR;
		case U_THAI:
			//cout << "THAI:" << S_THAI << endl;
			return S_THAI;
		case U_DIGIT:
			//cout << "DIGIT:" << S_DIGIT << endl;
			return S_DIGIT;
		case U_HIRAGANA:
			//cout << "HIRAGANA:" << S_HIRAGANA << endl;
			return S_HIRAGANA;
		case U_KATAKANA:
			//cout << "KATAKANA:" << S_KATAKANA << endl;
			return S_KATAKANA;
		case U_HANJI:
			//cout << "HANJI:" << S_HANJI << endl;
			return S_HANJI;
		case U_HIRA_KATA:
			//cout << "HIRA_KATA:" << S_HIRA_KATA << endl;
			return S_HIRA_KATA;
		case U_HIRA_HANJI:
			//cout << "HIRA_HANJI:" << S_HIRA_HANJI << endl;
			return S_HIRA_HANJI;
		case U_KATA_HANJI:
			//cout << "KATA_HANJI:" << S_KATA_HANJI << endl;
			return S_KATA_HANJI;
		case U_HIRA_KATA_HANJI:
			//cout << "HIRA_KATA_HANJI:" << S_HIRA_KATA_HANJI << endl;
			return S_HIRA_KATA_HANJI;
		case U_SYNBOL:
			//cout << "SYNBOL:" << S_SYNBOL << endl;
			return S_SYNBOL;
		default:
			//cout << "MISC:" << S_MISC << endl;
			return S_MISC;
	}
}
