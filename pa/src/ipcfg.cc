#include"ipcfg.h"
#include"cyk.h"

#ifdef _OPENMP
#include<omp.h>
#endif

#define C 50000
#define K 1000

using namespace std;
using namespace npbnlp;

static unordered_map<int, int> tfreq;

ipcfg::ipcfg():_m(20), _k(20),_K(K), _v(C), _a(1), _b(1) {
	_nonterm->set_v(_K);
	for (auto i = 0; i < _k+1; ++i) {
		_word->push_back(shared_ptr<hpyp>(new hpyp(1)));
		_letter->push_back(shared_ptr<vpyp>(new vpyp(_m)));
		(*_letter)[i]->set_v(_v);
		(*_word)[i]->set_base((*_letter)[i].get());
	}
}

ipcfg::ipcfg(int m):_m(m), _k(20), _K(K), _v(C), _a(1), _b(1) {
	_nonterm->set_v(_K);
	for (auto i = 0; i < _k+1; ++i) {
		_word->push_back(shared_ptr<hpyp>(new hpyp(1)));
		_letter->push_back(shared_ptr<vpyp>(new vpyp(_m)));
		(*_letter)[i]->set_v(_v);
		(*_word)[i]->set_base((*_letter)[i].get());
	}
}

ipcfg::~ipcfg() {
}

tree ipcfg::sample(io& f, int i) {
	cyk c(f, i);
	vt dp;
	_slice(c);
	// inside
	int size = c.s.size();
	for (auto j = 0; j < size; ++j) {
		_preterm(c, j, dp[j][j]);
	}
	for (auto l = 1; l < size; ++l) {
		for (auto j = 0; j < size-l; ++j) {
			double mu = c.mu[j][j+l];
			_nonterm(c, j, j+l, dp);
		}
	}
	// tree sampling
	tree t;
	int k = 0; // root
	int id = 0; // root id
	node& root = t[id];
	root.k = k;
	root.i = 0;
	root.j = size-1;
	_traceback(c, 0, c.s.size()-1, k, a, t, id);
	return t;
}

tree ipcfg::parse(io& f, int i) {
	tree t;
	return t;
}

void ipcfg::add(tree& t) {
	lock_guard<mutex> m(_mutex);
	queue<int> q;
	q.push(0);
	while (!q.empty()) {
		int a = q.front();
		q.pop();
		node& p = t[a];
		if (p.i != p.j) {
			q.push(2*a+1);
			q.push(2*a+2);
			node& l = t[2*a+1];
			node& r = t[2*a+2];
			context *h = _nonterm->h();
			h = h->make(r.k);
			h = h->make(l.k);
			_nonterm->add(p.k, h);
			// prior update
			h = _nonterm->h();
			_nonterm->add(l.k, h);
			h = h->make(l.k);
			_nonterm->add(r.k, h);
			tfreq[p.k]++;
			tfreq[l.k]++;
			tfreq[r.k]++;
		} else if (p.k > 0) { // pre terminal
			word& w = t.wd(p.i);
			context *h = (*_word)[p.k]->h();
			(*_word)[p.k]->add(w, h);
			_nonterm->add(p.k, _nonterm->h());
			tfreq[p.k]++;
		}
		if (p.k == _k)
			_resize();
	}
}

void ipcfg::remove(tree& t) {
	lock_guard<mutex> m(_mutex);
	queue<int> q;
	q.push(0);
	while(!q.empty()) {
		int a = q.front();
		q.pop();
		node& p = t[a];
		if (p.i != p.j) {
			q.push(2*a+1);
			q.push(2*a+2);
			node& l = t[2*a+1];
			node& r = t[2*a+2];
			context *h = _nonterm->h();
			h = h->find(r.k);
			h = h->find(l.k);
			_nonterm->remove(p.k, h);
			h = _nonterm->h();
			_nonterm->remove(l.k, h);
			h = h->find(l.k);
			_nonterm->remove(r.k, h);
			tfreq[p.k]--;
			tfreq[l.k]--;
			tfreq[r.k]--;
		} else if (p.k > 0) {
			word& w = t.wd(p.i);
			context *h = (*_word)[p.k]->h();
			(*_word)[p.k]->remove(w, h);
			_nonterm->remove(p.k, _nonterm->h());
			tfreq[p.k]--;
		}
	}
	for (int k = _k; tfreq[k] == 0; --k) {
		_shrink();
	}
}

