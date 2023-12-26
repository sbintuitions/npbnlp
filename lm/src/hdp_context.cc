#include"hdp_context.h"

using namespace std;
using namespace npbnlp;

hdp_context::hdp_context(): context(), _child(new hdp_context::children) {
}

hdp_context::hdp_context(int k, context *h):context(k, h), _child(new hdp_context::children) {
}

hdp_context::~hdp_context() {
}

context* hdp_context::find(int k) {
	auto it = _child->find(k);
	if (it == _child->end())
		return NULL;
	else
		return it->second.get();
		//return &(*it->second);
}

context* hdp_context::make(int k) {
	context *c = find(k);
	if (c)
		return c;
	else {
		lock_guard<mutex> m(_mutex);
		(*_child)[k] = shared_ptr<hdp_context>(new hdp_context(k, this));
		return (*_child)[k].get();
		//return &(*(*_child)[k]);
	}

}

bool hdp_context::add(int k, lm *m) {
	++_stop;
	context *c = this;
	while ((c = c->parent()))
		c->incr_pass();
	return _crp_add(k, m);
}

bool hdp_context::remove(int k) {
	--_stop;
	context *c = this;
	while ((c = c->parent()))
		c->decr_pass();
	return _crp_remove(k);

}

bool hdp_context::_crp_add(int k, lm *m) {
	++_customer;
	vector<int>& r = *_get_restaurant(k);
	int size = r.size();
	vector<double> table(size+1, 0);
	double z = 0;
	for (int i = 0; i < size; ++i) {
		table[i] = r[i];
		z += table[i];
	}
	table[size] = m->alpha(_n)*m->pr(k, _parent);
	z += table[size];
	int id = rd::draw(z, table);
	if (id == size) {
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

/*
bool hdp_context::_crp_remove(int k) {
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
		if (r.size() == 0)
			_restaurant->erase(k);
		return true;
	}
	return false;
}
*/

void hdp_context::estimate_a(vector<double>& a, vector<double>& b, lm *m) {
	for (auto it = _child->begin(); it != _child->end(); ++it)
		it->second->estimate_a(a, b, m);

	shared_ptr<generator> g = generator::create();
	bernoulli_distribution d;
	beta_distribution be;
	if (_n == 0) {
		bernoulli_distribution::param_type mu((double)_table/(_table+m->alpha(_n)));
		a[_n] = _restaurant->size()-1+d((*g)(), mu);
		b[_n] = log(be(1.+m->alpha(_n),_table));
	} else if (_n > 0) {
		bernoulli_distribution::param_type mu((double)_customer/(_customer+m->alpha(_n)));
		a[_n] += _table - d((*g)(), mu);
		b[_n] += log(be(1.+m->alpha(_n),_customer));
	}

	/*
	for (auto it = _restaurant->begin(); it != _restaurant->end(); ++it) {
		double cu = 0;
		for (auto c = it->second->begin(); c != it->second->end(); ++c) {
			cu += *c;
		}
		bernoulli_distribution::param_type mu(cu/m->alpha(_n));
		a[_n] -= d((*g)(), mu);
		b[_n] += log(be(1.+m->alpha(_n), cu));
	}
	*/

}

/*
void hdp_context::save(FILE *fp) {
	if (fwrite(&_k, sizeof(int), 1, fp) != 1)
		throw "failed to write _k in hdp_context::save";
	if (fwrite(&_n, sizeof(int), 1, fp) != 1)
		throw "failed to write _n in hdp_context::save";
	if (fwrite(&_customer, sizeof(int), 1, fp) != 1)
		throw "failed to write _customer in hdp_context::save";
	if (fwrite(&_table, sizeof(int), 1, fp) != 1)
		throw "failed to write _table in hdp_context::save";
	if (fwrite(&_a, sizeof(int), 1, fp) != 1)
		throw "failed to write _a in hdp_context::save";
	if (fwrite(&_b, sizeof(int), 1, fp) != 1)
		throw "failed to write _b in hdp_context::save";
	if (fwrite(&_stop, sizeof(int), 1, fp) != 1)
		throw "failed to write _stop in hdp_context::save";
	if (fwrite(&_pass, sizeof(int), 1, fp) != 1)
		throw "failed to write _pass in hdp_context::save";
	int rest_size = _restaurant->size();
	if (fwrite(&rest_size, sizeof(int), 1, fp) != 1)
		throw "failed to write rest_size in hdp_context::save";
	for (auto it = _restaurant->begin(); it != _restaurant->end(); ++it) {
		if (fwrite(&it->first, sizeof(int), 1, fp) != 1)
			throw "failed to write dish in hdp_context::save";
		int tables = it->second->size();
		if (fwrite(&tables, sizeof(int), 1, fp) != 1)
			throw "failed to write table num in hdp_context::save";
		if (fwrite(&(*it->second)[0], sizeof(int), tables, fp) != tables)
			throw "failed to write restaurant::table in hdp_context::save";
	}
	int childs = _child->size();
	if (fwrite(&childs, sizeof(int), 1, fp) != 1)
		throw "failed to write children num in hdp_context::save";
	for (auto it = _child->begin(); it != _child->end(); ++it) {
		if (fwrite(&it->first, sizeof(int), 1, fp) != 1)
			throw "failed to write child key in hdp_context::save";
		it->second->save(fp);
	}
}

void hdp_context::load(FILE *fp) {
}
*/
