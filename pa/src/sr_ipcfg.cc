#include"sr_ipcfg.h"
#include"cyk.h"
#include"rd.h"
#include"convinience.h"
#include"generator.h"
#include<queue>
#include<stack>

#define C 50000
#define K 1000

using namespace std;
using namespace npbnlp;

static unordered_map<int, int> tfreq;

spcfg::spcfg():_m(20), _k(20), _K(K), _v(C), _a(1), _b(1), _annl(50000), _nonterm(new hpyp(3)), _word(new vector<shared_ptr<hpyp> >), _letter(new vector<shared_ptr<vpyp> >) {
	_nonterm->set_v(_K);
	for (auto i = 0; i < _k+1; ++i) {
		_word->push_back(shared_ptr<hpyp>(new hpyp(1)));
		_letter->push_back(shared_ptr<vpyp>(new vpyp(_m)));
		(*_letter)[i]->set_v(_v);
		(*_word)[i]->set_base((*_letter)[i].get());
	}
}

spcfg::spcfg(int m):_m(m), _k(20), _K(K), _v(C), _a(1), _b(1), _annl(50000), _nonterm(new hpyp(3)), _word(new vector<shared_ptr<hpyp> >), _letter(new vector<shared_ptr<vpyp> >) {
	_nonterm->set_v(_K);
	for (auto i = 0; i < _k+1; ++i) {
		_word->push_back(shared_ptr<hpyp>(new hpyp(1)));
		_letter->push_back(shared_ptr<vpyp>(new vpyp(_m)));
		(*_letter)[i]->set_v(_v);
		(*_word)[i]->set_base((*_letter)[i].get());
	}
}

spcfg::~spcfg() {
}

void spcfg::anneal(double a) {
	if (a <= 0. || a > 1.)
		return;
	_annl = _annl/(_annl*a);
}

