#include"ihmm.h"
#include"convinience.h"
#include"rd.h"
#include"vtable.h"
#include"hlattice.h"
#include"generator.h"
#include<random>
#ifdef _OPENMP
#include<omp.h>
#endif

#define C 50000
#define K 1000

using namespace std;
using namespace npbnlp;

static unordered_map<int, int> wfreq;
static unordered_map<int, int> pfreq;

ihmm::ihmm():_n(2),_m(10),_v(C),_k(10),_K(K),_a(1),_b(1),_pos(new hpyp(_n)),_word(new vector<shared_ptr<hpyp> >),_letter(new vector<shared_ptr<vpyp> >) {
	_pos->set_v(_K);
	for (auto i = 0; i < _k+1; ++i) {
		_word->push_back(shared_ptr<hpyp>(new hpyp(1)));
		_letter->push_back(shared_ptr<vpyp>(new vpyp(_m)));
		(*_letter)[i]->set_v(_v);
		(*_word)[i]->set_base((*_letter)[i].get());
	}
}

ihmm::ihmm(int n, int m, int k):_n(n),_m(m),_v(C),_k(k),_K(K),_a(1),_b(1),_pos(new hpyp(_n)),_word(new vector<shared_ptr<hpyp> >),_letter(new vector<shared_ptr<vpyp> >) {
	_pos->set_v(_K);
	for (auto i = 0; i < _k+1; ++i) {
		_word->push_back(shared_ptr<hpyp>(new hpyp(1)));
		_letter->push_back(shared_ptr<vpyp>(new vpyp(_m)));
		(*_letter)[i]->set_v(_v);
		(*_word)[i]->set_base((*_letter)[i].get());
	}
}

ihmm::~ihmm() {
}

void ihmm::set(int v, int k) {
	_v = v;
	_K = k;
	_k = min(_k, _K);
	for (auto it = _letter->begin(); it != _letter->end(); ++it) {
		(*it)->set_v(_v);
	}
	_pos->set_v(_K);
}

int ihmm::n() {
	return _n;
}

int ihmm::m() {
	return _m;
}

int ihmm::k() {
	return _k;
}

