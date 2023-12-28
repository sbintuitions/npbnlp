#include"phsmm.h"
#include"convinience.h"
#include"rd.h"
#include"wordtype.h"
#include"vtable.h"
#include"lattice.h"
#include"generator.h"
#include<random>
#ifdef _OPENMP
#include<omp.h>
#endif

#define C 50000
#define K 1000
#define ZERO 1e-36

using namespace std;
using namespace npbnlp;

static unordered_map<int, int> wfreq;
static unordered_map<int, int> pfreq;

phsmm::phsmm():_n(1),_m(10),_l(2),_k(20),_v(C),_K(K),_a(1),_b(1),_pos(new hpyp(_l)),_word(new vector<shared_ptr<hpyp> >),_letter(new vector<shared_ptr<vpyp> >) {
	_pos->set_v(_K);
	for (auto i = 0; i < _k+1; ++i) {
		_word->push_back(shared_ptr<hpyp>(new hpyp(_n)));
		_letter->push_back(shared_ptr<vpyp>(new vpyp(_m)));
		(*_letter)[i]->set_v(_v);
		(*_word)[i]->set_base((*_letter)[i].get());
	}
}

phsmm::phsmm(int n, int m, int l, int k):_n(n),_m(m),_l(l),_k(k),_v(C),_K(K),_a(1),_b(1),_pos(new hpyp(_l)),_word(new vector<shared_ptr<hpyp> >),_letter(new vector<shared_ptr<vpyp> >) {
	_pos->set_v(_K);
	for (auto i = 0; i < _k+1; ++i) {
		_word->push_back(shared_ptr<hpyp>(new hpyp(_n)));
		_letter->push_back(shared_ptr<vpyp>(new vpyp(_m)));
		(*_letter)[i]->set_v(_v);
		(*_word)[i]->set_base((*_letter)[i].get());
	}
}

phsmm::~phsmm() {
}

void phsmm::set(int v, int k) {
	_v = v;
	_K = k;
	for (auto it = _letter->begin(); it != _letter->end(); ++it) {
		(*it)->set_v(_v);
	}
	_pos->set_v(k);
}

int phsmm::n() {
	return _n;
}

int phsmm::m() {
	return _m;
}

int phsmm::l() {
	return _l;
}

void phsmm::slice(double a, double b) {
	if (a <= 0 || b <= 0) {
		return;
	}
	_a = a;
	_b = b;
}

void phsmm::save(const char *file) {
	FILE *fp = NULL;
	if ((fp = fopen(file, "wb")) == NULL)
		throw "failed to open save file in phsmm::save";
	try {
		if (fwrite(&_n, sizeof(int), 1, fp) != 1)
			throw "failed to write _n in phsmm::save";
		if (fwrite(&_m, sizeof(int), 1, fp) != 1)
			throw "failed to write _m in phsmm::save";
		if (fwrite(&_l, sizeof(int), 1, fp) != 1)
			throw "failed to write _l in phsmm::save";
		if (fwrite(&_k, sizeof(int), 1, fp) != 1)
			throw "failed to write _k in phsmm::save";
		if (fwrite(&_v, sizeof(int), 1, fp) != 1)
			throw "failed to write _v in phsmm::save";
		if (fwrite(&_K, sizeof(int), 1, fp) != 1)
			throw "failed to write _K in phsmm::save";
		_pos->save(fp);
		for (auto i = 0; i < _k+1; ++i) {
			(*_word)[i]->save(fp);
			(*_letter)[i]->save(fp);
		}
	} catch (const char *ex) {
		throw ex;
	}
	fclose(fp);
}

