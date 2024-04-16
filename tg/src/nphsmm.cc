#include"nphsmm.h"
#include"convinience.h"
#include"rd.h"
#include"wordtype.h"
#ifdef _OPENMP
#include<omp.h>
#endif

#define C 50000
#define K 1000
using namespace std;
using namespace npbnlp;

static unordered_map<int, int> cfreq;
static unordered_map<int, int> kfreq;

nphsmm::nphsmm(): _n(1), _m(2), _l(10), _k(20), _v(C), _K(K), _a(1), _b(1), _class(new hpyp(_n)), _chunk(new vector<shared_ptr<hpyp> >), _word(new vector<shared_ptr<hpyp> >), _letter(new vector<shared_ptr<vpyp> >) {
	_class->set_v(K);
	for (auto i = 0; i < _k+1; ++i) {
		_chunk->push_back(shared_ptr<hpyp>(new hpyp(_n)));
		_word->push_back(shared_ptr<hpyp>(new hpyp(_m)));
		_letter->push_back(shared_ptr<vpyp>(new vpyp(_l)));
		(*_letter)[i]->set_v(_v);
		(*_word)[i]->set_base((*_letter)[i].get());
		(*_chunk)[i]->set_base((*_word)[i].get());
	}
}

nphsmm::nphsmm(int n, int m, int l, int k): _n(n), _m(m), _l(l), _k(k), _v(C), _K(K), _a(1), _b(1), _class(new hpyp(_n)), _chunk(new vector<shared_ptr<hpyp> >), _word(new vector<shared_ptr<hpyp> >), _letter(new vector<shared_ptr<vpyp> >) {
	_class->set_v(K);
	for (auto i = 0; i < _k+1; ++i) {
		_chunk->push_back(shared_ptr<hpyp>(new hpyp(_n)));
		_word->push_back(shared_ptr<hpyp>(new hpyp(_m)));
		_letter->push_back(shared_ptr<vpyp>(new vpyp(_l)));
		(*_letter)[i]->set_v(_v);
		(*_word)[i]->set_base((*_letter)[i].get());
		(*_chunk)[i]->set_base((*_word)[i].get());
	}
}

nphsmm::~nphsmm() {
}

void nphsmm::set(int v, int k) {
	_v = v;
	_K = k;
	_k = min(_k, _K);
	for (auto it = _letter->begin(); it != _letter->end(); ++it) {
		(*it)->set_v(_v);
	}
	_class->set_v(_K);
}

int nphsmm::n() {
	return _n;
}

int nphsmm::m() {
	return _m;
}

int nphsmm::l() {
	return _l;
}

void nphsmm::slice(double a, double b) {
	if (a <= 0 || b <= 0) {
		return;
	}
	_a = a;
	_b = b;
}

void nphsmm::save(const char *file) {
	FILE *fp = NULL;
	if ((fp = fopen(file,"wb")) == NULL)
		throw "failed to open save file in nphsmm::save";
	try {
		if (fwrite(&_n, sizeof(int), 1, fp) != 1)
			throw "failed to write _n in nphsmm::save";
		if (fwrite(&_m, sizeof(int), 1, fp) != 1)
			throw "failed to write _m in nphsmm::save";
		if (fwrite(&_l, sizeof(int), 1, fp) != 1)
			throw "failed to write _l in nphsmm::save";
		if (fwrite(&_k, sizeof(int), 1, fp) != 1)
			throw "failed to write _k in nphsmm::save";
		if (fwrite(&_v, sizeof(int), 1, fp) != 1)
			throw "failed to write _v in nphsmm::save";
		if (fwrite(&_K, sizeof(int), 1, fp) != 1)
			throw "failed to write _K in nphsmm::save";
		if (fwrite(&_a, sizeof(double), 1, fp) != 1)
			throw "failed to write _a in nphsmm::save";
		if (fwrite(&_b, sizeof(double), 1, fp) != 1)
			throw "failed to write _b in nphsmm::save";
		_class->save(fp);
		for (auto i = 0; i < _k+1; ++i) {
			(*_chunk)[i]->save(fp);
			(*_word)[i]->save(fp);
			(*_letter)[i]->save(fp);
		}
	} catch (const char *ex) {
		throw ex;
	}
	fclose(fp);
}

