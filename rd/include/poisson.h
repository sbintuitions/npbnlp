#ifndef NPBNLP_POISSON_H
#define NPBNLP_POISSON_H

namespace npbnlp {
	class poisson_distribution {
		public:
			poisson_distribution();
			virtual ~poisson_distribution();
			double operator()(double lambda, int k);
			double lp(double lambda, int k);
		private:
			double factorial(double p, int k);
	};
}

#endif
