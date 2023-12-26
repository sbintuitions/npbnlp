#ifndef NPBNLP_MATH_H
#define NPBNLP_MATH_H

#include<cmath>
#include<algorithm>

namespace npbnlp {
	class math {
		public:
			static double lse(double x, double y, double f=false) {
				if (f)
					return y;
				if (x == y)
					return x + 0.69314718055;
				double min = std::min(x, y);
				double max = std::max(x, y);
				double diff = max - min;
				if (diff > 50)
					return max;
				else
					return max+std::log(1.+std::exp(min-max));
			}

	};
}
#endif
