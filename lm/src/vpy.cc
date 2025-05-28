#include"vpy.h"
#include"math.h"
#include"rd.h"

using namespace npbnlp;
using namespace std;

vpy::vpy():hpy() {
}

vpy::vpy(int n):hpy(n) {
}

vpy::~vpy() {
}

double vpy::lp(word& w) {
	double lpr = 0;
	for (auto i = 0; i <= w.len; ++i) {
		lpr += lp(w, i);
	}
	return lpr;
}

double vpy::lp(sentence& s) {
	double lpr = 0;
	for (auto i = 0; i <= s.size(); ++i) {
		lpr += lp(s, i);
	}
	return lpr;
}

double vpy::lp(word& w, int i) {
	return _lp(w, i, _n);
}

double vpy::lp(sentence& s, int i) {
	return _lp(s, i, _n);
}

bool vpy::add(word& w) {
	for (auto i = 0; i <= w.len; ++i) {
		int n = _draw_n(w, i);
		_add(w, i, n);
		w.m[i] = n;
	}
	return true;
}

bool vpy::add(sentence& s) {
	for (auto i = 0; i <= s.size(); ++i) {
		int n = _draw_n(s, i);
		bool add_to_base = _add(s, i, n);
		s.n[i] = n;
		if (add_to_base && _base) {
			if (_bc == nullptr)
				_bc = shared_ptr<base_corpus>(new base_corpus);
			word w = s.wd(i);
			_base->add(w);
			(*_bc)[w.id].emplace_back(w);
		}
	}
	return true;
}

bool vpy::remove(word& w) {
	for (auto i = 0; i <= w.len; ++i) {
		int n = w.m[i];
		_remove(w, i, n);
	}
	return true;
}

bool vpy::remove(sentence& s) {
	for (auto i = 0; i <= s.size(); ++i) {
		int n = s.n[i];
		bool remove_from_base = _remove(s, i, n);
		if (remove_from_base && _base) {
			int size = (*_bc)[s[i]].size();
			int id = (*generator::create())()()%size;
			word& b = (*_bc)[s[i]][id];
			_base->remove(b);
			(*_bc)[s[i]].erase((*_bc)[s[i]].begin()+id);
		}
	}
	if (_bc && _bc->empty())
		_bc = nullptr;
	return true;
}

double vpy::_lp(word& w, int i, int n) {
	if (n == 0)
		return -log(_v);
	double lp = 0;
	double z = 0;
	auto c = _nz->cs_search(w, i-1, n-1);
	for (auto j = c.size(); j > 0; --j) {
		auto& rst = _nz->val(c[j-1].second);
		double ln_pr_stop = log(rst.stop)-log(rst.stop+rst.pass);
		double ln_pr_pass = log(rst.pass)-log(rst.stop+rst.pass);
		z = math::lse(z+ln_pr_pass, ln_pr_stop, (z==0));
		lp = math::lse(lp+ln_pr_pass, ln_pr_stop+hpy::_lp(w, i, j), (lp == 0));

	}
	return lp-z;
}

double vpy::_lp(sentence& s, int i, int n) {
	if (n == 0 && _base)
		return _base->lp(s.wd(i));
	else if (n ==0)
		return -log(_v);
	double lp = 0;
	double z = 0;
	auto c = _nz->cs_search(s, i-1, n-1);
	for (auto j = c.size(); j > 0; --j) {
		auto& rst = _nz->val(c[j-1].second);
		double ln_pr_stop = log(rst.stop)-log(rst.stop+rst.pass);
		double ln_pr_pass = log(rst.pass)-log(rst.stop+rst.pass);
		z = math::lse(z+ln_pr_pass, ln_pr_stop, (z==0));
		lp = math::lse(lp+ln_pr_pass, ln_pr_stop+hpy::_lp(s, i, j), (lp == 0));

	}
	return lp-z;
}

bool vpy::_add(word& w, int i, int n) {
	if (n == 0)
		return true;
	double lpr = _lp(w, i, n-1);
	auto z = _nz->rexactmatch(w, i-1, n-1);
	if (z < 0) {
		_nz->insert(w, i-1, n-1);
		z = _nz->rexactmatch(w, i-1, n-1);
	}
	auto& rst = _nz->val(z); rst.n = n-1;
	auto it = rst.arrangements->find(w[i]);
	if (it == rst.arrangements->end())
		(*rst.arrangements)[w[i]] = arrangement(n);
	auto& arr = (*rst.arrangements)[w[i]];
	++rst.customer;
	++rst.stop;
	auto c = _nz->cs_search(w, i-1, n-2);
	for (auto j = 0; j < c.size(); ++j) {
		auto& r = _nz->val(c[j].second);
		++r.pass;
	}
	++arr.customer;
	int size = arr.table->size();
	vector<double> t(size+1, 0);
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
	return false;
}

