#ifndef NPBNLP_PHSMM_H
#define NPBNLP_PHSMM_H

#include"io.h"
#include"lattice.h"
#include"vtable.h"
#include"hpyp.h"
#include"vpyp.h"
#include<mutex>
namespace npbnlp {
	class phsmm {
		public:
			phsmm();
			phsmm(int n, int m, int l, int k);
			virtual ~phsmm();
			virtual void add(sentence& s);
			virtual void remove(sentence& s);
			virtual void init(sentence& s);
			virtual sentence sample(io& f, int i);
			virtual sentence parse(io& f, int i);
			virtual void set(int v, int k);
			virtual int n();
			virtual int m();
			virtual int l();
			virtual void slice(double a, double b);
			virtual void estimate(int iter);
			virtual void poisson_correction(int n = 3000);
			virtual void save(const char *file);
			virtual void load(const char *file);
		protected:
			int _n;
			int _m;
			int _l;
			int _k;
			int _v;
			int _K;
			double _a;
			double _b;
			std::shared_ptr<hpyp> _pos;
			std::shared_ptr<std::vector<std::shared_ptr<hpyp> > > _word;
			std::shared_ptr<std::vector<std::shared_ptr<vpyp> > > _letter;
			std::mutex _mutex;

			void _forward(lattice& l, int i, const context *c, const context *t, word& w, int p, word& prev, int q, vt& a, vt& b, int n, int m, bool unk, bool not_exist);
			void _backward(lattice& l, int i, const context *c, const context *t, word& w, int p, word& prev, int q, double& lpr, vt& b, int n, int m, bool unk, bool not_exist);
			void _slice(lattice& l);
			void _resize();
			void _shrink();
	};
}

#endif