void nphsmm::load(const char *file) {
	FILE *fp = NULL;
	if ((fp = fopen(file,"rb")) == NULL)
		throw "failed to open save file in nphsmm::load";
	try {
		if (fread(&_n, sizeof(int), 1, fp) != 1)
			throw "failed to read _n in nphsmm::load";
		if (fread(&_m, sizeof(int), 1, fp) != 1)
			throw "failed to read _m in nphsmm::load";
		if (fread(&_l, sizeof(int), 1, fp) != 1)
			throw "failed to read _l in nphsmm::load";
		if (fread(&_k, sizeof(int), 1, fp) != 1)
			throw "failed to read _k in nphsmm::load";
		if (fread(&_v, sizeof(int), 1, fp) != 1)
			throw "failed to read _v in nphsmm::load";
		if (fread(&_K, sizeof(int), 1, fp) != 1)
			throw "failed to read _K in nphsmm::load";
		if (fread(&_a, sizeof(double), 1, fp) != 1)
			throw "failed to read _a in nphsmm::load";
		if (fread(&_b, sizeof(double), 1, fp) != 1)
			throw "failed to read _b in nphsmm::load";
		_class->load(fp);
		while ((int)_chunk->size() < _k+1) {
			int k = _chunk->size();
			_chunk->push_back(shared_ptr<hpyp>(new hpyp(_n)));
			_word->push_back(shared_ptr<hpyp>(new hpyp(_m)));
			_letter->push_back(shared_ptr<vpyp>(new vpyp(_l)));
			(*_chunk)[k]->set_base((*_word)[k].get());
			(*_word)[k]->set_base((*_letter)[k].get());
			(*_letter)[k]->set_v(_v);
		}
		for (auto i = 0; i < _k+1; ++i) {
			(*_chunk)[i]->load(fp);
			(*_word)[i]->load(fp);
			(*_letter)[i]->load(fp);
		}
	} catch (const char *ex) {
		throw ex;
	}
	fclose(fp);
}

void nphsmm::init(nsentence& s) {
	lock_guard<mutex> m(_mutex);
	shared_ptr<cid> dic = cid::create();
	for (int i = 0; i < s.size(); ++i) {
		chunk& x = s.ch(i);
		if ((*dic)[x] == 1) { // unk
			x.id = dic->index(x);
		} else {
			x.id = (*dic)[x];
		}
		cfreq[x.id]++;
		context *h = _class->h();
		for (int j = 1; j < _n; ++j) {
			chunk& ch = s.ch(i-j);
			h = h->make(ch.k);
		}
		vector<double> table;
		for (int k = 1; k < _k+1; ++k) {
			const context *c = (*_chunk)[k]->h();
			for (int j = 1; j < _n; ++j) {
				chunk& ch = s.ch(i-j);
				context *d = c->find(ch.id);
				if (!d)
					break;
				c = d;
			}
			double lp = _class->lp(k, h)+(*_chunk)[k]->lp(x, c);
			table.push_back(lp);
		}
		x.k = 1+rd::ln_draw(table);
		if (x.k == _k) {
			_resize();
		}
		context *c = (*_chunk)[x.k]->make(s, i);
		(*_chunk)[x.k]->add(x, c);
		_class->add(x.k, h);
		kfreq[x.k]++;
	}
	// eos
	context *h = (*_chunk)[0]->make(s, s.size());
	(*_chunk)[0]->add(s.ch(s.size()), h);
	context *c = _class->h();
	int eos = s.size();
	for (int j = 1; j < _n; ++j) {
		chunk& ch = s.ch(eos-j);
		c = c->make(ch.k);
	}
	_class->add(s.ch(s.size()).k, c);
}

void nphsmm::add(nsentence& s) {
	lock_guard<mutex> m(_mutex);
	shared_ptr<cid> dic = cid::create();
	for (auto i = 0; i < s.size(); ++i) {
		chunk& ch = s.ch(i);
		if ((*dic)[ch] == 1) {
			ch.id = dic->index(ch);
		} else {
			ch.id = (*dic)[ch];
		}
		cfreq[ch.id]++;
		kfreq[ch.k]++;
	}
	int rd[s.size()+1] = {0};
	rd::shuffle(rd, s.size()+1);
	for (int i = 0; i < s.size()+1; ++i) {
		chunk& ch = s.ch(rd[i]);
		context *h = (*_chunk)[ch.k]->make(s, rd[i]);
		(*_chunk)[ch.k]->add(ch, h);
		context *c = _class->h();
		for (int j = 1; j < _n; ++j) {
			chunk& x = s.ch(rd[i]-j);
			c = c->make(x.k);
		}
		_class->add(ch.k, c);
		if (ch.k == _k)
			_resize();
	}
}

