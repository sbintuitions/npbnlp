#ifndef NPBNLP_NNPYLM_H
#define NPBNLP_NNPYLM_H

#include"nio.h"
#include"hpyp.h"
#include"vpyp.h"
#include"clattice.h"
#include"vtable.h"
#include<mutex>
#include<functional>
namespace npbnlp {
	class nnpylm {
		public:
			nnpylm();
			nnpylm(int n, int m, int l);
			virtual ~nnpylm();
			virtual nsentence sample(nio& f, int i);
			virtual nsentence parse(nio& f, int i);
			virtual void add(nsentence& s);
			virtual void remove(nsentence& s);
			virtual void estimate(int iter);
			virtual void poisson_correction(int n = 100);
			virtual void set(int v);
			virtual int n();
			virtual int m();
			virtual int l();
			virtual void save(const char *file);
			virtual void load(const char *file);
		protected:
			int _n;
			std::shared_ptr<hpyp> _chunk;
			std::shared_ptr<hpyp> _word;
			std::shared_ptr<vpyp> _letter;
			void _forward(clattice& l, int i, const context *c, chunk& ch, chunk& p, vt& a, vt& b, int n, bool unk);
			void _backward(clattice& l, int i, const context *c, chunk& ch, double& lp, vt& b, int n, bool unk);
			void _ch_draw(std::vector<int>& c);
			std::mutex _mutex;
		private:

	};
}

#endif
