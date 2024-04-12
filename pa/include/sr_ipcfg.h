#ifndef NPBNLP_SR_IPCFG_H
#define NPBNLP_SR_IPCFG_H

#include"io.h"
#include"tree.h"
#include"cyk.h"
#include"hpyp.h"
#include"vpyp.h"
#include"vtable.h"
#include<functional>
#include<memory>

namespace npbnlp {
	class spcfg {
		public:
			spcfg();
			spcfg(int m);
			virtual ~spcfg();
			virtual tree sample(io& f, int i);
			virtual tree parse(io& f, int i);
			virtual void add(tree& t);
			virtual void remove(tree& t);
			virtual void estimate(int iter);
			virtual void poisson_correction(int n = 100);
			virtual void set(int v, int k);
			virtual void anneal(double a);
			virtual void slice(double a, double b);
			virtual void save(const char *file);
			virtual void load(const char *file);
		private:
			using cell = std::pair<int, int>;
			int _m;
			int _k;
			int _K;
			int _v;
			double _a;
			double _b;
			double _annl;
			std::shared_ptr<hpyp> _nonterm;
			std::shared_ptr<std::vector<std::shared_ptr<hpyp> > > _word;
			std::shared_ptr<std::vector<std::shared_ptr<vpyp> > > _letter;
			std::mutex _mutex;
			void _add(tree& t, int i);
			void _remove(tree& t, int i);
			void _init(cyk& l, bool best = false);
			void _sample_preterm(cyk& l, int i, bool best = false);
			bool _sample_tree(cyk& l, bool best = false);
			int _sample_nonterm(cyk& l, cell& left, cell& right, bool best = false);
			void _traceback(cyk& c, int i, int j, int z, tree& tr);

			void _resize();
			void _shrink();
	};
}

#endif