void nphsmm::remove(nsentence& s) {
	lock_guard<mutex> m(_mutex);
	shared_ptr<cid> dic = cid::create();
	for (auto i = 0; i < s.size(); ++i) {
		chunk& ch = s.ch(i);
		cfreq[ch.id]--;
		kfreq[ch.k]--;
		if (cfreq[ch.id] == 0) {
			dic->remove(ch);
			cfreq.erase(ch.id);
		}
	}
	for (int i = 0; i < s.size()+1; ++i) {
		chunk& ch = s.ch(i);
		context *h = (*_chunk)[ch.k]->find(s, i);
		(*_chunk)[ch.k]->remove(ch, h);
		context *c = _class->h();
		for (int j = 1; j < _n; ++j) {
			chunk& x = s.ch(i-j);
			c = c->find(x.k);
		}
		_class->remove(ch.k, c);
	}
	for (int k = _k-1; kfreq[k] == 0; --k) {
		_shrink();
	}
}

void nphsmm::estimate(int iter) {
	for (int i = 1; i < _k+1; ++i) {
		(*_chunk)[i]->gibbs(iter);
		(*_word)[i]->gibbs(iter);
		(*_chunk)[i]->estimate(iter);
		(*_word)[i]->estimate(iter);
		(*_letter)[i]->estimate(iter);
	}
	_class->estimate(iter);
}

void nphsmm::poisson_correction(int n) {
	for (int i = 1; i < _k+1; ++i) {
		(*_word)[i]->poisson_correction(n);
	}
}

nsentence nphsmm::parse(nio& f, int i) {
	clattice l(f, i);
	vt dp;
	_slice(l);
	for (auto t = 0; t < (int)l.c.size(); ++t) {
		for (auto j = 0; j < l.size(t); ++j) {
			chunk& ch = l.ch(t, j+1);
			for (auto k = l.begin(t, j); k != l.end(t, j); ++k) {
				const context *c = (*_chunk)[*k]->h();
				const context *z = _class->h();
				for (auto p = 0; p < l.size(t-ch.len); ++p) {
					const context *h = NULL;
					chunk& prev = l.ch(t-ch.len, p+1);
					if (_n > 1)
						h = c->find(prev.id);
					for (auto q = l.begin(t-ch.len, p); q != l.end(t-ch.len, p); ++q) {
						const context *u = NULL;
						if (_n > 1)
							u = z->find(*q);
						if (h && u)
							_forward(l, t-ch.len-prev.len, h, u, ch, *k, prev, *q, dp[t][j][*k], dp[t-ch.len][p][*q], _n-1, false, false);
						else if (h)
							_forward(l, t-ch.len-prev.len, h, z, ch, *k, prev, *q, dp[t][j][*k], dp[t-ch.len][p][*q], _n-1, false, true);
						else if (u)
							_forward(l, t-ch.len-prev.len, c, u, ch, *k, prev, *q, dp[t][j][*k], dp[t-ch.len][p][*q], _n-1, true, false);
						else
							_forward(l, t-ch.len-prev.len, c, z, ch, *k, prev, *q, dp[t][j][*k], dp[t-ch.len][p][*q], _n-1, true, true);
					}
				}
			}
		}
	}
	nsentence s;
	chunk *ch = l.cp(l.c.size(), 1);
	int t = (int)l.c.size()-ch->len;
	while (t >= 0) {
		const context *c = (*_chunk)[ch->k]->h();
		const context *z = _class->h();
		vector<double> table;
		vector<int> len;
		vector<int> k;
		for (int p = 0; p < l.size(t); ++p) {
			const context *h = NULL;
			chunk& prev = l.ch(t, p+1);
			if (_n > 1)
				h = c->find(prev.id);
			for (auto q = l.begin(t, p); q != l.end(t, p); ++q) {
				const context *u = NULL;
				int j = table.size();
				table.push_back(1.);	
				len.push_back(p+1);
				k.push_back(*q);
				if (_n > 1)
					u = z->find(*q);
				if (h && u)
					_backward(l, t-prev.len, h, u, *ch, ch->k, prev, *q, table[j], dp[t][p][*q], _n-1, false, false);
				else if (h)
					_backward(l, t-prev.len, h, z, *ch, ch->k, prev, *q, table[j], dp[t][p][*q], _n-1, false, true);
				else if (u)
					_backward(l, t-prev.len, c, u, *ch, ch->k, prev, *q, table[j], dp[t][p][*q], _n-1, true, false);
				else
					_backward(l, t-prev.len, c, z, *ch, ch->k, prev, *q, table[j], dp[t][p][*q], _n-1, true, true);
			}
		}
		int id = rd::best(table);
		ch = l.cp(t, len[id]);
		ch->k = k[id];
		s.c.push_back(*ch);
		t -= ch->len;
	}
	reverse(s.c.begin(), s.c.end());
	s.n.resize(s.c.size(), 0);
	return s;
}