void ipcfg::estimate(int iter) {
	for (int i = 1; i < _k+1; ++i) {
		(*_word)[i]->gibbs(iter);
		(*_word)[i]->estimate(iter);
		(*_letter)[i]->estimate(iter);
	}
	_nonterm->estimate(iter);
}

void ipcfg::poisson_correction(int n) {
	for (int i = 1; i < _k+1; ++i) {
		(*_word)[i]->poisson_correction(n);
	}
}

void ipcfg::set(int v, int k) {
	_v = v;
	_K = k;
	_k = min(_k, _K);
	for (auto it = _letter->begin(); it != _letter->end(); ++it) {
		(*it)->set_v(_v);
	}
	_nonterm->set_v(k);
}

void ipcfg::slice(double a, double b) {
	if (a <= 0 || b <= 0) {
		return;
	}
	_a = a;
	_b = b;
}

void ipcfg::_traceback(cyk& c, int i, int j, int z, vt& a, tree& tr, int parent) {
	double mu = c.mu[i][j];
	if (i == j) { // pre-terminal
		vector<double> table;
		vector<int> pt;
		word& w = c.wd(i);
		for (auto k = c.begin(i,i); k != c.end(i,i); ++k) {
			double lp = (*_word)[*k]->lp(w, (*_word)[*k]->h())+_nonterm->lp(*k, _nonterm->h());
			table.push_back(lp);
			pt.push_back(*k);
		}
		int id = rd::ln_draw(table);
		int k = pt[id];
		node& n = tr[2*parent+1];
		n.k = k;
		n.i = i;
		n.j = i;
	} else { // non-terminal
		vector<double> table;
		vector<int> left;
		vector<int> right;
		vector<int> brp; // break point
		for (auto k = i; k < j; ++k) {
			for (auto l = c.begin(i,k); l != c.end(i,k); ++l) {
				double lp_l = _nonterm->lp(*l, _nonterm->h());
				context *h = _nonterm->h();
				context *t = h->find(*l);
				if (t)
					h = t;
				for (auto r = c.begin(k+1,j); r != c.end(k+1,j); ++r) {
					double lp_r = _nonterm->lp(*r, h);
					context *s = _nonterm->h();
					context *u = h->find(*r);
					if (u) {
						s = u;
						u = s->find(*l);
						if (u)
							s = u;
					}
					double lp = _nonterm->lp(z,s)+lp_l+lp_r;
					if (lp < mu)
						continue;
					table.push_back(lp+a[i][k][*l].v+a[k+1][j][*r].v);
					left.push_back(*l);
					right.push_back(*r);
					brp.push_back(k);
				}
			}
		}
		node& ln = tr[2*parent+1];
		node& rn = tr[2*parent+2];
		int id = rd::ln_draw(table);
		int b = brp[id];
		ln.k = left[id];
		ln.i = i;
		ln.j = b;
		rn.k = right[id];
		rn.i = b+1;
		rn.j = j;
		_traceback(c, i, b, l, a, tr, 2*parent+1);
		_traceback(c, b+1, j, r, a, tr, 2*parent+2);
	}
}

void ipcfg::_preterm(cyk& c, int j, vt& a) {
	word& w = c.wd(j);
	double mu = c.mu[j][j];
	for (auto k = c.begin(j,j); k != c.end(j,j); ++k) {
		double lp = (*_word)[*k]->lp(w, (*_word)[*k]->h()+_nonterm->lp(*k, _nonterm->h()));
		if (lp >= mu) {
			a[*k].v = math::lse(a[*k].v, lp, true);
			a[*k].set(true);
		}
	}
}

void ipcfg::_nonterm(cyk& c, int i, int j, vt& a) {
	double mu = c.mu[i][j];
	for (auto k = i; k < j; ++k) {
		for (auto l = c.begin(i,k); l != c.end(i,k); ++l) {
			double lp_l = _nonterm->lp(*l, _nonterm->h());
			context *h = _nonterm->h();
			context *t = h->find(*l);
			if (t)
				h = t;
			for (auto r = c.begin(k+1,j); r != c.end(k+1,j); ++r) {
				double lp_r = _nonterm->lp(*r, h);
				context *s = _nonterm->h();
				context *u = s->find(*r);
				if (u) {
					s = u;
					u = s->find(*l);
					if (u)
						s = u;
				}
				for (auto z = c.begin(i,j); z != c.end(i,j); ++z) {
					double lp = _nonterm->lp(*z,s)+lp_l+lp_r;
					if (lp < mu)
						continue;
					a[i][j][*z].v = math::lse(a[i][j][*z].v, lp+a[i][k][*l].v+a[k+1][j][*r].v, !a[i][j].is_init());
					if (!a[i][j].is_init())
						a[i][j].set(true);
				}
			}
		}
	}
}

