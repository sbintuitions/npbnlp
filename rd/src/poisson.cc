#include"poisson.h"
#include<cmath>
using namespace std;
using namespace npbnlp;

poisson_distribution::poisson_distribution() {
}

poisson_distribution::~poisson_distribution() {
}

double poisson_distribution::operator()(double lambda, int k) {
	return factorial(pow(lambda, k)*exp(-lambda), k);
}

double poisson_distribution::factorial(double p, int k) {
	if (k <= 1)
		return p;
	return factorial(p/k, k-1);
}

double poisson_distribution::lp(double lambda, int k) {
	double lp = k*log(lambda)-lambda;
	for (auto i = 2; i <= k; ++i)
		lp -= log(i);
	return lp;
}
