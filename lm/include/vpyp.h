#ifndef NPBNLP_VPYP_H
#define NPBNLP_VPYP_H

#include"hpyp.h"
namespace npbnlp {
	class vpyp : public hpyp {
		public:
			vpyp();
			vpyp(int n, double a=1, double b=1);
			virtual ~vpyp();
			double pr(int k, const context *h);
			double pr(word& w, const context *h);
			double pr(chunk& c, const context *h);
			double lp(int k, const context *h);
			double lp(word& w, const context *h);
			double lp(chunk& c, const context *h);
		private:
			double _prb(word& w) const;
			double _prb(chunk& c) const;
			double _lpb(word& w) const;
			double _lpb(chunk& c) const;
	};
}

#endif
