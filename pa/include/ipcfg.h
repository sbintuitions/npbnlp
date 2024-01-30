#ifndef NPBNLP_IPCFG_H
#define NPBNLP_IPCFG_H

#include"io.h"
#include"tree.h"
#include"hpyp.h"
#include"vpyp.h"
#include"vtable.h"
#include<functional>
#include<memory>

namespace npbnlp {
	class ipcfg {
		public:
			ipcfg();
			virtual ~ipcfg();
			virtual tree sample(io& f, int i);
			virtual tree parse(io& f, int i);
			virtual void add(tree& t);
			virtual void remove(tree& t);
			virtual void estimate(int iter);
			virtual void poisson_correction(int n = 3000);
			virtual void set(int v, int k);
			virtual void slice(double a, double b);
		private:
			int _m;
			int _k;
			int _K;
			int _v;
			double _a;
			double _b;
			std::shared_ptr<hpyp> _nonterm;
			std::shared_ptr<hpyp> _word;
			std::shared_ptr<vpyp> _letters;
			std::mutex _mutex;

	};
}
#endif
