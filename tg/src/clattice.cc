#include"clattice.h"
#include"chartype.h"
#include"wordtype.h"
#ifdef _OPENMP
#include<omp.h>
#endif

using namespace std;
using namespace npbnlp;

static chunk eos;
static vector<int> bos(1, 0);

clattice::clattice(nio& f, int i) {
	int head = f.head[i];
	int tail = f.head[i+1];
	vector<type> wt;
	for (auto j = head; j < tail; ++j) {
		wt.push_back(wordtype::get((*f.raw)[j]));
	}
	c.resize(wt.size());
	k.resize(wt.size());
	shared_ptr<cid> dic = cid::create();
#ifdef _OPENMP
#pragma omp parallel for
#endif
	for (auto j = 0; j < wt.size(); ++j) {
		type t = wt[j];
		for (auto k = j; k >= 0 && j-k < _chsize(t, wt[k]); --k) {
			chunk ch(*f.raw, head+k, 1+j-k);
			ch.id = (*dic)[ch];
			c[j].push_back(ch);
		}
		k[j].resize(c[j].size());
	}
}

clattice::~clattice() {
}

chunk& clattice::ch(int i, int len) {
	if (i < 0 || i >= c.size())
		return eos;
	if (len-1 >= c[i].size())
		throw "invalid chunk size";
	return c[i][len-1];
}

chunk* clattice::cp(int i, int len) {
	if (i < 0 || i >= c.size())
		return &eos;
	if (len-1 >= c[i].size())
		throw "invalid chunk size";
	return &c[i][len-1];
}

int clattice::size(int i) {
	if (i < 0 || i >= c.size())
		return 1;
	return c[i].size();
}

vector<int>::iterator clattice::begin(int i, int j) {
	if (i < 0 || i >= k.size())
		return bos.begin();
	return k[i][j].begin();
}

vector<int>::iterator clattice::end(int i, int j) {
	if (i < 0 || i >= k.size())
		return bos.end();
	return k[i][j].end();
}

int clattice::_chsize(type& t, type& u) {
	switch (u) {
		case U_HIRAGANA:
			if (t == U_KATAKANA)
				t = U_HIRA_KATA;
			else if (t == U_HANJI)
				t = U_HIRA_HANJI;
			else if (t == U_KATA_HANJI)
				t = U_HIRA_KATA_HANJI;
			else if (t == U_KATA_OR_HIRA)
				t = U_HIRAGANA;
			else if (t == U_HIRA_HANJI || t == U_HIRA_KATA || t == U_HIRA_KATA_HANJI) {
			} else if (u != t)
				t = U_MISC;
			break;
		case U_KATAKANA:
			if (t == U_HIRAGANA)
				t = U_HIRA_KATA;
			else if (t == U_HANJI)
				t = U_KATA_HANJI;
			else if (t == U_HIRA_HANJI)
				t = U_HIRA_KATA_HANJI;
			else if (t == U_KATA_OR_HIRA)
				t = U_KATAKANA;
			else if (t == U_HIRA_KATA || t == U_KATA_HANJI || t == U_HIRA_KATA_HANJI) {
			} else if (u != t)
				t = U_MISC;
			break;
		case U_HANJI:
			if (t == U_HIRAGANA)
				t = U_HIRA_HANJI;
			else if (t == U_KATAKANA)
				t = U_KATA_HANJI;
			else if (t == U_HIRA_KATA || t == U_KATA_OR_HIRA)
				t = U_HIRA_KATA_HANJI;
			else if (t == U_HIRA_HANJI || t == U_KATA_HANJI || t == U_HIRA_KATA_HANJI) {
			} else if (u != t)
				t = U_MISC;
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
			return C_ARABIC;
		case U_GREEK:
			return C_GREEK;
		case U_HANGUL:
			return C_HANGUL;
		case U_HEBREW:
			return C_HEBREW;
		case U_LATIN:
			return C_LATIN;
		case U_MYANMAR:
			return C_MYANMAR;
		case U_THAI:
			return C_THAI;
		case U_DIGIT:
			return C_DIGIT;
		case U_HIRAGANA:
			return C_HIRAGANA;
		case U_KATAKANA:
			return C_KATAKANA;
		case U_HANJI:
			return C_HANJI;
		case U_HIRA_KATA:
			return C_HIRA_KATA;
		case U_HIRA_HANJI:
			return C_HIRA_HANJI;
		case U_KATA_HANJI:
			return C_KATA_HANJI;
		case U_HIRA_KATA_HANJI:
			return C_HIRA_KATA_HANJI;
		case U_SYNBOL:
			return C_SYNBOL;
		default:
			return C_MISC;
	}
}
