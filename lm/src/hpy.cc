#include"hpy.h"
#include"math.h"
#include"beta.h"
#include"rd.h"
#include<random>

#define DISCOUNT 0.5
#define STRENGTH 1

using namespace std;
using namespace npbnlp;
using gamma_dist = gamma_distribution<>;

arrangement::arrangement():n(0),customer(0),table(new vector<int>) {
}

arrangement::arrangement(int n):n(n),customer(0),table(new vector<int>) {
}

arrangement::arrangement(const arrangement& a) {
	n = a.n;
	customer = a.customer;
	table = a.table;
}

arrangement& arrangement::operator=(const arrangement& a) {
	n = a.n;
	customer = a.customer;
	table = a.table;
	return *this;
}

arrangement::~arrangement() {
}

restaurant::restaurant():n(0),table(0),customer(0),arrangements(new unordered_map<unsigned int, arrangement>) {
}

restaurant::restaurant(int n):n(n),table(0),customer(0),arrangements(new unordered_map<unsigned int, arrangement>) {
}

restaurant::restaurant(const restaurant& r) {
	n = r.n;
	table = r.table;
	customer = r.customer;
	arrangements = r.arrangements;
}

restaurant& restaurant::operator=(const restaurant& r) {
	n = r.n;
	table = r.table;
	customer = r.customer;
	arrangements = r.arrangements;
	return *this;
}

restaurant::~restaurant() {
}

hpy::hpy():_n(1),_v(1),_base(NULL),_discount(new vector<double>(_n, DISCOUNT)), _strength(new vector<double>(_n, STRENGTH)), /*_nc(new sda<arrangement>(3)),*/ _nz(new sda<restaurant>(3)) {
}

hpy::hpy(int n):_n(n),_v(1),_base(NULL),_discount(new vector<double>(_n, DISCOUNT)), _strength(new vector<double>(_n, STRENGTH)), /*_nc(new sda<arrangement>(3)), */_nz(new sda<restaurant>(3)) {
}

hpy::hpy(const hpy& lm) {
	_n = lm._n;
	_v = lm._v;
	_base = lm._base;
	_discount = lm._discount;
	_strength = lm._discount;
	//_nc = lm._nc;
	_nz = lm._nz;
}

hpy& hpy::operator=(const hpy& lm) {
	_n = lm._n;
	_v = lm._v;
	_base = lm._base;
	_discount = lm._discount;
	_strength = lm._discount;
	//_nc = lm._nc;
	_nz = lm._nz;
	return *this;
}

hpy::~hpy() {
}

void hpy::set_base(hpy *b) {
	if (b)
		_base = b;
}

bool hpy::add(word& w) {
	for (auto i = 0; i <= w.len; ++i) {
		_add(w, i, _n);
	}
	return true;
}

bool hpy::add(sentence& s) {
	for (auto i = 0; i <= s.size(); ++i) {
		_add(s, i, _n);
	}
	return true;
}

bool hpy::remove(word& w) {
	for (auto i = 0; i <= w.len; ++i) {
		_remove(w, i, _n);
	}
	return true;
}

bool hpy::remove(sentence& s) {
	for (auto i = 0; i <= s.size(); ++i) {
		_remove(s, i, _n);
	}
	return true;
}