void phsmm::load(const char *file) {
	FILE *fp = NULL;
	if ((fp = fopen(file, "rb")) == NULL)
		throw "failed to open save file in phsmm::save";
	try {
		if (fread(&_n, sizeof(int), 1, fp) != 1)
			throw "failed to read _n in phsmm::save";
		if (fread(&_m, sizeof(int), 1, fp) != 1)
			throw "failed to read _m in phsmm::save";
		if (fread(&_l, sizeof(int), 1, fp) != 1)
			throw "failed to read _l in phsmm::save";
		if (fread(&_k, sizeof(int), 1, fp) != 1)
			throw "failed to read _k in phsmm::save";
		if (fread(&_v, sizeof(int), 1, fp) != 1)
			throw "failed to read _v in phsmm::save";
		if (fread(&_K, sizeof(int), 1, fp) != 1)
			throw "failed to read _K in phsmm::save";
		_pos->load(fp);
		while (_word->size() < _k+1) {
			_word->push_back(shared_ptr<hpyp>(new hpyp(_n)));
			_letter->push_back(shared_ptr<vpyp>(new vpyp(_m)));
			(*_word)[_word->size()-1]->set_base((*_letter)[_word->size()-1].get());
			(*_letter)[_word->size()-1]->set_v(_v);
		}
		for (auto i = 0; i < _k+1; ++i) {
			(*_word)[i]->load(fp);
			(*_letter)[i]->load(fp);
		}
	} catch (const char *ex) {
		throw ex;
	}
	fclose(fp);
}

/*
 // random init
void phsmm::init(sentence& s) {
	lock_guard<mutex> m(_mutex);
	shared_ptr<wid> dic = wid::create();
	uniform_int_distribution<> u(1, _k-1);
	shared_ptr<generator> g = generator::create();
	for (int i = 0; i < s.size(); ++i) {
		word& x = s.wd(i);
		x.pos = u((*g)());
		if ((*dic)[x] == 1) { // unk
			x.id = dic->index(x);
		} else {
			x.id = (*dic)[x];
		}
		wfreq[x.id]++;
		context *p = _pos->h();
		for (int j = 1; j < _l; ++j) {
			word& w = s.wd(i-j);
			p = p->make(w.pos);
		}
		context *h = (*_word)[x.pos]->make(s, i);
		(*_word)[x.pos]->add(x, h);
		_pos->add(x.pos, p);
		pfreq[x.pos]++;
	}
	// eos
	context *h = (*_word)[0]->make(s, s.size());
	(*_word)[0]->add(s.wd(s.size()),h);
	context *p = _pos->h();
	int eos = s.size();
	for (int j = 1; j < _l; ++j) {
		word& w = s.wd(eos-j);
		p = p->make(w.pos);
	}
	_pos->add(s.wd(s.size()).pos, p);
}
*/

// initialization by model
void phsmm::init(sentence& s) {
	lock_guard<mutex> m(_mutex);
	shared_ptr<wid> dic = wid::create();
	for (int i = 0; i < s.size(); ++i) {
		word& x = s.wd(i);
		if ((*dic)[x] == 1) { // unk
			x.id = dic->index(x);
		} else {
			x.id = (*dic)[x];
		}
		wfreq[x.id]++;
		context *p = _pos->h();
		for (int j = 1; j < _l; ++j) {
			word& w = s.wd(i-j);
			context *q = p->make(w.pos);
			if (!q)
				break;
			p = q;
		}
		vector<double> table;
		for (int k = 1; k < _k+1; ++k) {
			const context *c = (*_word)[k]->h();
			for (int j = 1; j < _n; ++j) {
				word& w = s.wd(i-j);
				context *d = c->find(w.id);
				if (!d)
					break;
				c = d;
			}
			double lp = _pos->lp(k, p)+(*_word)[k]->lp(x, c);
			table.push_back(lp);
		}
		int pos = 1+rd::ln_draw(table);
		x.pos = pos;
		if (pos == _k) {
			_resize();
		}
		context *h = (*_word)[pos]->make(s, i);
		(*_word)[pos]->add(x, h);
		_pos->add(pos, p);
		pfreq[pos]++;
	}
	// eos
	context *h = (*_word)[0]->make(s, s.size());
	(*_word)[0]->add(s.wd(s.size()),h);
	context *p = _pos->h();
	int eos = s.size();
	for (int j = 1; j < _l; ++j) {
		word& w = s.wd(eos-j);
		p = p->make(w.pos);
	}
	_pos->add(s.wd(s.size()).pos, p);
}

