#include"convinience.h"
#include"hdp.h"
#include"hdp_context.h"
#include<random>

#define VOCAB 100000
#define ALPHA 0.001

using namespace std;
using namespace npbnlp;
using gamma_dist = gamma_distribution<>;

hdp::hdp(): _n(1), _a(1), _b(1), _base(NULL), _v(VOCAB), _h(new hdp_context), _alpha(new vector<double>(_n, ALPHA)), _bc(nullptr), _cbc(nullptr) {
}

hdp::hdp(int n, double a, double b): _n(n), _a(a), _b(b), _base(NULL), _v(VOCAB), _h(new hdp_context), _alpha(new vector<double>(_n, ALPHA)),_bc(nullptr), _cbc(nullptr) {
}

hdp::~hdp() {
}

double hdp::alpha(int n) const {
	return (*_alpha)[n];
}

double hdp::discount(int n) const {
	return 0;
}

double hdp::strength(int n) const {
	return 0;
}

void hdp::set_base(lm *b) {
	_base = b;
}

void hdp::set_v(int v) {
	if (v < 1)
		throw "found invalid vocab setting";
	_v = v;
}

int hdp::n() const {
	return _n;
}

int hdp::v() const {
	return _v;
}

context* hdp::h() const {
	return _h.get();
	//return &(*_h);
}

context* hdp::find(nsentence& s, int i) const {
	context *c = h();
	for (int m = 1; m < _n; ++m) {
		context *d = c->find(s[i-m]);
		if (d)
			c = d;
		else 
			break;
	}
	return c;
}

context* hdp::find(sentence& s, int i) const {
	context *c = h();
	for (int m = 1; m < _n; ++m) {
		context *d = c->find(s[i-m]);
		if (d)
			c = d;
		else 
			break;
	}
	return c;
}

context* hdp::find(word& w, int i) const {
	context *c = h();
	for (int m = 1; m < _n; ++m) {
		context *d = c->find(w[i-m]);
		if (d)
			c = d;
		else
			break;
	}
	return c;
}

context* hdp::find(chunk& b, int i) const {
	context *c = h();
	for (int m = 1; m < _n; ++m) {
		context *d = c->find(b[i-m]);
		if (d)
			c = d;
		else
			break;
	}
	return c;
}

context* hdp::make(nsentence& s, int i) {
	context *c = h();
	for (int m = 1; m < _n; ++m) {
		c = c->make(s[i-m]);
	}
	return c;
}

context* hdp::make(sentence& s, int i) {
	context *c = h();
	for (int m = 1; m < _n; ++m) {
		c = c->make(s[i-m]);
	}
	return c;
}

context* hdp::make(word& w, int i) {
	context *c = h();
	for (int m = 1; m < _n; ++m) {
		c = c->make(w[i-m]);
	}
	return c;
}

context* hdp::make(chunk& b, int i) {
	context *c = h();
	for (int m = 1; m < _n; ++m) {
		c = c->make(b[i-m]);
	}
	return c;
}

void hdp::estimate(int iter) {
	gamma_dist gm;
	shared_ptr<generator> g = generator::create();
	for (int i = 0; i < iter; ++i) {
		vector<double> aux_a(_n);
		vector<double> aux_b(_n);
		_h->estimate_a(aux_a, aux_b, this);
		for (int i = 0; i < _n; ++i) {
			gamma_dist::param_type param(_a+aux_a[i], 1./(_b-aux_b[i]));
			gm.param(param);
			(*_alpha)[i] = gm((*g)());
		}
	}
	_cache.clear();
}

double hdp::pr(chunk& c, const context *h) {
	return exp(lp(c,h));
}

double hdp::pr(word& w, const context *h) {
	return exp(lp(w,h));
	/*
	   if (!h) {
	   return _prb(w);
	   }
	   double c = h->c();
	   double cu = h->cu(w.id);
	   int n = h->n();
	   return cu/(c+(*_alpha)[n])+(*_alpha)[n]/(c+(*_alpha)[n])*pr(w, h->parent());
	   */
}

double hdp::pr(int k, const context *h) {
	return exp(lp(k,h));
	/*
	   if (!h) {
	   return 1./_v;
	   }
	//
	//if (!h && _h->cu(k) == 0 && _h->v() < _v)
	//	return 1./_v*(_v-_h->v());
	//else if (!h)
	//	return 1./_v;
	//	
	double c = h->c();
	double cu = h->cu(k);
	int n = h->n();
	return cu/(c+(*_alpha)[n])+(*_alpha)[n]/(c+(*_alpha)[n])*pr(k, h->parent());
	*/
}