void ihmm::save(const char *file) {
	FILE *fp = NULL;
	if ((fp = fopen(file, "wb")) == NULL)
		throw "failed to open save file in ihmm::save";
	try {
		if (fwrite(&_n, sizeof(int), 1, fp) != 1)
			throw "failed to write _n in ihmm::save";
		if (fwrite(&_m, sizeof(int), 1, fp) != 1)
			throw "failed to write _m in ihmm::save";
		if (fwrite(&_v, sizeof(int), 1, fp) != 1)
			throw "failed to write _v in ihmm::save";
		if (fwrite(&_k, sizeof(int), 1, fp) != 1)
			throw "failed to write _k in ihmm::save";
		if (fwrite(&_K, sizeof(int), 1, fp) != 1)
			throw "failed to write _K in ihmm::save";
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

void ihmm::load(const char *file) {
	FILE *fp = NULL;
	if ((fp = fopen(file, "rb")) == NULL)
		throw "failed to open save file in ihmm::load";
	try {
		if (fread(&_n, sizeof(int), 1, fp) != 1)
			throw "failed to read _n in ihmm::load";
		if (fread(&_m, sizeof(int), 1, fp) != 1)
			throw "failed to read _m in ihmm::load";
		if (fread(&_v, sizeof(int), 1, fp) != 1)
			throw "failed to read _v in ihmm::load";
		if (fread(&_k, sizeof(int), 1, fp) != 1)
			throw "failed to read _k in ihmm::load";
		if (fread(&_K, sizeof(int), 1, fp) != 1)
			throw "failed to read _K in ihmm::load";
		_pos->load(fp);
		for (auto i = 0; i < _k+1; ++i) {
			_word->push_back(shared_ptr<hpyp>(new hpyp(1)));
			_letter->push_back(shared_ptr<vpyp>(new vpyp(_m)));
			(*_word)[i]->load(fp);
			(*_letter)[i]->load(fp);
			(*_word)[i]->set_base((*_letter)[i].get());
		}
	} catch (const char *ex) {
		throw ex;
	}
	fclose(fp);
}

void ihmm::slice(double a, double b) {
	if (a <= 0 || b <= 0) {
		return;
	}
	_a = a;
	_b = b;
}

void ihmm::init(sentence& s) {
	lock_guard<mutex> m(_mutex);
	shared_ptr<wid> dic = wid::create();
	for (int i = 0; i < s.size(); ++i) {
		word& x = s.wd(i);
		if ((*dic)[x] == 1) {
			x.id = dic->index(x);
		} else {
			x.id = (*dic)[x];
		}
		wfreq[x.id]++;
		context *p = _pos->h();
		for (int j = 1; j < _n; ++j) {
			word& w = s.wd(i-j);
			p = p->make(w.pos);
		}
		vector<double> table;
		for (int k = 1; k < _k+1; ++k) {
			const context *c = (*_word)[k]->h();
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
	(*_word)[0]->add(s.wd(s.size()), h);
	context *p = _pos->h();
	int eos = s.size();
	for (int j = 1; j < _n; ++j) {
		word& w = s.wd(eos-j);
		p = p->make(w.pos);
	}
	_pos->add(s.wd(s.size()).pos, p);
}

void ihmm::add(sentence& s) {
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
		context *p = _pos->h();
		for (int j = 1; j < _n; ++j) {
			word& x = s.wd(rd[i]-j);
			p = p->make(x.pos);
		}
		_pos->add(w.pos, p);
		if (w.pos == _k)
			_resize();
	}
}

void ihmm::remove(sentence& s) {
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
		context *p = _pos->h();
		for (int j = 1; j < _n; ++j) {
			word& x = s.wd(i-j);
			p = p->find(x.pos);
		}
		_pos->remove(w.pos, p);
	}
	for (int k = _k-1; pfreq[k] == 0; --k) {
		_shrink();
	}
}

void ihmm::estimate(int iter) {
	for (int i = 1; i < _k+1; ++i) {
		(*_word)[i]->gibbs(iter);
		(*_word)[i]->estimate(iter);
		(*_letter)[i]->estimate(iter);
	}
	_pos->estimate(iter);
}

void ihmm::poisson_correction(int n) {
	for (int i = 1; i < _k+1; ++i) {
		(*_word)[i]->poisson_correction(n);
	}
}

sentence ihmm::sample(io& f, int i) {
	hlattice l(f, i);
	vt dp;
	_slice(l);
	for (auto t = 0; t < (int)l.k.size(); ++t) {
		for (auto it = l.begin(t); it!= l.end(t); ++it) {
			int k = *it;
			const context *h = _pos->h();
			double mu = l.u(t);
			for (auto jt = l.begin(t-1); jt != l.end(t-1); ++jt) {
				const context *g = NULL;
				if (_n > 1)
					g = h->find(*jt);
				if (g)
					_forward(l, t-1, mu, g, k, l.s.wd(t), *jt, dp[t][k], dp[t-1][*jt], _n-1, false);
				else
					_forward(l, t-1, mu, h, k, l.s.wd(t), *jt, dp[t][k], dp[t-1][*jt], _n-1, true);
			}
		}
	}
	int k = 0; // eos
	double mu = l.u(l.k.size());
	int t = l.k.size();
	sentence s(l.s);
	while (t >= 0) {
		const context *h = _pos->h();
		vector<double> table;
		vector<int> pos;
		for (auto jt = l.begin(t-1); jt != l.end(t-1); ++jt) {
			const context *g = NULL;
			if (_n > 1)
				g = h->find(*jt);
			int j = table.size();
			table.push_back(1.);
			pos.push_back(*jt);
			if (g)
				_backward(l, t-1, mu, g, l.s.wd(t), k, *jt, table[j], dp[t][*jt], _n-1, false);
			else
				_backward(l, t-1, mu, h, l.s.wd(t), k, *jt, table[j], dp[t][*jt], _n-1, true);
		}
		--t;
		int id = rd::ln_draw(table);
		k = pos[id];
		s.wd(t).pos = k;
		mu = l.u(t);
	}
	//sentence s(l.s);
	return s;
}

sentence ihmm::parse(io& f, int i) {
	hlattice l(f, i);
	vt dp;
	_slice(l, true);
	for (auto t = 0; t < (int)l.k.size(); ++t) {
		for (auto it = l.begin(t); it!= l.end(t); ++it) {
			int k = *it;
			const context *h = _pos->h();
			double mu = l.u(t);
			for (auto jt = l.begin(t-1); jt != l.end(t-1); ++jt) {
				const context *g = NULL;
				if (_n > 1)
					g = h->find(*jt);
				if (g)
					_forward(l, t-1, mu, g, k, l.s.wd(t), *jt, dp[t][k], dp[t-1][*jt], _n-1, false);
				else
					_forward(l, t-1, mu, h, k, l.s.wd(t), *jt, dp[t][k], dp[t-1][*jt], _n-1, true);
			}
		}
	}
	int k = 0; // eos
	double mu = l.u(l.k.size());
	int t = l.k.size();
	while (t >= 0) {
		const context *h = _pos->h();
		vector<double> table;
		vector<int> pos;
		for (auto jt = l.begin(t-1); jt != l.end(t-1); ++jt) {
			const context *g = NULL;
			if (_n > 1)
				g = h->find(*jt);
			int j = table.size();
			table.push_back(1.);
			pos.push_back(*jt);
			if (g)
				_backward(l, t-1, mu, g, l.s.wd(t), k, *jt, table[j], dp[t][*jt], _n-1, false);
			else
				_backward(l, t-1, mu, h, l.s.wd(t), k, *jt, table[j], dp[t][*jt], _n-1, true);
		}
		--t;
		int id = rd::best(table);
		k = pos[id];
		l.s.wd(t).pos = k;
		mu = l.u(t);
	}
	sentence s(l.s);
	return s;
}

void ihmm::_forward(hlattice& l, int i, double mu, const context *c, int k, word& w, int p, vt& a, vt& b, int n, bool not_exist) {
	if (n <= 1) {
		if (_pos->lp(k, c) < mu)
			return;
		a.v = math::lse(a.v, b.v+(*_word)[k]->lp(w, (*_word)[k]->h())+_pos->lp(k, c), !a.is_init());
		if (!a.is_init())
			a.set(true);
	} else {
		for (auto jt = l.begin(i-1); jt != l.end(i-1); ++jt) {
			const context *g = NULL;
			if (!not_exist && n > 1)
				g = c->find(*jt);
			if (g)
				_forward(l, i-1, mu, g, k, w, *jt, a[p], b[*jt], n-1, false);
			else
				_forward(l, i-1, mu, c, k, w, *jt, a[p], b[*jt], n-1, true);
		}
	}
}

void ihmm::_backward(hlattice& l, int i, double mu, const context *c, word& w, int k, int p, double& lpr, vt& b, int n, bool not_exist) {
	if (n <= 1) {
		if (_pos->lp(k, c) < mu)
			return;
		lpr = math::lse(lpr, b.v+(*_word)[k]->lp(w, (*_word)[k]->h())+_pos->lp(k, c), (lpr == 1.));
	} else {
		for (auto jt = l.begin(i-1); jt != l.end(i-1); ++jt) {
			const context *g = NULL;
			if (!not_exist && n > 1)
				g = c->find(*jt);
			if (g)
				_backward(l, i-1, mu, g, w, k, *jt, lpr, b[p], n-1, false);
			else
				_backward(l, i-1, mu, c, w, k, *jt, lpr, b[p], n-1, true);
		}
	}
}

void ihmm::_slice(hlattice& l, bool best) {
	beta_distribution be;
	shared_ptr<generator> g = generator::create();
	for (auto t = 0; t < (int)l.k.size(); ++t) {
		context *h = _pos->h();
		for (auto j = 1; j < _n; ++j) {
			word& w = l.s.wd(t-j);
			context *u = h->find(w.pos);
			if (!u)
				break;
			h = u;
		}
		word& wd = l.s.wd(t);
		double z = 0;
		vector<double> table;
		for (auto k = 1; k < _k+1; ++k) {
			double lp = (*_word)[k]->lp(wd, (*_word)[k]->h())+_pos->lp(k, h);
			z = math::lse(z, lp, (z==0));
			table.push_back(lp);
		}
		for (auto i = table.begin(); i != table.end(); ++i) {
			*i -= z;
		}
		int id = 0;
		if (best)
			id = rd::best(table);
		else
			id = rd::ln_draw(table);
		double mu = log(be(_a, _b))+table[id];
		l.slice(t, mu);
		wd.pos = id+1;
		for (auto i = 0; i < (int)table.size(); ++i) {
			if (table[i] >= mu)
				l.k[t].push_back(i+1);
		}
	}
}

void ihmm::_resize() {
	if (_k+1 > _K)
		return;
	++_k;
	_word->resize(_k+1, shared_ptr<hpyp>(new hpyp(1)));
	_letter->resize(_k+1, shared_ptr<vpyp>(new vpyp(_m)));
	(*_word)[_k]->set_base((*_letter)[_k].get());
	(*_letter)[_k]->set_v(_v);
}

void ihmm::_shrink() {
	--_k;
	_word->pop_back();
	_letter->pop_back();
}