bool hpy::_add(sentence& s, int i, int n) {
	if (n == 0)
		return true;
	double lpr = _lp(s, i, n-1);
	/*
	auto c = _nc->rexactmatch(s, i, n);
	if (c < 0) {
		_nc->insert(s, i, n);
		c = _nc->rexactmatch(s, i, n);
	}
	*/
	auto z = _nz->rexactmatch(s, i-1, n-1);
	if (z < 0) {
		_nz->insert(s, i-1, n-1);
		z = _nz->rexactmatch(s, i-1, n-1);
	}

	auto& rst = _nz->val(z); rst.n = n-1;
	auto it = rst.arrangements->find(s[i]);
	if (it == rst.arrangements->end()) {
		(*rst.arrangements)[s[i]] = arrangement(n);
	}
	//auto& arr = _nc->val(c); arr.n = n;
	auto& arr = (*rst.arrangements)[s[i]];
	++rst.customer;
	++arr.customer;
	int size = arr.table->size();
	vector<double> t(size+1,0);
	double r = 0;
	for (auto j = 0; j < size; ++j) {
		t[j] = (*arr.table)[j] - (*_discount)[rst.n];
		r += t[j];
	}
	t[size] = ((*_strength)[rst.n] + rst.table * (*_discount)[rst.n]) * exp(lpr);
	r += t[size];
	int id = rd::draw(r, t);
	if (id == size) {
		arr.table->emplace_back(1);
		++rst.table;
		if (n == 1 && id == 0)
			++_v;
		return _add(s, i, n-1);
	} else {
		(*arr.table)[id]++;
	}
	return true;
}

bool hpy::_add(word& w, int i, int n) {
	if (n == 0)
		return true;
	double lpr = _lp(w, i, n-1);
	/*
	   auto c = _nc->rexactmatch(w, i, n);
	   if (c < 0) {
	   _nc->insert(w, i, n);
	   c = _nc->rexactmatch(w, i, n);
	   }
	   */
	auto z = _nz->rexactmatch(w, i-1, n-1);
	if (z < 0) {
		_nz->insert(w, i-1, n-1);
		z = _nz->rexactmatch(w, i-1, n-1);
	}

	auto& rst = _nz->val(z); rst.n = n-1;
	auto it = rst.arrangements->find(w[i]);
	if (it == rst.arrangements->end()) {
		(*rst.arrangements)[w[i]] = arrangement(n);
	}
	//auto& arr = _nc->val(c); arr.n = n;
	auto& arr = (*rst.arrangements)[w[i]];
	++rst.customer;
	++arr.customer;
	int size = arr.table->size();
	vector<double> t(size+1,0);
	double r = 0;
	for (auto j = 0; j < size; ++j) {
		t[j] = (*arr.table)[j] - (*_discount)[rst.n];
		r += t[j];
	}
	t[size] = ((*_strength)[rst.n] + rst.table * (*_discount)[rst.n]) * exp(lpr);
	r += t[size];
	int id = rd::draw(r, t);
	if (id == size) {
		arr.table->emplace_back(1);
		++rst.table;
		if (n == 1 && id == 0)
			++_v;
		return _add(w, i, n-1);
	} else {
		(*arr.table)[id]++;
	}
	return true;
}

bool hpy::_remove(sentence& s, int i, int n) {
	if (n == 0)
		return true;
	//auto c = _nc->rexactmatch(s, i, n);
	auto z = _nz->rexactmatch(s, i-1, n-1);
	auto& rst = _nz->val(z);
	//auto& arr = _nc->val(c);
	auto& arr = (*rst.arrangements)[s[i]];
	--rst.customer;
	--arr.customer;
	double r = 0;
	int size = arr.table->size();
	vector<double> t(size, 0);
	for (auto j = 0; j < size; ++j) {
		t[j] = (*arr.table)[j];
		r += t[j];
		//r += (*arr.table)[j];
	}
	int id = rd::draw(r, t);
	(*arr.table)[id]--;
	if ((*arr.table)[id] == 0) {
		--rst.table;
		arr.table->erase(arr.table->begin()+id);
		if (arr.customer == 0) {
			rst.arrangements->erase(s[i]);
			//_nc->erase(s, i, n);
			if (n == 1)
				--_v;
		}
		if (rst.customer == 0) {
			_nz->erase(s, i-1, n-1);
		}
		return _remove(s, i, n-1);
	}
	return true;
}

