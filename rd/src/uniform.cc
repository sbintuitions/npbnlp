#include"uniform.h"
#include"generator.h"
#include<random>

using namespace npbnlp;
using namespace std;

uniform::uniform() {
}

uniform::~uniform() {
}

double uniform::operator()(double min, double max) {
	uniform_real_distribution<double> u(min, max);
	shared_ptr<generator> g = generator::create();
	return u((*g)());
}
