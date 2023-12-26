#include"cache.h"
#ifdef _OPENMP
#include<omp.h>
#endif

using namespace std;
using namespace npbnlp;

cache::cache() {
#ifdef _OPENMP
	_c.resize(omp_get_max_threads());
#endif
}

cache::~cache() {
}

double cache::get(int k, const context *h, bool& chk) {
#ifdef _OPENMP
	auto i = _c[omp_get_thread_num()].find(h);
	if (i == _c[omp_get_thread_num()].end())
#else
	auto i = _c.find(h);
	if (i == _c.end())
#endif
	{
		chk = false;
		return 0;
	} else {
		auto j = i->second.find(k);
		if (j == i->second.end()) {
			chk = false;
			return 0;
		} else {
			chk = true;
			return j->second;
		}
	}
}

double cache::set(int k, const context *h, double lp) {
#ifdef _OPENMP
	_c[omp_get_thread_num()][h][k] = lp;
	return _c[omp_get_thread_num()][h][k];
#else
	_c[h][k] = lp;
	return _c[h][k];
#endif
}

double cache::get(word& w, const context *h, bool& chk) {
	if (w.id == 1) { // unk
		chk = false;
		return 0;
	}
	return get(w.id, h, chk);
}

double cache::set(word& w, const context *h, double lp) {
	if (w.id == 1)
		return lp;
	return set(w.id, h, lp);
}

double cache::get(chunk& c, const context *h, bool& chk) {
	if (c.id == 1) { // unk
		chk = false;
		return 0;
	}
	return get(c.id, h, chk);
}

double cache::set(chunk& c, const context *h, double lp) {
	if (c.id == 1)
		return lp;
	return set(c.id, h, lp);
}

void cache::clear() {
	_c.clear();
#ifdef _OPENMP
	_c.resize(omp_get_max_threads());
#endif
}
