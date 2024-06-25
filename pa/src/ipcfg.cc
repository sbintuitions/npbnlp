#include"ipcfg.h"
#include"cyk.h"
#include"rd.h"
#include"convinience.h"
#include"generator.h"
#include<queue>

#ifdef _OPENMP
#include<omp.h>
#endif

#define C 50000
#define K 1000

using namespace std;
using namespace npbnlp;

static unordered_map<int, int> tfreq;

ipcfg::ipcfg():_m(20), _k(20),_K(K), _v(C), _a(1), _b(1), _nonterm(new hpyp(3)),_word(new vector<shared_ptr<hpyp> >),_letter(new vector<shared_ptr<vpyp> >) {
	_nonterm->set_v(_K);
	for (auto i = 0; i < _k+1; ++i) {
		_word->push_back(shared_ptr<hpyp>(new hpyp(1)));
		_letter->push_back(shared_ptr<vpyp>(new vpyp(_m)));
		(*_letter)[i]->set_v(_v);
		(*_word)[i]->set_base((*_letter)[i].get());
	}
}

ipcfg::ipcfg(int m):_m(m), _k(20), _K(K), _v(C), _a(1), _b(1), _nonterm(new hpyp(3)), _word(new vector<shared_ptr<hpyp> >), _letter(new vector<shared_ptr<vpyp> >) {
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

void ipcfg::save(const char *f) {
	FILE *fp = NULL;
	if ((fp = fopen(f, "wb")) == NULL)
		throw "failed to open save file in ipcfg::save";
	try {
		if (fwrite(&_m, sizeof(int), 1, fp) != 1)
			throw "failed to write _m in ipcfg::save";
		if (fwrite(&_k, sizeof(int), 1, fp) != 1)
			throw "failed to write _k in ipcfg::save";
		if (fwrite(&_K, sizeof(int), 1, fp) != 1)
			throw "failed to write _K in ipcfg::save";
		if (fwrite(&_v, sizeof(int), 1, fp) != 1)
			throw "failed to write _v in ipcfg::save";
		_nonterm->save(fp);
		for (auto i = 0; i < _k+1; ++i) {
			(*_word)[i]->save(fp);
			(*_letter)[i]->save(fp);
		}
	} catch (const char *ex) {
		throw ex;
	}
	fclose(fp);
}

void ipcfg::load(const char *f) {
	FILE *fp = NULL;
	if ((fp = fopen(f, "rb")) == NULL)
		throw "failed to open save file in ipcfg::load";
	try {
		if (fread(&_m, sizeof(int), 1, fp) != 1)
			throw "failed to read _m in ipcfg::load";
		if (fread(&_k, sizeof(int), 1, fp) != 1)
			throw "failed to read _k in ipcfg::load";
		if (fread(&_K, sizeof(int), 1, fp) != 1)
			throw "failed to read _K in ipcfg::load";
		if (fread(&_v, sizeof(int), 1, fp) != 1)
			throw "failed to read _v in ipcfg::load";
		_nonterm->load(fp);
		while ((int)_word->size() < _k+1) {
			_word->push_back(shared_ptr<hpyp>(new hpyp(1)));
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

tree ipcfg::sample(io& f, int i) {
	cyk c(f, i);
	if (c.s.size() == 1) {
		tree t(c.s);
		t[0].k = 0;
		t[0].i = 0;
		t[0].j = 0;
		return t;
	}
	vt dp;
	_slice(c);
	// inside
	int size = c.s.size();
	for (auto j = 0; j < size; ++j) {
		_calc_preterm(c, j, dp[j][j]);
	}
	for (auto l = 1; l < size; ++l) {
		for (auto j = 0; j < size-l; ++j) {
			//double mu = c.mu[j][j+l];
			_calc_nonterm(c, j, j+l, dp);
		}
	}
	// tree sampling
	tree t(c.s);
	int k = 0; // root
	int id = t.s.size()-1; // root id
	node& root = t[id];
	root.k = k;
	root.i = 0;
	root.j = size-1;
	_traceback(c, 0, t.s.size()-1, k, dp, t);
	return t;
}

tree ipcfg::parse(io& f, int i) {
	cyk c(f, i);
	if (c.s.size() == 1) {
		tree t(c.s);
		t[0].k = 0;
		t[0].i = 0;
		t[0].j = 0;
		return t;
	}
	vt dp;
	_slice(c);
	// inside
	int size = c.s.size();
	for (auto j = 0; j < size; ++j) {
		_calc_preterm(c, j, dp[j][j]);
	}
	for (auto l = 1; l < size; ++l) {
		for (auto j = 0; j < size-l; ++j) {
			//double mu = c.mu[j][j+l];
			_calc_nonterm(c, j, j+l, dp);
		}
	}
	// tree sampling
	tree t(c.s);
	int k = 0; // root
	int id = t.s.size()-1; // root id
	node& root = t[id];
	root.k = k;
	root.i = 0;
	root.j = size-1;
	_traceback(c, 0, t.s.size()-1, k, dp, t, true);
	return t;
}

void ipcfg::add(tree& t) {
	lock_guard<mutex> m(_mutex);
	_add(t, t.s.size()-1);
	if (tfreq[_k] > 0)
		_resize();
}

void ipcfg::_add(tree& t, int i) {
	node& z = t[i];
	tfreq[z.k]++;
	if (z.i != z.j) { // nonterminal
		node& left = t[t.s.size()*z.i+z.b-z.i*(1.+z.i)/2];
		node& right = t[t.s.size()*(z.b+1)+z.j-(1.+z.b)*(z.b+2)/2];
		context *h = _nonterm->h();
		h = h->make(right.k);
		h = h->make(left.k);
		_nonterm->add(z.k, h);
		h = _nonterm->h();
		_nonterm->add(left.k, h);
		h = h->make(left.k);
		_nonterm->add(right.k, h);
		_add(t, t.s.size()*z.i+z.b-z.i*(1.+z.i)/2);
		_add(t, t.s.size()*(z.b+1)+z.j-(1.+z.b)*(z.b+2)/2);
	} else if (z.k > 0) { // preterminal
		word& w = t.wd(z.i);
		context *h = (*_word)[z.k]->h();
		(*_word)[z.k]->add(w, h);
		_nonterm->add(z.k, _nonterm->h());
	}
}

void ipcfg::remove(tree& t) {
	lock_guard<mutex> m(_mutex);
	_remove(t, t.s.size()-1);
	for (int k = _k-1; tfreq[k] == 0; --k) {
		_shrink();
	}
}

void ipcfg::_remove(tree& t, int i) {
	node& z = t[i];
	tfreq[z.k]--;
	if (z.i != z.j) { // nonterminal
		node& left = t[t.s.size()*z.i+z.b-z.i*(1.+z.i)/2];
		node& right = t[t.s.size()*(z.b+1)+z.j-(1.+z.b)*(z.b+2)/2];
		context *h = _nonterm->h();
		h = h->find(right.k);
		h = h->find(left.k);
		_nonterm->remove(z.k, h);
		h = _nonterm->h();
		_nonterm->remove(left.k, h);
		h = h->find(left.k);
		_nonterm->remove(right.k, h);
		_remove(t, t.s.size()*z.i+z.b-z.i*(1.+z.i)/2);
		_remove(t, t.s.size()*(z.b+1)+z.j-(1.+z.b)*(z.b+2)/2);
	} else if (z.k > 0) { // preterminal
		word& w = t.wd(z.i);
		context *h = (*_word)[z.k]->h();
		(*_word)[z.k]->remove(w, h);
		_nonterm->remove(z.k, _nonterm->h());
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

void ipcfg::_traceback(cyk& c, int i, int j, int z, vt& a, tree& tr, bool best) {
	double mu = c.mu[i][j];
	if (i == j) { // pre-terminal
		return;
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
					if (z > max(*l, *r))
						continue;
					double lp_r = _nonterm->lp(*r, h);
					context *s = _nonterm->h();
					context *u = s->find(*r);
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
		int id = 0;
		if (best)
			id = rd::best(table);
		else
			id = rd::ln_draw(table);
		int b = brp[id];
		node& n = tr[tr.s.size()*i+j-i*(1.+i)/2];
		n.b = b;
		node& ln = tr[tr.s.size()*i+b-i*(1.+i)/2];
		node& rn = tr[tr.s.size()*(b+1)+j-(b+1.)*(2.+b)/2];
		ln.k = left[id];
		ln.i = i;
		ln.j = b;
		rn.k = right[id];
		rn.i = b+1;
		rn.j = j;
		_traceback(c, i, b, ln.k, a, tr);
		_traceback(c, b+1, j, rn.k, a, tr);
	}
}

void ipcfg::_calc_preterm(cyk& c, int j, vt& a) {
	word& w = c.wd(j);
	double mu = c.mu[j][j];
	for (auto k = c.begin(j,j); k != c.end(j,j); ++k) {
		double lp = (*_word)[*k]->lp(w, (*_word)[*k]->h())+_nonterm->lp(*k, _nonterm->h());
		if (lp >= mu) {
			a[*k].v = math::lse(a[*k].v, lp, true);
			a[*k].set(true);
		}
	}
}

void ipcfg::_calc_nonterm(cyk& c, int i, int j, vt& a) {
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
					if (*z > max(*l, *r))
						continue;
					double lp = _nonterm->lp(*z,s)+lp_l+lp_r;
					if (lp < mu)
						continue;
					a[i][j][*z].v = math::lse(a[i][j][*z].v, lp+a[i][k][*l].v+a[k+1][j][*r].v, !a[i][j][*z].is_init());
					if (!a[i][j][*z].is_init())
						a[i][j][*z].set(true);
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
	vector<double> table;
	for (auto i = 0; i < l.s.size()-1; ++i) {
		table.push_back(_marginalize(l,i,i+1));
	}
	int p = rd::ln_draw(table);
	//shared_ptr<generator> g = generator::create();
	//uniform_int_distribution<> u(0, l.s.size()-2);
	//int p = u((*g)());
	//non terminal
	for (auto m = 1; m < l.s.size()-1; ++m) {
		//int len = l.s.size()-m;
		//uniform_int_distribution<> v(0, len-1);
		//int p = v((*g)());
		// draw nu for slice non-terminal
		//double nu = _draw(l, p, p+m);
		double nu = _draw(l, p, p+m);
		// slice non-terminals
		for (auto i = 0; i < l.s.size()-m; ++i) {
			if (i == p)
				continue;
			_slice_nonterm(l, i, i+m, nu);
		}
		vector<double> cand;
		if (p > 0) {
			cand.push_back(_marginalize(l,p-1,p+m));
		}
		if (p+m < l.s.size()-1) {
			cand.push_back(_marginalize(l,p,p+m+1));
		}
		if (cand.size() > 1) {
			int id = rd::ln_draw(cand);
			if (id == 0) {
				p -= 1;
			}
		} else if (p > 0) {
			p -= 1;
		}
	}
	// root
	_slice_root(l);
}

double ipcfg::_marginalize(cyk& c, int i, int j) {
	double z = 0;
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
				for (auto m = max(*l,*r); m > 0; --m) {
					double lp =_nonterm->lp(m,s)+lp_l+lp_r;
					math::lse(z,lp,(z==0.));
				}
			}
		}
	}
	return z;
}

double ipcfg::_draw(cyk& c, int i, int j) {
	beta_distribution be;
	vector<double> table;
	vector<int> z;
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
				for (auto m = max(*l,*r); m > 0; --m) {
					double lp = _nonterm->lp(m,s)+lp_l+lp_r;
					table.push_back(lp);
					z.push_back(m);
				}
			}
		}
	}
	int id = rd::ln_draw(table);
	double mu = log(be(_a,_b))+table[id];
	c.mu[i][j] = mu;
	for (auto m = 0; m < (int)table.size(); ++m) {
		if (table[m] >= mu)
			c.k[i][j].insert(z[m]);
	}
	return mu;
}