void phsmm::add(sentence& s) {
	lock_guard<mutex> m(_mutex);
	shared_ptr<wid> dic = wid::create();
	for (auto i = 0; i < s.size(); ++i) {
		word& w = s.wd(i);
		if ((*dic)[w] == 1) {
			w.id = dic->index(w);
		} else {
			w.id = (*dic)[w];
		}
		wfreq[w.id]++;
		pfreq[w.pos]++;
	}
	int rd[s.size()+1] = {0};
	rd::shuffle(rd, s.size()+1);
	for (int i = 0; i < s.size()+1; ++i) {
		word& w = s.wd(rd[i]);
		context *h = (*_word)[w.pos]->make(s, rd[i]);
		(*_word)[w.pos]->add(w, h);
		// update pos arrangement
		context *p = _pos->h();
		for (int j = 1; j < _l; ++j) {
			word& x = s.wd(rd[i]-j);
			p = p->make(x.pos);
		}
		_pos->add(w.pos, p);
		if (w.pos == _k)
			_resize();
	}
}

void phsmm::remove(sentence& s) {
	lock_guard<mutex> m(_mutex);
	shared_ptr<wid> dic = wid::create();
	for (auto i = 0; i < s.size(); ++i) {
		word& w = s.wd(i);
		wfreq[w.id]--;
		pfreq[w.pos]--;
		if (wfreq[w.id] == 0) {
			dic->remove(w);
			wfreq.erase(w.id);
		}
	}
	for (int i = 0; i < s.size()+1; ++i) {
		word& w = s.wd(i);
		context *h = (*_word)[w.pos]->find(s, i);
		(*_word)[w.pos]->remove(w, h);
		// update pos arrangement
		context *p = _pos->h();
		for (int j = 1; j < _l; ++j) {
			word& x = s.wd(i-j);
			p = p->find(x.pos);
		}
		_pos->remove(w.pos, p);
	}
	for (int k = _k; pfreq[k] == 0; --k) {
		_shrink();
	}
}

void phsmm::estimate(int iter) {
	for (int i = 1; i < _k+1; ++i) {
		(*_word)[i]->gibbs(iter);
		(*_word)[i]->estimate(iter);
		(*_letter)[i]->estimate(iter);
	}
	_pos->estimate(iter);
}

void phsmm::poisson_correction(int n) {
	for (int i = 1; i < _k+1; ++i) {
		(*_word)[i]->poisson_correction(n);
	}
}