bool vpy::_add(sentence& s, int i, int n) {
	if (n == 0)
		return true;
	double lpr = _lp(s, i, n-1);
	auto z = _nz->rexactmatch(s, i-1, n-1);
	if (z < 0) {
		_nz->insert(s, i-1, n-1);
		z = _nz->rexactmatch(s, i-1, n-1);
	}
	auto& rst = _nz->val(z); rst.n = n-1;
	auto it = rst.arrangements->find(s[i]);
	if (it == rst.arrangements->end())
		(*rst.arrangements)[s[i]] = arrangement(n);
	auto& arr = (*rst.arrangements)[s[i]];
	++rst.customer;
	++rst.stop;
	auto c = _nz->cs_search(s, i-1, n-2);
	for (auto j = 0; j < c.size(); ++j) {
		auto& r = _nz->val(c[j].second);
		++r.pass;
	}
	++arr.customer;
	int size = arr.table->size();
	vector<double> t(size+1, 0);
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
	return false;
}

bool vpy::_remove(word& w, int i, int n) {
	if (n == 0)
		return true;
	auto z = _nz->cs_search(w, i-1, n-1);
	for (auto j = 0; j < z.size()-1; ++j) {
		auto& r = _nz->val(z[j].second);
		--r.pass;
	}
	auto& rst = _nz->val(z[z.size()-1].second);
	--rst.customer;
	--rst.stop;
	auto& arr = (*rst.arrangements)[w[i]];
	--arr.customer;
	double r = 0;
	int size = arr.table->size();
	vector<double> t(size, 0);
	for (auto j = 0; j < size; ++j) {
		t[j] = (*arr.table)[j];
		r += t[j];
	}
	int id = rd::draw(r, t);
	(*arr.table)[id]--;
	if ((*arr.table)[id] == 0) {
		--rst.table;
		arr.table->erase(arr.table->begin()+id);
		if (arr.customer == 0) {
			rst.arrangements->erase(w[i]);
			if (n == 1)
				--_v;
		}
		if (rst.customer == 0)
			_nz->erase(w, i-1, n-1);
		return _remove(w, i, n-1);
	}
	return false;
}

bool vpy::_remove(sentence& s, int i, int n) {
	if (n == 0)
		return true;
	auto z = _nz->cs_search(s, i-1, n-1);
	for (auto j = 0; j < z.size()-1; ++j) {
		auto& r = _nz->val(z[j].second);
		--r.pass;
	}
	auto& rst = _nz->val(z[z.size()-1].second);
	--rst.customer;
	--rst.stop;
	auto& arr = (*rst.arrangements)[s[i]];
	--arr.customer;
	double r = 0;
	int size = arr.table->size();
	vector<double> t(size, 0);
	for (auto j = 0; j < size; ++j) {
		t[j] = (*arr.table)[j];
		r += t[j];
	}
	int id = rd::draw(r, t);
	(*arr.table)[id]--;
	if ((*arr.table)[id] == 0) {
		--rst.table;
		arr.table->erase(arr.table->begin()+id);
		if (arr.customer == 0) {
			rst.arrangements->erase(s[i]);
			if (n == 1)
				--_v;
		}
		if (rst.customer == 0)
			_nz->erase(s, i-1, n-1);
		return _remove(s, i, n-1);
	}
	return false;
}

int vpy::_draw_n(word& w, int i) {
	vector<double> table;
	double ln_pr_pass = 0;
	double ln_pr_stop = 0;
	double lp_cache = 0;
	auto z = _nz->cs_search(w, i-1, _n-1);
	for (auto j = 0; j < _n; ++j) {
		if (j < z.size()) {
			auto& r = _nz->val(z[j].second);
			ln_pr_stop = log(r.stop)-log(r.stop+r.pass);
			ln_pr_pass += log(r.stop)-log(r.stop+r.pass);
			lp_cache = _lp(w, i, j);
		} else {
			ln_pr_stop = -log(2);
			ln_pr_pass += -log(2);
		}
		table.emplace_back(lp_cache+ln_pr_stop+ln_pr_pass);
	}
	return 1+rd::ln_draw(table);
}

int vpy::_draw_n(sentence& s, int i) {
	vector<double> table;
	double ln_pr_pass = 0;
	double ln_pr_stop = 0;
	double lp_cache = 0;
	auto z = _nz->cs_search(s, i-1, _n-1);
	for (auto j = 0; j < _n; ++j) {
		if (j < z.size()) {
			auto& r = _nz->val(z[j].second);
			ln_pr_stop = log(r.stop)-log(r.stop+r.pass);
			ln_pr_pass += log(r.stop)-log(r.stop+r.pass);
			lp_cache = _lp(s, i, j);
		} else {
			ln_pr_stop = -log(2);
			ln_pr_pass += -log(2);
		}
		table.emplace_back(lp_cache+ln_pr_stop+ln_pr_pass);
	}
	return 1+rd::ln_draw(table);
}

