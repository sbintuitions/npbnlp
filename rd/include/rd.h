#ifndef NPBNLP_RD_H
#define NPBNLP_RD_H
#include"math.h"
#include"seed.h"
#include"generator.h"
#include"uniform.h"
#include<random>
#include<utility>
#include<vector>
namespace npbnlp {
	class rd {
		public:
			static void shuffle(int *a, int size) {
				std::shared_ptr<generator> r = generator::create();
				for (int i = 0; i < size; ++i)
					*(a+i) = i;
				for (int i = 0; i < size; ++i)
					std::swap(*(a+i),*(a+(*r)()()%(i+1)));
			}
			static int draw(double z, std::vector<double>& t) {
				if (t.size() == 1)
					return 0;
				uniform u;
				double r = u(0, z);
				double c = 0;
				int i = 0;
				while (c < r)
					c += t[i++];
				return i-1;
			}
			static int ln_draw(std::vector<double>& t) {
				if (t.size() == 1)
					return 0;
				double z = 0;
				for (int i = 0; i < (int)t.size(); ++i)
					z = math::lse(z, t[i], (i == 0));
				std::vector<double> p(t.size(), 0);
				double c = 0;
				for (int i = 0; i < (int)t.size(); ++i) {
					p[i] = std::exp(t[i]-z);
					c += p[i];
				}
				return draw(c, p);
			}
			static int best(std::vector<double>& t) {
				if (t.size() == 1)
					return 0;
				double m = t[0];
				int j = 0;
				for (int i = 1; i < (int)t.size(); ++i) {
					if (m < t[i]) {
						m = t[i];
						j = i;
					}
				}
				return j;
			}
		private:
	};
}
#endif