double hdp::lp(chunk& b, const context *h) {
	if (!h)
		return _lpb(b);
	bool chk = false;
	double lpr = _cache.get(b, h, chk);
	if (chk)
		return lpr;
	double c = h->c();
	double cu = h->cu(b.id);
	int n = h->n();
	if (cu == 0.)
		lpr = log((*_alpha)[n])+lp(b, h->parent())-log(c+(*_alpha)[n]);
	else
		lpr = math::lse(log(cu)-log(c+(*_alpha)[n]), log((*_alpha)[n])+lp(b, h->parent())-log(c+(*_alpha)[n]));
	return _cache.set(b, h, lpr);
}

double hdp::lp(word& w, const context *h) {
	if (!h) {
		return _lpb(w);
	}
	bool chk = false;
	double lpr = _cache.get(w, h, chk);
	if (chk)
		return lpr;
	double c = h->c();
	double cu = h->cu(w.id);
	int n = h->n();
	// cu == 0
	// log(cu/(c+a)+a*pr/(c+a))
	// = log(a*pr/(c+a))
	// = log(a*pr)-log(c+a)
	// = log(a)+log(pr)-log(c+a)
	// otherwise
	// log(cu/(c+a)+a*pr/(c+a))
	// = logsumexp(log(cu/(c+a)) + log(a*pr/(c+a)))
	// = logsumexp(log cu - log(c+a) + log(a*pr)- log(c+a))
	// = logsumexp(log cu - log(c+a) + log(a)+log(pr)-log(c+a))
	if (cu == 0.)
		lpr = log((*_alpha)[n])+lp(w, h->parent())-log(c+(*_alpha)[n]);
	else
		lpr = math::lse(log(cu)-log(c+(*_alpha)[n]), log((*_alpha)[n])+lp(w, h->parent())-log(c+(*_alpha)[n]));
	return _cache.set(w, h, lpr);
}

double hdp::lp(int k, const context *h) {
	if (!h) {
		return -log(_v);
	}
	/*
	   if (!h && _h->cu(k) == 0 && _h->v() < _v)
	   return log(_v-_h->v()) -log(_v);
	   else if (!h)
	   return -log(_v);
	   */
	bool chk = false;
	double lpr = _cache.get(k, h, chk);
	if (chk)
		return lpr;
	double c = h->c();
	double cu = h->cu(k);
	int n = h->n();
	if (cu == 0.)
		lpr = log((*_alpha)[n])+lp(k, h->parent())-log(c+(*_alpha)[n]);
	else
		lpr = math::lse(log(cu)-log(c+(*_alpha)[n]), log((*_alpha)[n])+lp(k, h->parent())-log(c+(*_alpha)[n]));
	return _cache.set(k, h, lpr);
}

/*
   double hdp::_find_cache(word& w, const context *c) {
   if (w.id == 1) // unk
   return 1;
   return _find_cache(w.id, c);
   }

   double hdp::_find_cache(int k, const context *c) {
   auto i = _cache.find(c);
   if (i == _cache.end()) {
   return 1;
   } else {
   auto j = i->second.find(k);
   if (j == i->second.end())
   return 1;
   else
   return j->second;
   }
   }

   double hdp::_set_cache(word& w, const context *c, double lpr) {
   if (w.id == 1) // unk
   return lpr;
   return _set_cache(w.id, c, lpr);
   }

   double hdp::_set_cache(int k, const context *c, double lpr) {
   lock_guard<mutex> m(_mutex);
   _cache[c][k] = lpr;
   return _cache[c][k];
   }
   */

double hdp::_prb(word& w) const {
	return exp(_lpb(w));
}

double hdp::_prb(chunk& c) const {
	return exp(_lpb(c));
}

double hdp::_lpb(word& w) const {
	if (!_base)
		return -log(_v);
	/*
	   if (!_base && _h->cu(w.id) == 0 && _h->v() < _v) { 
	   return log(_v-_h->v())-log(_v);
	   } else if (!_base) {
	   return -log(_v);
	   }
	   */
	double lp = 0;
	for (int i = 0; i < w.len+1; ++i) {
		int n = _base->n();
		context *h = _base->h();
		for (int j = 1; j < n; ++j) {
			context *c = h->find(w[i-j]);
			if (!c)
				break;
			else
				h = c;
		}
		lp += _base->lp(w[i], h);
	}
	//return max(-log(_v), lp);
	return lp;
}

