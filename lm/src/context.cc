#include"rd.h"
#include"context.h"
#include"beta.h"
#include"wordtype.h"
#include<random>
#include<fstream>
#ifdef _OPENMP
#include<omp.h>
#endif

using namespace npbnlp;
using namespace std;

context::context():_k(-1),_n(0),_customer(0),_table(0),_a(1),_b(1),_stop(0),_pass(0),_parent(NULL),_child(new children),_restaurant(new arrangement) {
}

context::context(int k, context *h):_k(k), _n(h->n()+1),_customer(0),_table(0),_a(h->a()),_b(h->b()),_stop(0),_pass(0),_parent(h),_child(new children),_restaurant(new arrangement) {
}

context::~context() {
}

context* context::parent() const {
	return _parent;
}

int context::n() const {
	return _n;
}

int context::a() const {
	return _a;
}

int context::b() const {
	return _b;
}

int context::c() const {
	return _customer;
}

int context::t() const {
	return _table;
}

int context::stop() const {
	return _a+_stop;
}

int context::pass() const {
	return _b+_pass;
}

int context::cu(int k) const {
	const auto it = _restaurant->find(k);
	int n = 0;
	if (it == _restaurant->cend())
		return n;
	/*
#ifdef _OPENMP
#pragma omp parallel for reduction(+:n)
#endif
*/
	for (auto i = it->second->cbegin(); i != it->second->cend(); ++i) {
		n += *i;
	}
	return n;
}

int context::tu(int k) const {
	const auto it = _restaurant->find(k);
	if (it == _restaurant->cend())
		return 0;
	else
		return it->second->size();
}


void context::set(int a, int b) {
	_a = a;
	_b = b;
}

context* context::find(int k) const {
	auto const it = _child->find(k);
	if (it == _child->cend())
		return NULL;
	else
		return it->second.get();
		//return &(*it->second);
}

context* context::make(int k) {
	context *c = find(k);
	if (c)
		return c;
	else {
		lock_guard<mutex> m(_mutex);
		//c = new context(k, this);
		(*_child)[k] = shared_ptr<context>(new context(k, this));
		return (*_child)[k].get();
		//return &(*(*_child)[k]);
		//return c;
	}
}

void context::incr_stop() {
	++_stop;
}

void context::decr_stop() {
	--_stop;
}

void context::incr_pass() {
	++_pass;
}

void context::decr_pass() {
	--_pass;
}

bool context::add(int k, lm *m) {
	++_stop;
	context *c = this;
	while ((c = c->parent()))
		++c->_pass;
	return _crp_add(k, m);
}

bool context::remove(int k) {
	--_stop;
	context *c = this;
	while ((c = c->parent()))
		--c->_pass;
	return _crp_remove(k);
}

int context::v() const {
	return _restaurant->size();
}

int context::sample(lm *m) {
	context *h = this;
	for (; h->_parent;)
		h = h->_parent;
	vector<double> pr;
	vector<int> k;
	auto it = h->_restaurant->cbegin();
	for (; it != h->_restaurant->cend(); ++it) {
		double p = m->lp(it->first, this);
		pr.push_back(p);
		k.push_back(it->first);
	}
	return k[rd::ln_draw(pr)];
}

shared_ptr<vector<int> > context::_get_restaurant(int k) {
	auto it = _restaurant->find(k);
	if (it != _restaurant->end()) {
		return it->second;
	} else {
		lock_guard<mutex> m(_mutex);
		(*_restaurant)[k] = make_shared<vector<int> >();
		return (*_restaurant)[k];
	}
}