nsentence nphsmm::sample(nio& f, int i) {
	clattice l(f, i);
	vt dp;
	_slice(l);
	for (auto t = 0; t < (int)l.c.size(); ++t) {
		for (auto j = 0; j < l.size(t); ++j) {
			chunk& ch = l.ch(t, j+1);
			for (auto k = l.begin(t, j); k != l.end(t, j); ++k) {
				const context *c = (*_chunk)[*k]->h();
				const context *z = _class->h();
				for (auto p = 0; p < l.size(t-ch.len); ++p) {
					const context *h = NULL;
					chunk& prev = l.ch(t-ch.len, p+1);
					if (_n > 1)
						h = c->find(prev.id);
					for (auto q = l.begin(t-ch.len, p); q != l.end(t-ch.len, p); ++q) {
						const context *u = NULL;
						if (_n > 1)
							u = z->find(*q);
						if (h && u)
							_forward(l, t-ch.len-prev.len, h, u, ch, *k, prev, *q, dp[t][j][*k], dp[t-ch.len][p][*q], _n-1, false, false);
						else if (h)
							_forward(l, t-ch.len-prev.len, h, z, ch, *k, prev, *q, dp[t][j][*k], dp[t-ch.len][p][*q], _n-1, false, true);
						else if (u)
							_forward(l, t-ch.len-prev.len, c, u, ch, *k, prev, *q, dp[t][j][*k], dp[t-ch.len][p][*q], _n-1, true, false);
						else
							_forward(l, t-ch.len-prev.len, c, z, ch, *k, prev, *q, dp[t][j][*k], dp[t-ch.len][p][*q], _n-1, true, true);
					}
				}
			}
		}
	}
	nsentence s;
	chunk *ch = l.cp(l.c.size(), 1);
	int t = (int)l.c.size()-ch->len;
	while (t >= 0) {
		const context *c = (*_chunk)[ch->k]->h();
		const context *z = _class->h();
		vector<double> table;
		vector<int> len;
		vector<int> k;
		for (int p = 0; p < l.size(t); ++p) {
			const context *h = NULL;
			chunk& prev = l.ch(t, p+1);
			if (_n > 1)
				h = c->find(prev.id);
			for (auto q = l.begin(t, p); q != l.end(t, p); ++q) {
				const context *u = NULL;
				int j = table.size();
				table.push_back(1.);	
				len.push_back(p+1);
				k.push_back(*q);
				if (_n > 1)
					u = z->find(*q);
				if (h && u)
					_backward(l, t-prev.len, h, u, *ch, ch->k, prev, *q, table[j], dp[t][p][*q], _n-1, false, false);
				else if (h)
					_backward(l, t-prev.len, h, z, *ch, ch->k, prev, *q, table[j], dp[t][p][*q], _n-1, false, true);
				else if (u)
					_backward(l, t-prev.len, c, u, *ch, ch->k, prev, *q, table[j], dp[t][p][*q], _n-1, true, false);
				else
					_backward(l, t-prev.len, c, z, *ch, ch->k, prev, *q, table[j], dp[t][p][*q], _n-1, true, true);
			}
		}
		int id = rd::ln_draw(table);
		ch = l.cp(t, len[id]);
		ch->k = k[id];
		s.c.push_back(*ch);
		t -= ch->len;
	}
	reverse(s.c.begin(), s.c.end());
	s.n.resize(s.c.size(), 0);
	return s;
}

