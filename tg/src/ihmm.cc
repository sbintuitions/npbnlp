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
}

void ihmm::load(const char *file) {
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
		for (int k = 1; i < _k+1; ++k) {
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
		qfreq[w.pos]++;
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
		qfreq[w.pos]--;
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
	for (int k = _k; pfreq[k] == 0; --k) {
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
}

sentence ihmm::parse(io& f, int i) {
	sentense s;
	return s;
}