bool context::_crp_add(int k, lm *m) {
	++_customer;
	vector<int>& r = *_get_restaurant(k);
	int size = r.size();
	vector<double> table(size+1,0);
	double z = 0;
	for (int i = 0; i < size; ++i) {
		table[i] = r[i] - m->discount(_n);
		z += table[i];
	}
	table[size] = (m->strength(_n) + _table * m->discount(_n))*m->pr(k, _parent); 
	z += table[size];
	int id = rd::draw(z, table);
	if (id == size) { // sit down new table
		lock_guard<mutex> m(_mutex);
		r.push_back(1);
		++_table;
		return true;
	} else {
		lock_guard<mutex> m(_mutex);
		r[id]++;
		return false;
	}
}

bool context::_crp_remove(int k) {
	--_customer;
	vector<int> &r = *_get_restaurant(k);
	int size = r.size();
	vector<double> table(size, 0);
	double z = 0;
	for (int i = 0; i < size; ++i) {
		table[i] = r[i];
		z += table[i];
	}
	int id = rd::draw(z, table);
	lock_guard<mutex> m(_mutex);
	r[id]--;
	if (r[id] == 0) {
		--_table;
		r.erase(r.begin()+id);
		if (r.size() == 0) {
			_restaurant->erase(k);
		}
		if (_restaurant->size() == 0) // this context has no customer 
			if (_parent)
				_parent->_child->erase(_k);
		return true;
	}
	return false;
}

//void context::estimate_l(vector<double>& a, vector<double>& b, vector<int>& f, unordered_map<int, vector<word> > *corpus) {
void context::estimate_l(vector<double>& a, vector<double>& b, unordered_map<int, vector<word> > *corpus) {
	for (auto it = _restaurant->begin(); it != _restaurant->end(); ++it) {
		word& w = (*corpus)[it->first][0];
		type t = wordtype::get(w);
		a[t] += w.len * it->second->size();
		b[t] += it->second->size();
		//++f[t];
	}
}

void context::estimate_d(vector<double>& a, vector<double>& b, lm *m) {
	for (auto it = _child->begin(); it != _child->end(); ++it) {
		it->second->estimate_d(a, b, m);
	}
	shared_ptr<generator> g = generator::create();
	bernoulli_distribution d;
	double y = 0;
	/*
#ifdef _OPENMP
#pragma omp parallel for reduction(+:y)
#endif
*/
	for (int i = 0; i < _table; ++i) {
		bernoulli_distribution::param_type mu( m->strength(_n)/(m->strength(_n)+m->discount(_n)*i) );
		y += 1. - d((*g)(), mu);
	}
	double z = 0;
	for (auto it = _restaurant->begin(); it != _restaurant->end(); ++it) {
		/*
#ifdef _OPENMP
#pragma omp parallel for reduction(+:z)
#endif
*/
		for (auto c = it->second->begin(); c != it->second->end(); ++c) {
			for (int j = 1; j < *c; ++j) {
				bernoulli_distribution::param_type mu( ((double)j-1)/((double)j-m->discount(_n)) );
				z += 1. - d((*g)(), mu);
			}
		}
	}
	a[_n] += y;
	b[_n] += z;
}

void context::estimate_t(vector<double>&a , vector<double>& b, lm *m) {
	for (auto it = _child->begin(); it != _child->end(); ++it) {
		it->second->estimate_t(a, b, m);
	}
	shared_ptr<generator> g = generator::create();
	bernoulli_distribution d;
	double y = 0;
	/*
#ifdef _OPENMP
#pragma omp parallel for reduction(+:y)
#endif
*/
	for (int i = 1; i < _table; ++i) {
		bernoulli_distribution::param_type mu( m->strength(_n)/(m->strength(_n)+m->discount(_n)*i) );
		y += d((*g)(), mu);
	}
	double x = 0;
	if (_table > 1) {
		beta_distribution be;
		x = log(be(m->strength(_n)+1,(double)_customer-1));
	}
	a[_n] += y;
	b[_n] += x;
}

