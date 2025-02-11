#include"kn.h"
#include"math.h"

using namespace npbnlp;
using namespace std;

// 3: means controll code ^c that is used as a terminal code for double array trie
kn::kn():_n(1), _d(0.75), _nc(new count(3)), _nz(new count(3)), _nk(new count(3)) {
}

kn::kn(int n):_n(n), _d(0.75), _nc(new count(3)), _nz(new count(3)), _nk(new count(3)) {
}

kn::kn(const kn& lm) {
	_n = lm._n;
	_d = lm._d;
	_nc = lm._nc;
	_nz = lm._nz;
	_nk = lm._nk;
}

kn& kn::operator=(const kn& lm) {
	_n = lm._n;
	_d = lm._d;
	_nc = lm._nc;
	_nz = lm._nz;
	_nk = lm._nk;
	return *this;
}

kn::~kn() {
}

void kn::set_discount(double d) {
	if (d > 0)
		_d = d;
}

bool kn::_add(word& w, int i, int n) {
	if (n == 0)
		return true;
	_nc->incr(w, i, n);
	_nz->incr(w, i-1, n-1);
	auto id = _nc->rexactmatch(w, i, n);
	auto v = _nc->getval(id);
	if (v && v.value() == 1) {
		_nk->incr(w, i-1, n-1);
		return _add(w, i, n-1);
	}
	return true;
}

bool kn::add(word& w) {
	for (auto i = 0; i <= w.len; ++i) {
		_add(w, i, _n);
	}
	return true;
}

bool kn::_remove(word& w, int i, int n) {
	if (n == 0)
		return true;
	_nc->decr(w, i, n);
	_nz->decr(w, i-1, n-1);
	auto id = _nc->rexactmatch(w, i, n);
	auto v = _nc->getval(id);
	if (!v) {
		_nk->decr(w, i-1, n-1);
		return _remove(w, i, n-1);
	}
	return true;
}

bool kn::remove(word& w) {
	for (auto i = 0; i <= w.len; ++i) {
		_remove(w, i, _n);
	}
	return true;
}

bool kn::_add(sentence& s, int i, int n) {
	if (n == 0)
		return true;
	_nc->incr(s, i, n);
	_nz->incr(s, i-1, n-1);
	auto id = _nc->rexactmatch(s, i, n);
	auto v = _nc->getval(id);
	if (v && v.value() == 1) {
		_nk->incr(s, i-1, n-1);
		return _add(s, i, n-1);
	}
	return true;
}

bool kn::add(sentence& s) {
	for (auto i = 0; i <= s.size(); ++i) {
		_add(s, i, _n);
	}
	return true;
}

bool kn::_remove(sentence& s, int i, int n) {
	if (n == 0)
		return true;
	_nc->decr(s, i, n);
	_nz->decr(s, i-1, n-1);
	auto id = _nc->rexactmatch(s, i, n);
	auto v = _nc->getval(id);
	if (!v) {
		_nk->decr(s, i-1, n-1);
		return _remove(s, i, n-1);
	}
	return true;
}

bool kn::remove(sentence& s) {
	for (auto i = 0; i <= s.size(); ++i) {
		_remove(s, i, _n);
	}
	return true;
}

double kn::lp(word& w, int i) {
	auto nc = _nc->cs_search(w, i, _n);
	auto nz = _nz->cs_search(w, i-1, _n-1);
	auto nk = _nk->cs_search(w, i-1, _n-1);
	double lp = -log(_nk->getval(nk[0].second).value());
	for (auto j = 1; j < nz.size()+1; ++j) {
		auto z = _nz->getval(nz[j-1].second);
		auto k = _nk->getval(nk[j-1].second);
		lp += log(_d)+log(k.value())-log(z.value());
		if (nc.size() > j) {
			auto c = _nc->getval(nc[j].second);
			lp = math::lse(lp, (log((double)c.value()-_d))-log(z.value()));
		}
	}
	/*
	for (auto j = 1; j < nc.size(); ++j) {
		auto c = _nc->getval(nc[j].second);
		auto z = _nz->getval(nz[j-1].second);
		auto k = _nk->getval(nk[j-1].second);
		lp += log(_d)+log(k.value())-log(z.value());
		lp = math::lse(lp, (log((double)c.value()-_d))-log(z.value()));

	}
	*/
	return lp;
}

double kn::lp(sentence& s, int i) {
	auto nc = _nc->cs_search(s, i, _n);
	auto nz = _nz->cs_search(s, i-1, _n-1);
	auto nk = _nk->cs_search(s, i-1, _n-1);
	double lp = -log(_nk->getval(nk[0].second).value());
	for (auto j = 1; j < nz.size()+1; ++j) {
		auto z = _nz->getval(nz[j-1].second);
		auto k = _nk->getval(nk[j-1].second);
		lp += log(_d)+log(k.value())-log(z.value());
		if (nc.size() > j) {
			auto c = _nc->getval(nc[j].second);
			lp = math::lse(lp, (log((double)c.value()-_d))-log(z.value()));
		}
	}
	/*
	for (auto j = 1; j < nc.size(); ++j) {
		auto c = _nc->getval(nc[j].second);
		auto z = _nz->getval(nz[j-1].second);
		auto k = _nk->getval(nk[j-1].second);
		lp += log(_d)+log(k.value())-log(z.value());
		lp = math::lse(lp, (log((double)c.value()-_d))-log(z.value()));

	}
	*/
	return lp;
}

double kn::lp(word& w) {
	double l = 0;
	for (auto i = 0; i < w.len+1; ++i) {
		l += lp(w, i);
	}
	return l;
}

double kn::lp(sentence& s) {
	double l = 0;
	for (auto i = 0; i < s.size()+1; ++i) {
		l += lp(s, i);
	}
	return l;
}

int kn::save(const char *file) {
	FILE *fp = NULL;
	if ((fp = fopen(file, "wb")) == NULL)
		return 1;
	if (fwrite(&_n, sizeof(int), 1, fp) != 1)
		return 1;
	if (fwrite(&_d, sizeof(double), 1, fp) != 1)
		return 1;
	fclose(fp);
	string nc(file);
	nc += ".nc";
	string nz(file);
	nz += ".nz";
	string nk(file);
	nk += ".nk";
	_nc->save(nc.c_str(), count::int_writer, count::uint_writer);
	_nz->save(nz.c_str(), count::int_writer, count::uint_writer);
	_nk->save(nk.c_str(), count::int_writer, count::uint_writer);
	return 0;
}

int kn::load(const char *file) {
	FILE *fp = NULL;
	if ((fp = fopen(file, "rb")) == NULL)
		return 1;
	if (fread(&_n, sizeof(int), 1, fp) != 1)
		return 1;
	if (fread(&_d, sizeof(double), 1, fp) != 1)
		return 1;
	fclose(fp);
	string nc(file);
	nc += ".nc";
	string nz(file);
	nz += ".nz";
	string nk(file);
	nk += ".nk";
	_nc->load(nc.c_str(), count::int_reader, count::uint_reader);
	_nz->load(nz.c_str(), count::int_reader, count::uint_reader);
	_nk->load(nk.c_str(), count::int_reader, count::uint_reader);
	return 0;
}