double hdp::_lpb(chunk& b) const {
	if (!_base)
		return -log(_v);
	double lp = 0;
	for (int i = 0; i < b.len+1; ++i) {
		int n = _base->n();
		context *h = _base->h();
		for (int j = 1; j < n; ++j) {
			context *c = h->find(b[i-j]);
			if (!c)
				break;
			else
				h = c;
		}
		lp += _base->lp(b[i], h);
	}
	return lp;
}

bool hdp::add(int k, context *h) { 
	bool add_to_parent = false;
	while (h && (add_to_parent = h->add(k, this)))
		h = h->parent();
	_cache.clear();
	return add_to_parent;
}

bool hdp::remove(int k, context *h) {
	bool remove_from_parent = false;
	while (h && (remove_from_parent = h->remove(k)))
		h = h->parent();
	_cache.clear();
	return remove_from_parent;
}

void hdp::add(word& w, context *h) {
	if (add(w.id, h) && _base) {
		lock_guard<mutex> m(_mutex);
		if (_bc == nullptr)
			_bc = shared_ptr<base_corpus>(new base_corpus);
		wrap::add_a(w, _base);
		(*_bc)[w.id].push_back(w);
	}
	_cache.clear();
}

void hdp::remove(word& w, context *h) {
	if (remove(w.id, h) && _base) {
		lock_guard<mutex> m(_mutex);
		int size = (*_bc)[w.id].size();
		int id = (*generator::create())()()%size;
		word& b = (*_bc)[w.id][id];
		wrap::remove_a(b, _base);
		(*_bc)[w.id].erase((*_bc)[w.id].begin()+id);
	}
	_cache.clear();
}

void hdp::add(chunk& b, context *h) {
	if (add(b.id, h) && _base) {
		lock_guard<mutex> m(_mutex);
		if (_cbc == nullptr)
			_cbc = shared_ptr<cbase_corpus>(new cbase_corpus);
		wrap::add_a(b, _base);
		(*_cbc)[b.id].push_back(b);
	}
	_cache.clear();
}

void hdp::remove(chunk& c, context *h) {
	if (remove(c.id, h) && _base) {
		lock_guard<mutex> m(_mutex);
		int size = (*_cbc)[c.id].size();
		int id = (*generator::create())()()%size;
		chunk& b = (*_cbc)[c.id][id];
		wrap::remove_a(b, _base);
		(*_cbc)[c.id].erase((*_cbc)[c.id].begin()+id);
	}
	_cache.clear();
}

int hdp::draw_n(nsentence& s, int i) {
	return _n;
}

int hdp::draw_n(sentence& s, int i) {
	return _n;
}

int hdp::draw_n(word& w, int i) {
	return _n;
}

int hdp::draw_n(chunk& c, int i) {
	return _n;
}

int hdp::draw_k(context *h) {
	return h->sample(this);
}

void hdp::save(const char *file) {
	FILE *fp = NULL;
	if ((fp = fopen(file, "wb")) == NULL)
		return;
	if (fwrite(&_n, sizeof(int), 1, fp) != 1)
		throw "failed to write _n in hdp::save";
	if (fwrite(&_a, sizeof(double), 1, fp) != 1)
		throw "failed to write _a in hdp::save";
	if (fwrite(&_b, sizeof(double), 1, fp) != 1)
		throw "failed to write _b in hdp::save";
	if (fwrite(&_v, sizeof(int), 1, fp) != 1)
		throw "failed to write _v in hdp::save";
	if (fwrite(&(*_alpha)[0], sizeof(double), _alpha->size(), fp) != _alpha->size())
		throw "failed to write _alpha in hdp::save";
	_h->save(fp);
	fclose(fp);
}

void hdp::load(const char *file) {
	FILE *fp = NULL;
	if ((fp = fopen(file, "rb")) == NULL)
		return;
	if (fread(&_n, sizeof(int), 1, fp) != 1)
		throw "failed to read _n in hdp::load";
	if (fread(&_a, sizeof(double), 1, fp) != 1)
		throw "failed to read _a in hdp::load";
	if (fread(&_b, sizeof(double), 1, fp) != 1)
		throw "failed to read _b in hdp::load";
	if (fread(&_v, sizeof(int), 1, fp) != 1)
		throw "failed to read _v in hdp::load";
	_alpha->resize(_n);
	if (fread(&(*_alpha)[0], sizeof(double), _n, fp) != (size_t)_n)
		throw "failed to read _alpha in hdp::load";
	_h->load(fp);
	fclose(fp);
}
