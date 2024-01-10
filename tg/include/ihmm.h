#ifndef NPBNLP_IHMM
#define NPBNLP_IHMM

#include"io.h"
#include"hpyp.h"
#include"vpyp.h"
#include"vtable.h"
#include"hlattice.h"
#include<functional>
#include<memory>

namespace npbnlp {
	class ihmm {
		public:
			ihmm();
			ihmm(int n, int m, int k);
			virtual ~ihmm();
			virtual sentence sample(io& f, int i);
			virtual sentence parse(io& f, int i);
			virtual void add(sentence& s);
			virtual void remove(sentence& s);
			virtual void estimate(int iter);
			virtual void poisson_correction(int n = 3000);
			virtual void set(int v, int k);
			virtual int n();
			virtual int m();
			virtual int k();
			virtual init(sentence& s);
			virtual void slice(double a, double b);
			virtual void save(const char *file);
			virtual void load(const char *file);
		protected:
			int _n;
			int _m;
			int _v;
			int _k;
			int _K;
			double _a;
			double _b;
			std::shared_ptr<hpyp> _pos;
			std::shared_ptr<std::vector<std::shared_ptr<hpyp> > > _word;
			std::shared_ptr<std::vector<std::shared_ptr<vpyp> > > _letter;
			std::mutex _mutex;
		private:
	};
}

#endif