bool hpy::_remove(word& w, int i, int n) {
	if (n == 0)
		return true;
	//auto c = _nc->rexactmatch(w, i, n);
	auto z = _nz->rexactmatch(w, i-1, n-1);
	auto& rst = _nz->val(z);
	//auto& arr = _nc->val(c);
	auto& arr = (*rst.arrangements)[w[i]];
	--rst.customer;
	--arr.customer;
	double r = 0;
	int size = arr.table->size();
	vector<double> t(size, 0);
	for (auto j = 0; j < size; ++j) {
		t[j] = (*arr.table)[j];
		r += t[j];
		//r += (*arr.table)[j];
	}
	int id = rd::draw(r, t);
	(*arr.table)[id]--;
	if ((*arr.table)[id] == 0) {
		--rst.table;
		arr.table->erase(arr.table->begin()+id);
		if (arr.customer == 0) {
			rst.arrangements->erase(w[i]);
			//_nc->erase(w, i, n);
			if (n == 1)
				--_v;
		}
		if (rst.customer == 0)
			_nz->erase(w, i-1, n-1);
		return _remove(w, i, n-1);
	}
	return true;
}

void hpy::estimate(int iter) {
	beta_distribution be;
	gamma_dist gm;
	shared_ptr<generator> g = generator::create();
	for (auto i = 0; i < iter; ++i) {
		vector<double> discount_a(_n, 0);
		vector<double> discount_b(_n, 0);
		vector<double> strength_a(_n, 0);
		vector<double> strength_b(_n, 0);
		_estimate_d(discount_a, discount_b);
		_estimate_t(strength_a, strength_b);
		for (auto j = 0; j < _n; ++j) {
			gamma_dist::param_type param(1.+strength_a[j], 1./(1.-strength_b[j]));
			gm.param(param);
			(*_discount)[j] = be(1.+discount_a[j],1.+discount_b[j]);
			(*_strength)[j] = gm((*g)());
		}
	}
}

void hpy::_estimate_d(vector<double>& a, vector<double>& b) {
	shared_ptr<generator> g = generator::create();
	bernoulli_distribution d;
	for (auto it = _nz->begin(); it != _nz->end(); ++it) {
		if (it->n < 0) // erased node
			continue;
		for (auto i = 1; i < it->table; ++i) {
			bernoulli_distribution::param_type mu((*_strength)[it->n]/((*_strength)[it->n]+(*_discount)[it->n]*i));
			a[it->n] += 1. - d((*g)(), mu);
		}
		for (auto arr = it->arrangements->begin(); arr != it->arrangements->end(); ++arr) {
			for (auto& c : *arr->second.table) {
				for (auto j = 1; j < c; ++j) {
					bernoulli_distribution::param_type mu( ((double)j-1)/((double)j-(*_discount)[it->n]) );
					b[it->n] += 1. - d((*g)(), mu);
				}
			}
		}
	}
	/*
	   for (auto it = _nc->begin(); it != _nc->end(); ++it) {
	   if (it->n < 0) // erased node
	   continue;
	   for (auto& c : *it->table) {
	   for (auto j = 1; j < c; ++j) {
	   bernoulli_distribution::param_type mu( ((double)j-1)/((double)j-(*_discount)[it->n-1]) );
	   b[it->n-1] += 1. - d((*g)(), mu);
	   }
	   }
	   }
	   */
}

void hpy::_estimate_t(vector<double>& a, vector<double>& b) {
	shared_ptr<generator> g = generator::create();
	bernoulli_distribution d;
	beta_distribution be;
	for (auto it = _nz->begin(); it != _nz->end(); ++it) {
		if (it->n < 0) // erased node
			continue;
		for (auto i = 1; i < it->table; ++i) {
			bernoulli_distribution::param_type mu((*_strength)[it->n]/((*_strength)[it->n]+(*_discount)[it->n]*i));
			a[it->n] += d((*g)(), mu);
		}
		if (it->table > 1) {
			b[it->n] += log(be((*_strength)[it->n]+1,(double)it->customer-1));
		}
	}
}

