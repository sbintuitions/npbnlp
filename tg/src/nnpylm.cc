#include"nnpylm.h"
#include"convinience.h"
#include"vtable.h"
#include"rd.h"
#include"wordtype.h"
#ifdef _OPENMP
#include<omp.h>
#endif

#define C 50000
using namespace std;
using namespace npbnlp;

static unordered_map<int, int> freq;

//nnpylm::nnpylm(int n, npylm *lm): _n(n), _lm(lm), _chunk(new hpyp(_n)), _word(lm->_word), _letter(lm->_letter) {
nnpylm::nnpylm(int n, int m, int l): _n(n),  _chunk(new hpyp(_n)), _word(new hpyp(m)), _letter(new vpyp(l)) {
	_chunk->set_base(_word.get());
	_word->set_base(_letter.get());
	_letter->set_v(C);
}

nnpylm::nnpylm():_n(1), _chunk(new hpyp(_n)), _word(new hpyp(2)), _letter(new vpyp(10)) {
	_chunk->set_base(_word.get());
	_word->set_base(_letter.get());
	_letter->set_v(C);
}

nnpylm::~nnpylm() {
}

void nnpylm::add(nsentence& s) {
	lock_guard<mutex> m(_mutex);
	shared_ptr<cid> dic = cid::create();
	for (auto i = 0; i < s.size(); ++i) {
		chunk& c = s.ch(i);
		if ((*dic)[c] == 1) { // unk
			c.id = dic->index(c);
		} else {
			c.id = (*dic)[c];
		}
		freq[c.id]++;
	}
	wrap::add_a(s, _chunk.get());
}

void nnpylm::remove(nsentence& s) {
	lock_guard<mutex> m(_mutex);
	shared_ptr<cid> dic = cid::create();
	for (auto i = 0; i < s.size(); ++i) {
		chunk& c = s.ch(i);
		freq[c.id]--;
		if (freq[c.id] == 0) {
			dic->remove(c);
			freq.erase(c.id);
		}
	}
	wrap::remove_a(s, _chunk.get());
}

void nnpylm::estimate(int iter) {
	_chunk->gibbs(iter);
	_word->gibbs(iter);
	_chunk->estimate(iter);
	_word->estimate(iter);
	_letter->estimate(iter);
}

void nnpylm::poisson_correction(int n) {
	_word->poisson_correction(n);
}

void nnpylm::set(int v) {
	if (v > 0)
		_letter->set_v(v);
}

int nnpylm::n() {
	return _n;
}

int nnpylm::m() {
	return _word->n();
}

int nnpylm::l() {
	return _letter->n();
}

void nnpylm::save(const char *file) {
	FILE *fp = NULL;
	if ((fp = fopen(file, "wb")) == NULL)
		throw "failed to open save file in nnpylm::save";
	try {
		_chunk->save(fp);
		_word->save(fp);
		_letter->save(fp);
	} catch (const char *ex) {
		throw ex;
	}
	fclose(fp);
}

void nnpylm::load(const char *file) {
	FILE *fp = NULL;
	if ((fp = fopen(file, "rb")) == NULL)
		throw "failed to open model file in nnpylm::load";
	try {
		_chunk->load(fp);
		_word->load(fp);
		_letter->load(fp);
		_n = _chunk->n();
	} catch (const char *ex) {
		throw ex;
	}
}