void ipcfg::_slice(cyk& l) {
	// terminal
	for (auto i = 0; i < l.s.size(); ++i) {
		_slice_preterm(l, i);
	}
	// non terminal
	for (auto m = 1; m < l.s.size()-1; ++m) {
		for (auto i = 0; i < l.s.size()-m; ++i) {
			_slice_nonterm(l, i, i+m);
		}
	}
	// root
	_slice_root(l);
}

void ipcfg::_slice_preterm(cyk& l, int i) {
	beta_distribution be;
	shared_ptr<generator> g = generator::create();
	word& w = l.wd(i);
	vector<double> table;
	for (auto k = 1; k < _k+1; ++k) {
		double lp = (*_word)[k]->lp(w, (*_word)[k]->h())+_nonterm->lp(k, _nonterm->h());
		table.push_back(lp);
	}
	int id = rd::ln_draw(table);
	double mu = log(be(_a, _b))+table[id];
	l.mu[i][i] = mu;
	for (auto j = 0; j < table.size(); ++j) {
		if (table[i] >= mu)
			l.k[i][i].insert(j+1);
	}
}

void ipcfg::_slice_nonterm(cyk& c, int i, int j) {
	beta_distribution be;
	shared_ptr<generator> g = generator::create();
	vector<double> table;
	vector<int> z;
	for (auto k = i; k < j; ++k) {
		for (auto l = c.begin(i, k); l != c.end(i,k); ++l) {
			double lp_l = _nonterm->lp(*l, _nonterm->h());
			context *h = _nonterm->h();
			context *t = h->find(*l);
			if (t)
				h = t;
			for (auto r = c.begin(k+1, j); r != c.end(k+1,j); ++r) {
				double lp_r = _nonterm->lp(*r, h);
				context *s = _nonterm->h();
				context *u = s->find(*r);
				if (u) {
					s = u;
					u = s->find(*l);
					if (u)
						s = u;
				}
				for (auto m = 1; m < _k+1; ++m) {
					double lp = _nonterm->lp(m, s)+lp_l+lp_r;
					table.push_back(lp);
					z.push_back(m);
				}
			}
		}
	}
	// P(B,C|A) := P(A->B C|A)
	// P(B,C|A) \propto P(B,C,A) = P(A|B,C)P(B,C)
	// draw A ~ P(A,B,C) for slice at cell_{i,j}
	int id = rd::ln_draw(table);
	double mu = log(be(_a, _b))+table[id];
	c.mu[i][j] = mu;
	for (auto m = 0; m < table.size(); ++m) {
		if (table[m] >= mu)
			c.k[i][j].insert(z[m]);
	}
}

void ipcfg::_slice_root(cyk& c) {
	beta_distribution be;
	shared_ptr<generator> g = generator::create();
	int size = c.s.size();
	vector<double> table;
	for (auto k = 0; k < size-1; ++k) {
		for (auto l = c.begin(0, k); l != c.end(0, k); ++l) {
			double lp_l = _nonterm->lp(*l, _nonterm->h());
			context *h = _nonterm->h();
			context *t = h->find(*l);
			if (t)
				h = t;
			for (auto r = c.begin(k+1, size-1); r != c.end(k+1, size-1); ++r) {
				double lp_r = _nonterm->lp(*r, h);
				context *s = _nonterm->h();
				context *u = s->find(*r);
				if (u) {
					s = u;
					u = s->find(*l);
					if (u)
						s = u;
				}
				double lp = _nonterm->lp(0, s)+lp_l+lp_r;
				table.push_back(lp);
			}
		}
	}
	int id = rd::ln_draw(table);
	double mu = log(be(_a, _b)+table[id]);
	c.mu[0][size-1] = mu;
	c.k[0][size-1].insert(0);
	/*
	   for (auto m = 0; m < table.size(); ++m) {
	   if (table[m] >= mu)
	   c.k[0][size-1][0]+=1;
	   }
	   */
}

void ipcfg::_resize() {
	if (_k+1 > _K)
		return;
	++_k;
	_word->resize(_k+1, shared_ptr<hpyp>(new hpyp(1)));
	_letter->resize(_k+1, shared_ptr<vpyp>(new vpyp(_m)));
	(*_word)[_k]->set_base((*_letter)[_k].get());
	(*_letter)[_k]->set_v(_v);
}

void ipcfg::_shrink() {
	--_k;
	_word->pop_back();
	_letter->pop_back();
}
