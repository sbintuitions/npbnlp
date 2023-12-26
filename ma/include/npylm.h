#ifndef NPBNLP_NPYLM_H
#define NPBNLP_NPYLM_H

#include"io.h"
#include"hpyp.h"
#include"vpyp.h"
#include"lattice.h"
#include"vtable.h"
#include<mutex>
#include<functional>
namespace npbnlp {
	class npylm {
		public:
			npylm();
			npylm(int n, int m);
			virtual ~npylm();
			virtual sentence sample(io& f, int i);
			virtual sentence parse(io& f, int i);
			virtual void set(int v);
			virtual int n();
			virtual int m();
			virtual void add(sentence& s);
			virtual void remove(sentence& s);
			virtual void estimate(int iter);
			virtual void poisson_correction(int n = 3000);
			virtual void save(const char *file);
			virtual void load(const char *file);
		protected:
			int _n;
			std::shared_ptr<hpyp> _word;
			std::shared_ptr<vpyp> _letter;	
			virtual void _forward(lattice& l, int i, const context *c, word& w, word& p, vt& a, vt& b, int n, bool unk);
			virtual void _backward(lattice& l, int i, const context *c, word& w, double& lp, vt& b, int n, bool unk);
			std::mutex _mutex;
	};
}

#endif
