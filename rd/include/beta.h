#ifndef NPBNLP_BETA_H
#define NPBNLP_BETA_H

#include<random>
namespace npbnlp {
	class beta_distribution {
		public:
			beta_distribution();
			virtual ~beta_distribution();
			double operator()(double a, double b);
			double density(double x, double a, double b);
		private:
			std::gamma_distribution<double> _g;
			std::gamma_distribution<double> _f;
	};
}
#endif
