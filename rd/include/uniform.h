#ifndef NPBNLP_UNIFORM_H
#define NPBNLP_UNIFORM_H

namespace npbnlp {
	class uniform {
		public:
			uniform();
			virtual ~uniform();
			double operator()(double min, double max);
		private:
	};
}

#endif