void nphsmm::_forward(clattice& l, int i, const context *c, const context *z, chunk& ch, int k, chunk& prev, int q, vt& a, vt& b, int n, bool unk, bool not_exist) {
	if (n <= 1) {
		a.v = math::lse(a.v, b.v+(*_chunk)[k]->lp(ch, c)+_class->lp(k, z), !a.is_init());
		if (!a.is_init())
			a.set(true);
	} else {
		for (auto j = 0; j < l.size(i); ++j) {
			chunk& y = l.ch(i, j+1);
			const context *h = NULL;
			if (!unk && n > 1)
				h = c->find(y.id);
			for (auto r = l.begin(i, j); r != l.end(i, j); ++r) {
				const context *u = NULL;
				if (!not_exist && n > 1)
					u = z->find(*r);
				if (h && u)
					_forward(l, i-y.len, h, u, ch, k, y, *r, a[prev.len-1][q], b[j][*r], n-1, false, false);
				else if (h)
					_forward(l, i-y.len, h, z, ch, k, y, *r, a[prev.len-1][q], b[j][*r], n-1, false, true);
				else if (u)
					_forward(l, i-y.len, c, u, ch, k, y, *r, a[prev.len-1][q], b[j][*r], n-1, true, false);
				else
					_forward(l, i-y.len, c, z, ch, k, y, *r, a[prev.len-1][q], b[j][*r], n-1, true, true);
			}
		}
	}
}

void nphsmm::_backward(clattice& l, int i, const context *c, const context *z, chunk& ch, int k, chunk& prev, int q, double& lpr, vt& b, int n, bool unk, bool not_exist) {
	if (n <= 1) {
		lpr = math::lse(lpr, b.v+(*_chunk)[k]->lp(ch, c)+_class->lp(k, z), (lpr == 1.));
	} else {
		for (auto j = 0; j < l.size(i); ++j) {
			chunk& y = l.ch(i, j+1);
			const context *h = NULL;
			if (!unk && n > 1)
				h = c->find(y.id);
			for (auto r = l.begin(i, j); r != l.end(i, j); ++r) {
				const context *u = NULL;
				if (!not_exist && n > 1)
					u = z->find(*r);
				if (h && u)
					_backward(l, i-y.len, h, u, ch, k, y, *r, lpr, b[j][*r], n-1, false, false);
				else if (h)
					_backward(l, i-y.len, h, z, ch, k, y, *r, lpr, b[j][*r], n-1, false, true);
				else if (u)
					_backward(l, i-y.len, c, u, ch, k, y, *r, lpr, b[j][*r], n-1, true, false);
				else
					_backward(l, i-y.len, c, z, ch, k, y, *r, lpr, b[j][*r], n-1, true, true);
			}
		}
	}
}

void nphsmm::_slice(clattice& l) {
	beta_distribution be;
	shared_ptr<generator> g = generator::create();
	for (auto t = 0; t < (int)l.c.size(); ++t) {
		for (auto c = l.c[t].begin(); c != l.c[t].end(); ++c) {
			double z = 0;
			vector<double> table;
			for (auto k = 1; k < _k+1; ++k) {
				double lp = (*_chunk)[k]->lp(*c, (*_chunk)[k]->h())+_class->lp(k, _class->h());
				z = math::lse(z, lp, (z==0));
				table.push_back(lp);
			}
			for (auto i = table.begin(); i != table.end(); ++i) {
				*i -= z;
			}
			int id = rd::ln_draw(table);
			double mu = log(be(_a, _b))+table[id];
			for (auto i = 0; i < (int)table.size(); ++i) {
				if (table[i] >= mu)
					l.k[t][c->len-1].push_back(i+1);
			}
		}
	}
}

void nphsmm::_resize() {
	if (_k+1 > _K)
		return;
	++_k;
	_chunk->resize(_k+1, shared_ptr<hpyp>(new hpyp(_n)));
	_word->resize(_k+1, shared_ptr<hpyp>(new hpyp(_m)));
	_letter->resize(_k+1, shared_ptr<vpyp>(new vpyp(_l)));
	(*_chunk)[_k+1]->set_base((*_word)[_k].get());
	(*_word)[_k]->set_base((*_letter)[_k].get());
	(*_letter)[_k]->set_v(_v);
}

void nphsmm::_shrink() {
	--_k;
	_chunk->pop_back();
	_word->pop_back();
	_letter->pop_back();
}