void spcfg::save(const char *f) {
	FILE *fp = NULL;
	if ((fp = fopen(f, "wb")) == NULL)
		throw "failed to open save file in spcfg::save";
	try {
		if (fwrite(&_m, sizeof(int), 1, fp) != 1)
			throw "failed to write _m in spcfg::save";
		if (fwrite(&_k, sizeof(int), 1, fp) != 1)
			throw "failed to write _k in spcfg::save";
		if (fwrite(&_K, sizeof(int), 1, fp) != 1)
			throw "failed to write _K in spcfg::save";
		if (fwrite(&_v, sizeof(int), 1, fp) != 1)
			throw "failed to write _v in spcfg::save";
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

void spcfg::load(const char *f) {
	FILE *fp = NULL;
	if ((fp = fopen(f, "rb")) == NULL)
		throw "failed to open save file in spcfg::load";
	try {
		if (fread(&_m, sizeof(int), 1, fp) != 1)
			throw "failed to read _m in spcfg::load";
		if (fread(&_k, sizeof(int), 1, fp) != 1)
			throw "failed to read _k in spcfg::load";
		if (fread(&_K, sizeof(int), 1, fp) != 1)
			throw "failed to read _K in spcfg::load";
		if (fread(&_v, sizeof(int), 1, fp) != 1)
			throw "failed to read _v in scpfg::load";
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

tree spcfg::sample(io& f, int i) {
	bool is_completed = false;
	cyk *c = NULL;
	int n = 0;
	do {
		if (c)
			delete c;
		c = new cyk(f, i);
		// sample preterminal
		_init(*c); 
		// sample non-terminal
		is_completed = _sample_tree(*c);
		++n;
	} while (!is_completed); 
	// tree sampling
	tree t(c->s);
	int id = t.s.size()-1; // root id
	int k = *c->k[0][id].begin(); // root
	node& root = t[id];	
	root.k = k;
	root.i = 0;
	root.j = id;
	_traceback(*c, 0, id, k, t);
	delete c;
	return t;
}

tree spcfg::parse(io& f, int i) {
	bool is_completed = false;
	cyk c(f,i);
	_init(c, true);
	is_completed = _sample_tree(c, true);
	tree t(c.s);
	if (!is_completed)
		return t;
	int id = t.s.size()-1;
	int k = *c.k[0][id].begin();
	node& root = t[id];
	root.k = k;
	root.i = 0;
	root.j = id;
	_traceback(c, 0, id, k, t);
	return t;
}

void spcfg::add(tree& t){
	lock_guard<mutex> m(_mutex);
	_add(t, t.s.size()-1);
	if (tfreq[_k] > 0)
		_resize();
}

void spcfg::_add(tree& t, int i) {
	node& z = t[i];
	tfreq[z.k]++;
	if (z.i != z.j) {
		int size = t.s.size();
		node& left = t[size*z.i+z.b-z.i*(1.+z.i)/2];
		node& right = t[size*(z.b+1)+z.j-(1.+z.b)*(z.b+2)/2];
		context *h = _nonterm->h();
		h = h->make(right.k);
		h = h->make(left.k);
		_nonterm->add(z.k, h);
		h = _nonterm->h();
		_nonterm->add(left.k, h);
		h = h->make(left.k);
		_nonterm->add(right.k, h);
		_add(t, size*z.i+z.b-z.i*(1.+z.i)/2);
		_add(t, size*(z.b+1)+z.j-(1.+z.b)*(z.b+2)/2);
	} else {
		word& w = t.wd(z.i);
		context *h = (*_word)[z.k]->h();
		(*_word)[z.k]->add(w, h);
		_nonterm->add(z.k, _nonterm->h());
	}
}

void spcfg::remove(tree& t) {
	lock_guard<mutex> m(_mutex);
	_remove(t, t.s.size()-1);
	for (int k = _k-1; tfreq[k] == 0; --k) {
		_shrink();
	}
}

void spcfg::_remove(tree& t, int i) {
	node& z = t[i];
	tfreq[z.k]--;
	if (z.i != z.j) {
		int size = t.s.size();
		node& left = t[size*z.i+z.b-z.i*(1.+z.i)/2];
		node& right = t[size*(z.b+1)+z.j-(1.+z.b)*(z.b+2)/2];
		context *h = _nonterm->h();
		h = h->find(right.k);
		h = h->find(left.k);
		_nonterm->remove(z.k, h);
		h = _nonterm->h();
		_nonterm->remove(left.k, h);
		h = h->find(left.k);
		_nonterm->remove(right.k, h);
		_remove(t, size*z.i+z.b-z.i*(1.+z.i)/2);
		_remove(t, size*(z.b+1)+z.j-(1.+z.b)*(z.b+2)/2);

	} else {
		word& w = t.wd(z.i);
		context *h = (*_word)[z.k]->h();
		(*_word)[z.k]->remove(w, h);
		_nonterm->remove(z.k, _nonterm->h());
	}
}

void spcfg::estimate(int iter) {
	for (int i = 1; i < _k+1; ++i) {
		(*_word)[i]->gibbs(iter);
		(*_word)[i]->estimate(iter);
		(*_letter)[i]->estimate(iter);
	}
	_nonterm->estimate(iter);
}

void spcfg::poisson_correction(int n) {
	for (int i = 1; i < _k+1; ++i) {
		(*_word)[i]->poisson_correction(n);
	}
}

void spcfg::set(int v, int k) {
	_v = v;
	_K = k;
	_k = min(_k, _K);
	for (auto it = _letter->begin(); it != _letter->end(); ++it) {
		(*it)->set_v(_v);
	}
	_nonterm->set_v(k);
}

void spcfg::slice(double a, double b) {
	return;
}

void spcfg::_traceback(cyk& c, int i, int j, int z, tree& tr) {
	if (i == j) { // pre-terminal
		return;
	} else {
		for (auto k = i; k < j; ++k) {
			if (!c.k[i][k].empty() && !c.k[k+1][j].empty()) {
				int l = *c.k[i][k].begin();
				int r = *c.k[k+1][j].begin();
				int b = k;
				node& n = tr[tr.s.size()*i+j-i*(1.+i)/2];
				n.b = b;
				node& ln = tr[tr.s.size()*i+b-i*(1.+i)/2];
				ln.i = i;
				ln.j = b;
				ln.k = l;
				node& rn = tr[tr.s.size()*(b+1)+j-(b+1.)*(2.+b)/2];
				rn.i = b+1;
				rn.j = j;
				rn.k = r;
				_traceback(c, i, b, ln.k, tr);
				_traceback(c, b+1, j, rn.k, tr);
				break;
			}
		}
	}
}

void spcfg::_init(cyk& l, bool best) {
	// terminal
	for (auto i = 0; i < l.s.size(); ++i) {
		_sample_preterm(l, i, best);
	}
}

void spcfg::_sample_preterm(cyk& l, int i, bool best) {
	word& w = l.wd(i);
	vector<double> table;
	for (auto k = 1; k < _k+1; ++k) {
		double lp = (*_word)[k]->lp(w, (*_word)[k]->h())+_nonterm->lp(k, _nonterm->h());
		table.push_back(lp);
	}
	if (best) {
		int k = 1+rd::best(table);
		l.k[i][i].insert(k);
	}
	else {
		int k = 1+rd::ln_draw(table);
		l.k[i][i].insert(k);
	}
}

bool spcfg::_sample_tree(cyk& l, bool best) {
	stack<spcfg::cell> s;
	deque<spcfg::cell> q;
	int size = l.s.size();
	for (auto i = 0; i < size; ++i) {
		q.push_back(make_pair(i,i));
	}
	while (!q.empty()) {
		spcfg::cell rc = q.front();
		if (s.empty()) {
			s.push(rc);
			q.pop_front();
		} else {
			spcfg::cell& lc = s.top();
			int z = _sample_nonterm(l, lc, rc, best);
			if (z < 0) {
				s.push(rc);
				q.pop_front();
			} else {
				l.k[lc.first][rc.second].insert(z);
				q.pop_front();
				q.push_front(make_pair(lc.first,rc.second));
				s.pop();
			}
		}
	}
	while (s.size() > 1) {
		spcfg::cell rc = s.top(); s.pop();
		spcfg::cell lc = s.top(); s.pop();
		int z = _sample_nonterm(l, lc, rc, best);
		if (z < 0)
			return false;
		l.k[lc.first][rc.second].insert(z);
		s.push(make_pair(lc.first, rc.second));
	}
	return true;
}

int spcfg::_sample_nonterm(cyk& l, spcfg::cell& left, spcfg::cell& right, bool best) {
	int lc = *l.k[left.first][left.second].begin();
	int rc = *l.k[right.first][right.second].begin();
	context *h = _nonterm->h();
	context *s = _nonterm->h();
	double lp_l = _nonterm->lp(lc, h);
	context *t = h->find(lc);
	if (t)
		h = t;
	double lp_r = _nonterm->lp(rc, h);
	context *u = s->find(rc);
	if (u) {
		s = u;
		u = s->find(lc);
		if (u)
			s = u;
	}
	double z = 0;
	vector<double> table;
	vector<int> id;
	for (auto k = max(lc, rc); k > 0; --k) {
		double lp = _nonterm->lp(k,s)+lp_l+lp_r;
		table.push_back(lp);
		id.push_back(k);
		z = math::lse(z, lp, (z==0));
	}
	double p = pow(1.-exp(z), _annl);
	shared_ptr<generator> g = generator::create();
	bernoulli_distribution d;
	bernoulli_distribution::param_type mu(p);
	if (d((*g)(), mu))
		return -1;
	if (best) {
		return id[rd::best(table)];
	} else {
		return id[rd::ln_draw(table)];
	}
}

void spcfg::_resize() {
	if (_k+1 > _K)
		return;
	++_k;
	_word->resize(_k+1, shared_ptr<hpyp>(new hpyp(1)));
	_letter->resize(_k+1, shared_ptr<vpyp>(new vpyp(_m)));
	(*_word)[_k]->set_base((*_letter)[_k].get());
	(*_letter)[_k]->set_v(_v);
}

void spcfg::_shrink() {
	--_k;
	_word->pop_back();
	_letter->pop_back();
}
