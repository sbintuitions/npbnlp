#ifndef NPBNLP_IPCFG_H
#define NPBNLP_IPCFG_H

#include"io.h"
#include"tree.h"
#include"hpyp.h"
#include"vpyp.h"
#include"vtable.h"
#include"cyk.h"
#include<functional>
#include<memory>

namespace npbnlp {
	class ipcfg {
		public:
			ipcfg();
			ipcfg(int m);
			virtual ~ipcfg();
			virtual tree sample(io& f, int i);
			virtual tree parse(io& f, int i);
			virtual void add(tree& t);
			virtual void remove(tree& t);
			virtual void estimate(int iter);
			virtual void poisson_correction(int n = 100);
			virtual void set(int v, int k);
			virtual void slice(double a, double b);
			virtual void save(const char *file);
			virtual void load(const char *file);
		private:
			int _m;
			int _k;
			int _K;
			int _v;
			double _a;
			double _b;
			std::shared_ptr<hpyp> _nonterm;
			std::shared_ptr<std::vector<std::shared_ptr<hpyp> > > _word;
			std::shared_ptr<std::vector<std::shared_ptr<vpyp> > > _letter;
			std::mutex _mutex;
			void _traceback(cyk& c, int i, int j, int z, vt& a, tree& tr, bool best = false);
			void _add(tree& t, int i);
			void _remove(tree& t, int i);
			void _calc_preterm(cyk& c, int j, vt& a);
			void _calc_nonterm(cyk& c, int i, int j, vt& a);
			void _slice(cyk& l);
			void _slice_preterm(cyk& l, int i);
			//void _slice_nonterm(cyk& c, int i, int j);
			double _draw(cyk& c, int i, int j);
			void _slice_nonterm(cyk& c, int i, int j, double mu);
			void _slice_root(cyk& c);
			void _resize();
			void _shrink();
	};
}
#endif