sentence phsmm::parse(io& f, int i) {
	lattice l(f, i);
	vt dp;
	// slice
	_slice(l);
	// forward filtering
	for (auto t = 0; t < l.w.size(); ++t) {
		for (auto j = 0; j < l.size(t); ++j) {
			/*
			if (l.skip(t,j))
				continue;
				*/
			word& w = l.wd(t, j+1);
			for (auto pt = l.sbegin(t, j); pt != l.send(t, j); ++pt) {
			//for (auto p = 1; p < _k+1; ++p) {
				int p = *pt;
				const context *c = (*_word)[p]->h();
				const context *z = _pos->h();
				for (auto k = 0; k < l.size(t-w.len); ++k) {
					/*
					if (l.skip(t-w.len,k))
						continue;
						*/
					const context *h = NULL;
					word& prev = l.wd(t-w.len, k+1);
					if (_n > 1)
						h = c->find(prev.id);
					// pos transition
					//for (auto q = 1; q < _k+1; ++q) {
					for (auto it = l.sbegin(t-w.len, k); it != l.send(t-w.len, k); ++it) {
						int q = *it;
						const context *u = NULL;
						if (_l > 1)
							u = z->find(q);
						if (h && u)
							_forward(l, t-w.len-prev.len, h, u, w, p, prev, q, dp[t][j][p], dp[t-w.len][k][q], _n-1, _l-1, false, false);
						else if (h)
							_forward(l, t-w.len-prev.len, h, z, w, p, prev, q, dp[t][j][p], dp[t-w.len][k][q], _n-1, _l-1, false, true);
						else if (u)
							_forward(l, t-w.len-prev.len, c, u, w, p, prev, q, dp[t][j][p], dp[t-w.len][k][q], _n-1, _l-1, true, false);
						else
							_forward(l, t-w.len-prev.len, c, z, w, p, prev, q, dp[t][j][p], dp[t-w.len][k][q], _n-1, _l-1, true, true);
					}
				}
			}
		}
	}
	// backward sampling
	sentence s;
	word *w = l.wp(l.w.size(), 1);
	int t = (int)l.w.size()-w->len;
	while (t >= 0) {
		const context *c = (*_word)[w->pos]->h();
		const context *z = _pos->h();
		vector<double> table;
		vector<int> len;
		vector<int> pos;
		for (auto k = 0; k < l.size(t); ++k) {
			/*
			if (l.skip(t, k))
				continue;
				*/
			const context *h = NULL;
			word& prev = l.wd(t, k+1);
			if (_n > 1)
				h = c->find(prev.id);
			//for (auto q = 1; q < _k+1; ++q) {
			for (auto qt = l.sbegin(t, k); qt != l.send(t, k); ++qt) {
				int q = *qt;
				// prev slice
				/*
				if (l.u(t) && (*_word)[q]->lp(prev, (*_word)[q]->h())+_pos->lp(q, _pos->h()) < l.u(t))
					continue;
					*/
				const context *u = NULL;
				int i = table.size();
				table.push_back(1.);
				len.push_back(k+1);
				pos.push_back(q);
				if (_l > 1)
					u = z->find(q);
				if (h && u)
					_backward(l, t-prev.len, h, u, *w, w->pos, prev, q, table[i], dp[t][k][q], _n-1, _l-1, false, false);
				else if (h)
					_backward(l, t-prev.len, h, z, *w, w->pos, prev, q, table[i], dp[t][k][q], _n-1, _l-1, false, true);
				else if (u)
					_backward(l, t-prev.len, c, u, *w, w->pos, prev, q, table[i], dp[t][k][q], _n-1, _l-1, true, false);
				else
					_backward(l, t-prev.len, c, z, *w, w->pos, prev, q, table[i], dp[t][k][q], _n-1, _l-1, true, true);
			}
		}
		int id = rd::best(table);
		w = l.wp(t, len[id]);
		w->pos = pos[id];
		s.w.push_back(*w);
		t -= w->len;
	}
	reverse(s.w.begin(), s.w.end());
	s.n.resize(s.w.size(), 0);
	return s;
}