void ipcfg::_slice_nonterm(cyk& c, int i, int j, double mu) {
	vector<double> table;
	vector<int> z;
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
				for (auto m = max(*l, *r); m > 0; --m) {
					double lp = _nonterm->lp(m,s)+lp_l+lp_r;
					table.push_back(lp);
					z.push_back(m);
				}
			}
		}
	}
	// P(B,C|A) := P(A->B C|A)
	// P(B,C|A) \propto P(B,C,A) = P(A|B,C)P(B,C)
	c.mu[i][j] = mu;
	for (auto m = 0; m < (int)table.size(); ++m) {
		if (table[m] >= mu) {
			c.k[i][j].insert(z[m]);
		}
	}
}
/*
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
*/

void ipcfg::_slice_preterm(cyk& l, int i) {
	beta_distribution be;
	//shared_ptr<generator> g = generator::create();
	word& w = l.wd(i);
	vector<double> table;
	for (auto k = 1; k < _k+1; ++k) {
		double lp = (*_word)[k]->lp(w, (*_word)[k]->h())+_nonterm->lp(k, _nonterm->h());
		table.push_back(lp);
	}
	int id = rd::ln_draw(table);
	double mu = log(be(_a, _b))+table[id];
	//double mu = table[id];
	l.mu[i][i] = mu;
	for (auto j = 0; j < (int)table.size(); ++j) {
		if (table[j] >= mu) {
			l.k[i][i].insert(j+1);
		}
	}
}

/*
   void ipcfg::_slice_nonterm(cyk& c, int i, int j) {
   beta_distribution be;
//shared_ptr<generator> g = generator::create();
vector<double> table;
vector<int> z;
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
//for (auto m = 1; m < _k+1; ++m) {
for (auto m = max(*l,*r); m > 0; --m) {
double lp = _nonterm->lp(m, s)+lp_l+lp_r;
table.push_back(lp);
z.push_back(m);
}
}
}
}
// P(B,C|A) := P(A->B C|A)
// P(B,C|A) \propto P(B,C,A) = P(A|B,C)P(B,C)
// draw A ~ P(A,B,C) for a threshold at cell_{i,j}
int id = rd::ln_draw(table);
double mu = log(be(_a, _b))+table[id];
//double mu = table[id];
c.mu[i][j] = mu;
for (auto m = 0; m < (int)table.size(); ++m) {
if (table[m] >= mu) {
c.k[i][j].insert(z[m]);
}
}
}
*/

void ipcfg::_slice_root(cyk& c) {
	beta_distribution be;
	//shared_ptr<generator> g = generator::create();
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
