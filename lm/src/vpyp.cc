#include"vpyp.h"
#include"hpyp.h"
#include"math.h"
#include"convinience.h"
#include<random>


using namespace std;
using namespace npbnlp;

vpyp::vpyp():hpyp() {
}

vpyp::vpyp(int n, double a, double b):hpyp(n,a,b) {
}

vpyp::~vpyp() {
}

double vpyp::pr(int k, const context *h) {
	return exp(lp(k,h));
	/*
	if (!h)
		return 1./_v;
	*/
	//
	//if (!h && _h->cu(k) == 0 && _h->v() < _v)
	//	return 1./_v*(_v-_h->v());
	//else if (!h)
	//	return 1./_v;
	//	
	//return exp(max(-log(_v),lp(k, h)));
}

double vpyp::pr(word& w, const context *h) {
	return exp(lp(w,h));
	/*
	if (!h)
		return _prb(w);
	return exp(lp(w, h));
	*/
}

double vpyp::pr(chunk& c, const context *h) {
	return exp(lp(c, h));
}

double vpyp::lp(int k, const context *h) {
	if (!h)
		return -log(_v);
	/*
	if (!h && _h->cu(k) == 0 && _h->v() < _v)
		return log(_v-_h->v())-log(_v);
	else if (!h)
		return -log(_v);
		*/
	bool chk = false;
	double ln_pr = _cache.get(k, h, chk);
	if (chk)
		return ln_pr;
	ln_pr = 0;
	double z = 0;
	const context *c = h;
	while (c) {
		int s = c->stop();
		int p = c->pass();
		double ln_pr_stop = log(s) - log(s+p);
		double ln_pr_pass = log(p) - log(s+p);
		z = math::lse(z+ln_pr_pass, ln_pr_stop, (z==0));
		ln_pr = math::lse(ln_pr+ln_pr_pass,ln_pr_stop+hpyp::lp(k,c),(ln_pr==0));
		c = c->parent();
	}
	return _cache.set(k, h, ln_pr-z);
}

double vpyp::lp(word& w, const context *h) {
	if (!h)
		return _lpb(w);
	bool chk = false;
	double ln_pr = _cache.get(w, h, chk);
	if (chk)
		return ln_pr;
	ln_pr = 0;
	double z = 0;
	const context *c = h;
	while (c) {
		int s = c->stop();
		int p = c->pass();
		double ln_pr_stop = log(s) - log(s+p);
		double ln_pr_pass = log(p) - log(s+p);
		z = math::lse(z+ln_pr_pass, ln_pr_stop, (z==0));
		ln_pr = math::lse(ln_pr+ln_pr_pass, ln_pr_stop+hpyp::lp(w,c),(ln_pr == 0));
		c = c->parent();
	}
	return _cache.set(w, h, ln_pr-z);
}

double vpyp::lp(chunk& b, const context *h) {
	if (!h)
		return _lpb(b);
	bool chk = false;
	double ln_pr = _cache.get(b, h, chk);
	if (chk)
		return ln_pr;
	ln_pr = 0;
	double z = 0;
	const context *c = h;
	while (c) {
		int s = c->stop();
		int p = c->pass();
		double ln_pr_stop = log(s) - log(s+p);
		double ln_pr_pass = log(p) - log(s+p);
		z = math::lse(z+ln_pr_pass, ln_pr_stop, (z==0));
		ln_pr = math::lse(ln_pr+ln_pr_pass, ln_pr_stop+hpyp::lp(b,c),(ln_pr == 0));
		c = c->parent();
	}
	return _cache.set(b, h, ln_pr-z);
}

double vpyp::_lpb(word& w) const {
	if (!_base)
		return -log(_v);
	/*
	if (!_base && _h->cu(w.id) == 0 && _h->v() < _v)
		return log(_v-_h->v())-log(_v);
	else if (!_base)
		return -log(_v);
		*/
	double lp = 0;
	for (int i = 0; i < w.len+1; ++i) {
		int n = w.m[i];
		context *h = _base->h();
		for (int j = 1; j < n && i-j >= -1; ++j) {
			context *c = h->find(w[i-j]);
			if (!c)
				break;
			else
				h = c;
		}
		lp += _base->lp(w[i], h);
	}
	//return max(-log(_v),lp);
	return lp;
}

double vpyp::_lpb(chunk& b) const {
	if (!_base)
		return -log(_v);
	double lp = 0;
	for (int i = 0;i < b.len+1; ++i) {
		int n = b.n[i];
		context *h = _base->h();
		for (int j = 1; j < n && i-j >= -1; ++j) {
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

double vpyp::_prb(chunk& c) const {
	return exp(_lpb(c));
}

double vpyp::_prb(word& w) const {
	return exp(_lpb(w));
}