void context::save(FILE *fp) {
	if (fwrite(&_k, sizeof(int), 1, fp) != 1)
		throw "failed to write _k in context::save";
	if (fwrite(&_n, sizeof(int), 1, fp) != 1)
		throw "failed to write _n in context::save";
	if (fwrite(&_customer, sizeof(int), 1, fp) != 1)
		throw "failed to write _customer in context::save";
	if (fwrite(&_table, sizeof(int), 1, fp) != 1)
		throw "failed to write _table in context::save";
	if (fwrite(&_a, sizeof(int), 1, fp) != 1)
		throw "failed to write _a in context::save";
	if (fwrite(&_b, sizeof(int), 1, fp) != 1)
		throw "failed to write _b in context::save";
	if (fwrite(&_stop, sizeof(int), 1, fp) != 1)
		throw "failed to write _stop in context::save";
	if (fwrite(&_pass, sizeof(int), 1, fp) != 1)
		throw "failed to write _pass in context::save";
	int rests = _restaurant->size();
	if (fwrite(&rests, sizeof(int), 1, fp) != 1)
		throw "failed to write _restaurant->size() in context::save";
	for (auto it = _restaurant->begin(); it != _restaurant->end(); ++it) {
		if (fwrite(&it->first, sizeof(int), 1, fp) != 1)
			throw "failed to write restaurant::key in context::save";
		int tables = it->second->size();
		if (fwrite(&tables, sizeof(int), 1, fp) != 1)
			throw "failed to write restaurant::table_size in context::save";
		if (fwrite(&(*it->second)[0], sizeof(int), tables, fp) != tables)
			throw "failed to write restaurant::table in context::save";

	}
	int childs = _child->size();
	if (fwrite(&childs, sizeof(int), 1, fp) != 1)
		throw "failed to write _child->size() in context::save";
	for (auto it = _child->begin(); it != _child->end(); ++it) {
		if (fwrite(&it->first, sizeof(int), 1, fp) != 1)
			throw "failed to write children::key in context::save";
		it->second->save(fp);
	}
}

void context::load(FILE *fp) {
	if (fread(&_k, sizeof(int), 1, fp) != 1)
		throw "failed to read _k in context::load";
	if (fread(&_n, sizeof(int), 1, fp) != 1)
		throw "failed to read _n in context::load";
	if (fread(&_customer, sizeof(int), 1, fp) != 1)
		throw "failed to read _customer in context::load";
	if (fread(&_table, sizeof(int), 1, fp) != 1)
		throw "failed to read _table in context::load";
	if (fread(&_a, sizeof(int), 1, fp) != 1)
		throw "failed to read _a in context::load";
	if (fread(&_b, sizeof(int), 1, fp) != 1)
		throw "failed to read _b in context::load";
	if (fread(&_stop, sizeof(int), 1, fp) != 1)
		throw "failed to read _stop in context::load";
	if (fread(&_pass, sizeof(int), 1, fp) != 1)
		throw "failed to read _pass in context::load";
	int rests = _restaurant->size();
	if (fread(&rests, sizeof(int), 1, fp) != 1)
		throw "failed to read _restaurant->size() in context::load";
	for (int i = 0; i < rests; ++i) {
		int key;
		if (fread(&key, sizeof(int), 1, fp) != 1)
			throw "failed to read restaurant::key in context::load";
		int tables;
		if (fread(&tables, sizeof(int), 1, fp) != 1)
			throw "failed to read restaurant::table_size in context::load";
		vector<int>& r = *_get_restaurant(key);
		r.resize(tables);
		if (fread(&r[0], sizeof(int), tables, fp) != tables)
			throw "failed to read restaurant::table in context::load";
	}
	int childs;
	if (fread(&childs, sizeof(int), 1, fp) != 1)
		throw "failed to read _child->size() in context::load";
	for (int i = 0; i < childs; ++i) {
		int key;
		if (fread(&key, sizeof(int), 1, fp) != 1)
			throw "failed to read children::key in context::load";

		shared_ptr<context> p = shared_ptr<context>(new context(key, this));
		p->load(fp);
		lock_guard<mutex> m(_mutex);
		(*_child)[key] = p;
	}
}