sentence phsmm::sample(io& f, int i) {
	lattice l(f, i);
	vt dp;
	// slice
	_slice(l);
	// forward filtering
	for (auto t = 0; t < l.w.size(); ++t) {
		for (auto j = 0; j < l.size(t); ++j) {
			/*
			if (l.skip(t,j))
				continue;
				*/
			word& w = l.wd(t, j+1);
			//for (auto p = 1; p < _k+1; ++p) {
			for (auto pt = l.sbegin(t, j); pt != l.send(t, j); ++pt) {
				int p = *pt;
				const context *c = (*_word)[p]->h();
				const context *z = _pos->h();
				for (auto k = 0; k < l.size(t-w.len); ++k) {
					/*
					if (l.skip(t-w.len,k))
						continue;
						*/
					const context *h = NULL;
					word& prev = l.wd(t-w.len, k+1);
					if (_n > 1)
						h = c->find(prev.id);
					// pos transition
					//for (auto q = 1; q < _k+1; ++q) {
					for (auto it = l.sbegin(t-w.len, k); it != l.send(t-w.len, k); ++it) {
						int q = *it;
						const context *u = NULL;
						if (_l > 1)
							u = z->find(q);
						if (h && u)
							_forward(l, t-w.len-prev.len, h, u, w, p, prev, q, dp[t][j][p], dp[t-w.len][k][q], _n-1, _l-1, false, false);
						else if (h)
							_forward(l, t-w.len-prev.len, h, z, w, p, prev, q, dp[t][j][p], dp[t-w.len][k][q], _n-1, _l-1, false, true);
						else if (u)
							_forward(l, t-w.len-prev.len, c, u, w, p, prev, q, dp[t][j][p], dp[t-w.len][k][q], _n-1, _l-1, true, false);
						else
							_forward(l, t-w.len-prev.len, c, z, w, p, prev, q, dp[t][j][p], dp[t-w.len][k][q], _n-1, _l-1, true, true);
					}
				}
			}
		}
	}
	// backward sampling
	sentence s;
	word *w = l.wp(l.w.size(), 1);
	int t = (int)l.w.size()-w->len;
	while (t >= 0) {
		const context *c = (*_word)[w->pos]->h();
		const context *z = _pos->h();
		vector<double> table;
		vector<int> len;
		vector<int> pos;
		for (auto k = 0; k < l.size(t); ++k) {
			/*
			if (l.skip(t, k))
				continue;
				*/
			const context *h = NULL;
			word& prev = l.wd(t, k+1);
			if (_n > 1)
				h = c->find(prev.id);
			//for (auto q = 1; q < _k+1; ++q) {
			for (auto qt = l.sbegin(t, k); qt != l.send(t, k); ++qt) {
				int q = *qt;
				const context *u = NULL;
				int j = table.size();
				table.push_back(1.);
				len.push_back(k+1);
				pos.push_back(q);
				if (_l > 1)
					u = z->find(q);
				if (h && u)
					_backward(l, t-prev.len, h, u, *w, w->pos, prev, q, table[j], dp[t][k][q], _n-1, _l-1, false, false);
				else if (h)
					_backward(l, t-prev.len, h, z, *w, w->pos, prev, q, table[j], dp[t][k][q], _n-1, _l-1, false, true);
				else if (u)
					_backward(l, t-prev.len, c, u, *w, w->pos, prev, q, table[j], dp[t][k][q], _n-1, _l-1, true, false);
				else
					_backward(l, t-prev.len, c, z, *w, w->pos, prev, q, table[j], dp[t][k][q], _n-1, _l-1, true, true);
			}
		}
		int id = rd::ln_draw(table);
		w = l.wp(t, len[id]);
		w->pos = pos[id];
		s.w.push_back(*w);
		t -= w->len;
	}
	reverse(s.w.begin(), s.w.end());
	s.n.resize(s.w.size(), 0);
	return s;
}

void phsmm::_forward(lattice& l, int i, const context *c, const context *t, word& w, int p, word& prev, int q, vt& a, vt& b, int n, int m, bool unk, bool not_exist) {
	if (n <= 1 && m <= 1) {
		a.v = math::lse(a.v, b.v+(*_word)[p]->lp(w, c)+_pos->lp(p, t), !a.is_init());
		if (!a.is_init())
			a.set(true);
	} else {
		for (auto j = 0; j < l.size(i); ++j) {
			/*
			if (l.skip(i, j))
				continue;
				*/
			word& y = l.wd(i, j+1);
			const context *h = NULL;
			if (!unk && n > 1)
				h = c->find(y.id);
			for (auto pt = l.sbegin(i, j); pt != l.send(i, j); ++pt) {
				int r = *pt;
				const context *u = NULL;
				if (!not_exist && m > 1)
					u = t->find(r);
				if (h && u)
					_forward(l, i-y.len, h, u, w, p, y, r, a[prev.len-1][q], b[j][r], n-1, m-1, false, false);
				else if (h)
					_forward(l, i-y.len, h, t, w, p, y, r, a[prev.len-1][q], b[j][r], n-1, m-1, false, true);
				else if (u)
					_forward(l, i-y.len, c, u, w, p, y, r, a[prev.len-1][q], b[j][r], n-1, m-1, true, false);
				else
					_forward(l, i-y.len, c, t, w, p, y, r, a[prev.len-1][q], b[j][r], n-1, m-1, true, true);
			}
		}
	}
}