nsentence nnpylm::sample(nio& f, int i) {
	clattice l(f, i);
	vt dp;
	for (auto t = 0; t < (int)l.c.size(); ++t) {
		for (auto j = 0; j < l.size(t); ++j) {
			const context *c = _chunk->h();
			chunk& ch = l.ch(t, j+1);
			for (auto k = 0; k < l.size(t-ch.len); ++k) {
				chunk& prev = l.ch(t-ch.len, k+1);
				const context *h = NULL;
				if (_n > 1)
					h = c->find(prev.id);
				if (h)
					_forward(l, t-ch.len-prev.len, h, ch, prev, dp[t][j], dp[t-ch.len][k], _n-1, false);
				else
					_forward(l, t-ch.len-prev.len, c, ch, prev, dp[t][j], dp[t-ch.len][k], _n-1, true);
			}
		}
	}
	nsentence s;
	chunk *ch = l.cp(l.c.size(), 1); // eos
	int t = (int)l.c.size()-ch->len;
	while (t >= 0) {
		const context *c = _chunk->h();
		vector<double> table(l.size(t), 1.);
		for (auto k = 0; k < l.size(t); ++k) {
			chunk& prev = l.ch(t, k+1);
			const context *h = NULL;
			if (_n > 1)
				h = c->find(prev.id);
			if (h)
				_backward(l, t-prev.len, h, *ch, table[k], dp[t][k], _n-1, false);
			else
				_backward(l, t-prev.len, c, *ch, table[k], dp[t][k], _n-1, true);
		}
		int len = 1+rd::ln_draw(table);
		ch = l.cp(t, len);
		s.c.push_back(*ch);
		t -= len;
	}
	reverse(s.c.begin(), s.c.end());
	s.n.resize(s.c.size(), 0);
	return s;
}

nsentence nnpylm::parse(nio& f, int i) {
	clattice l(f, i);
	vt dp;
	for (auto t = 0; t < (int)l.c.size(); ++t) {
		for (auto j = 0; j < l.size(t); ++j) {
			const context *c = _chunk->h();
			chunk& ch = l.ch(t, j+1);
			for (auto k = 0; k < l.size(t-ch.len); ++k) {
				chunk& prev = l.ch(t-ch.len, k+1);
				const context *h = NULL;
				if (_n > 1)
					h = c->find(prev.id);
				if (h)
					_forward(l, t-ch.len-prev.len, h, ch, prev, dp[t][j], dp[t-ch.len][k], _n-1, false);
				else
					_forward(l, t-ch.len-prev.len, c, ch, prev, dp[t][j], dp[t-ch.len][k], _n-1, true);
			}
		}
	}
	nsentence s;
	chunk *ch = l.cp(l.c.size(), 1); // eos
	int t = (int)l.c.size()-ch->len;
	while (t >= 0) {
		const context *c = _chunk->h();
		vector<double> table(l.size(t), 1.);
		for (auto k = 0; k < l.size(t); ++k) {
			chunk& prev = l.ch(t, k+1);
			const context *h = NULL;
			if (_n > 1)
				h = c->find(prev.id);
			if (h)
				_backward(l, t-prev.len, h, *ch, table[k], dp[t][k], _n-1, false);
			else
				_backward(l, t-prev.len, c, *ch, table[k], dp[t][k], _n-1, true);
		}
		int len = 1+rd::best(table);
		ch = l.cp(t, len);
		s.c.push_back(*ch);
		t -= len;
	}
	reverse(s.c.begin(), s.c.end());
	s.n.resize(s.c.size(), 0);
	return s;
}

void nnpylm::_forward(clattice& l, int i, const context *c, chunk& ch, chunk& p, vt& a, vt& b, int n, bool unk) {
	if (n <= 1) {
		a.v = math::lse(a.v, b.v+_chunk->lp(ch, c), !a.is_init());
		if (!a.is_init())
			a.set(true);
	} else {
		for (auto j = 0; j < l.size(i); ++j) {
			chunk& prev = l.ch(i, j+1);
			const context *h = NULL;
			if (!unk)
				h = c->find(prev.id);
			if (h)
				_forward(l, i-prev.len, h, ch, prev, a[p.len-1], b[j], n-1, false);
			else
				_forward(l, i-prev.len, c, ch, prev, a[p.len-1], b[j], n-1, true);
		}
	}
}

void nnpylm::_backward(clattice& l, int i, const context *c, chunk& ch, double& lpr, vt& b, int n, bool unk) {
	if (n <= 1) {
		lpr = math::lse(lpr, b.v+_chunk->lp(ch, c), (lpr == 1.));
	} else {
		for (auto j = 0; j < l.size(i); ++j) {
			chunk& prev = l.ch(i, j+1);
			const context *h = NULL;
			if (!unk)
				h = c->find(prev.id);
			if (h)
				_backward(l, i-prev.len, h, ch, lpr, b[j], n-1, false);
			else
				_backward(l, i-prev.len, c, ch, lpr, b[j], n-1, true);
		}
	}
}
