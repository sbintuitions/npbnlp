#include"seed.h"

using namespace std;
using namespace npbnlp;

shared_ptr<seed> seed::_sd;
mutex seed::_mutex;
shared_ptr<seed> seed::create() {
	lock_guard<mutex> lock(_mutex);
	if (_sd == nullptr)
		_sd = shared_ptr<seed>(new seed);
	return _sd;
}

seed::seed() {
}

seed::~seed() {
}

unsigned int seed::operator()() {
	return _seed();
}