double hpy::lp(sentence& s) {
	double lpr = 0;
	for (auto i = 0; i <= s.size(); ++i) {
		lpr += lp(s, i);
	}
	return lpr;
}

double hpy::lp(word& w) {
	double lpr = 0;
	for (auto i = 0; i <= w.len; ++i) {
		lpr += lp(w, i);
	}
	return lpr;
}

double hpy::lp(sentence& s, int i) {
	return _lp(s, i, _n);
}

double hpy::lp(word& w, int i) {
	return _lp(w, i, _n);
}

int hpy::save(const char *file) {
	FILE *fp = NULL;
	if ((fp = fopen(file, "wb")) == NULL)
		return 1;
	if (fwrite(&_n, sizeof(int), 1, fp) != 1)
		return 1;
	if (fwrite(&_v, sizeof(int), 1, fp) != 1)
		return 1;
	fclose(fp);
	//string nc(file);
	//nc += ".nc";
	string nz(file);
	nz += ".nz";
	//_nc->save(nc.c_str(), arrangement::val_writer, arrangement::key_writer);
	_nz->save(nz.c_str(), restaurant::val_writer, restaurant::key_writer);

	return 0;
}

int hpy::load(const char *file) {
	FILE *fp = NULL;
	if ((fp = fopen(file, "rb")) == NULL)
		return 1;
	if (fread(&_n, sizeof(int), 1, fp) != 1)
		return 1;
	if (fread(&_v, sizeof(int), 1, fp) != 1)
		return 1;
	fclose(fp);
	//string nc(file);
	//nc += ".nc";
	string nz(file);
	nz += ".nz";
	//_nc->load(nc.c_str(), arrangement::val_reader, arrangement::key_reader);
	_nz->load(nz.c_str(), restaurant::val_reader, restaurant::key_reader);

	return 0;
}

double hpy::_lp(sentence& s, int i, int n) {
	double lp = -log(_v); // base measure
	if (n == 0)
		return lp;
	//auto c = _nc->cs_search(s, i, n);
	auto z = _nz->cs_search(s, i-1, n-1);
	for (auto j = 0; j < z.size(); ++j) {
		auto& r = _nz->val(z[j].second);
		double tu = r.table;
		double cu = r.customer;
		double b = (*_strength)[j]+(*_discount)[j]*tu;
		double d = (*_strength)[j]+cu;
		lp += log(b) - log(d);
		auto it = r.arrangements->find(s[i]);
		if (it != r.arrangements->end()) {
			auto& a = it->second;
			//auto a = _nc->getval(c[j].second);
			double tuk = a.table->size();
			double cuk = a.customer;
			double p = cuk-(*_discount)[j]*tuk;
			lp = math::lse(lp, log(p)-log(d));
		}
	}
	return lp;
}

double hpy::_lp(word& w, int i, int n) {
	double lp = -log(_v); // base measure
	if (n == 0)
		return lp;
	//auto c = _nc->cs_search(w, i, n);
	auto z = _nz->cs_search(w, i-1, n-1);
	for (auto j = 0; j < z.size(); ++j) {
		auto& r = _nz->val(z[j].second);
		//auto r = _nz->getval(z[j-1].second);
		double tu = r.table;
		double cu = r.customer;
		double b = (*_strength)[j]+(*_discount)[j]*tu;
		double d = (*_strength)[j]+cu;
		lp += log(b) - log(d);
		auto it = r.arrangements->find(w[i]);
		if (it != r.arrangements->end()) {
			auto& a = it->second;
			//auto a = _nc->getval(c[j].second);
			double tuk = a.table->size();
			double cuk = a.customer;
			double p = cuk-(*_discount)[j]*tuk;
			lp = math::lse(lp, log(p)-log(d));
		}
	}
	return lp;
}