void phsmm::_backward(lattice& l, int i, const context *c, const context *t, word& w, int p, word& prev, int q, double& lpr, vt& b, int n, int m, bool unk, bool not_exist) {
	if (n <= 1 && m <= 1) {
		lpr = math::lse(lpr, b.v+(*_word)[p]->lp(w, c)+_pos->lp(p, t), (lpr == 1.));
	} else {
		for (auto j = 0; j < l.size(i); ++j) {
			/*
			if (l.skip(i, j))
				continue;
				*/
			word& y = l.wd(i, j+1);
			const context *h = NULL;
			if (!unk && n > 1)
				h = c->find(y.id);
			for (auto pt = l.sbegin(i, j); pt != l.send(i, j); ++pt) {
				int r = *pt;
				const context *u = NULL;
				if (!not_exist && m > 1)
					u = t->find(r);
				if (h && u)
					_backward(l, i-y.len, h, u, w, p, y, r, lpr, b[j][r], n-1, m-1, false, false);
				else if (h)
					_backward(l, i-y.len, h, t, w, p, y, r, lpr, b[j][r], n-1, m-1, false, true);
				else if (u)
					_backward(l, i-y.len, c, u, w, p, y, r, lpr, b[j][r], n-1, m-1, true, false);
				else
					_backward(l, i-y.len, c, t, w, p, y, r, lpr, b[j][r], n-1, m-1, true, true);
			}
		}
	}
}

void phsmm::_slice(lattice& l) {
	beta_distribution be;
	shared_ptr<generator> g = generator::create();
	for (auto t = 0; t < l.w.size(); ++t) {
		// marginarize \sum_k p(c_{t-j+1}^t, k)
		//vector<double> lpw;
		for (auto w = l.w[t].begin(); w != l.w[t].end(); ++w) {
			double z = 0; // p(c_{t-j+1}^t)
			vector<double> table;
			for (auto k = 1; k < _k+1; ++k) {
				double lp = (*_word)[k]->lp(*w, (*_word)[k]->h())+_pos->lp(k, _pos->h());
				z = math::lse(z, lp, (z==0));
				table.push_back(lp);
			}
			// p(k|c_{t-j+1}~t)
			for (auto i = table.begin(); i != table.end(); ++i) {
				*i -= z;
			}
			//lpw.push_back(z);
			// for slice pos
			/*
			for (auto k = 1; k < _k+1; ++k) {
				// p(k|c_{t-j+1}~t)
				double lp = (*_word)[k]->lp(*w, (*_word)[k]->h())+_pos->lp(k, _pos->h())-z;
				table.push_back(lp);
			}
			*/
			//w->pos = rd::ln_draw(table)+1;
			int id = rd::ln_draw(table);
			double mu = log(be(_a, _b))+table[id];
			for (auto i = 0; i < table.size(); ++i) {
				if (table[i] >= mu)
					l.k[t][w->len-1].push_back(i+1);
			}

		}
		/*
		   //uniform_int_distribution<> v(1, l.size(t));
		   //int len = v((*g)()); // for slice words
		   int len = 1+rd::ln_draw(lpw);
		   double nu = log(be(_a, _b))+lpw[len-1];
		   for (auto j = 0; j < lpw.size(); ++j) {
		   if (lpw[j] < nu) {
		   l.check[t][j] = 1;
		   }
		   }
		   */
	}
}

void phsmm::_resize() {
	if (_k+1 > _K)
		return;
	++_k;
	_word->resize(_k+1, shared_ptr<hpyp>(new hpyp(_n)));
	_letter->resize(_k+1, shared_ptr<vpyp>(new vpyp(_m)));
	(*_word)[_k]->set_base((*_letter)[_k].get());
	(*_letter)[_k]->set_v(_v);
}

void phsmm::_shrink() {
	--_k;
	_word->pop_back();
	_letter->pop_back();
}
