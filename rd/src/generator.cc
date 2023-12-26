#include"seed.h"
#include"generator.h"

using namespace std;
using namespace npbnlp;

mutex generator::_mutex;
shared_ptr<generator> generator::_gn;

shared_ptr<generator> generator::create() {
	lock_guard<mutex> lock(_mutex);
	if (_gn == nullptr) {
		_gn = shared_ptr<generator>(new generator);
	}
	return _gn;
}


generator::generator() {
	shared_ptr<seed> s = seed::create();
#ifdef _OPENMP
	int t = omp_get_max_threads();
	_g = shared_ptr<gen_t>(new sp(t));
	for (int i = 0; i < t; ++i)
		(*_g)[i] = mt19937((*s)()); 
#else
	_g = shared_ptr<sp>(new mt19937((*s)()));
#endif
}

generator::~generator() {
}

generator::gen_t& generator::operator()() {
#ifdef _OPENMP
	return (*_g)[omp_get_thread_num()];
#else
	return (*_g);
#endif
}
