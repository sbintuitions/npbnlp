#ifndef NPBNLP_DA_VPYP_H
#define NPBNLP_DA_VPYP_H

#include"hpy.h"

namespace npbnlp {
	class vpy : public hpy {
		public:
			vpy();
			vpy(int n);
			vpy(const vpy& lm);
			vpy& operator=(const vpy& lm);
			virtual ~vpy();
			double lp(word& w, int i);
			double lp(sentence& s, int i);
			double lp(word& w);
			double lp(sentence& s);
			bool add(word& w);
			bool remove(word& w);
			bool add(sentence& s);
			bool remove(sentence& s);

		protected:
			int _draw_n(word& w, int i);
			int _draw_n(sentence& s, int i);
			bool _add(word& w, int i, int n);
			bool _add(sentence& s, int i, int n);
			bool _remove(word& w, int i, int n);
			bool _remove(sentence& s, int i, int n);
			double _lp(word& w, int i, int n);
			double _lp(sentence& s, int i, int n);
	};
}

#endif
