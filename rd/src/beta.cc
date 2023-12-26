#include"beta.h"
#include"seed.h"
#include"generator.h"
#include<random>
#include<cmath>

using namespace std;
using namespace npbnlp;
using dist_type = gamma_distribution<>;

beta_distribution::beta_distribution() {
}

beta_distribution::~beta_distribution() {
}

double beta_distribution::operator()(double a, double b) {
	dist_type::param_type p1(a, 1);
	dist_type::param_type p2(b, 1);
	_g.param(p1);
	_f.param(p2);
	shared_ptr<generator> g = generator::create();
	double x = _g((*g)());
	double y = _f((*g)());
	return x/(x+y);
}

double beta_distribution::density(double x, double a, double b) {
	return pow(x, a-1.)*pow(1.-x, b-1.)/exp(log(abs(tgamma(a)*tgamma(b)/tgamma(a+b))));
}
