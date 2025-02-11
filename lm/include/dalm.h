#ifndef NPBNLP_DA_LM_H
#define NPBNLP_DA_LM_H

#include"word.h"
#include"sentence.h"

namespace npbnlp {
	class dalm {
		public:
			virtual double lp(word& w) = 0;
			virtual double lp(sentence& s) = 0;
			virtual double lp(word& w, int i) = 0;
			virtual double lp(sentence& s, int i) = 0;
			virtual bool add(word& w) = 0;
			virtual bool add(sentence& s) = 0;
			virtual bool remove(word& w) = 0;
			virtual bool remove(sentence& s) = 0;
			virtual int save(const char *file) = 0;
			virtual int load(const char *file) = 0;
	};
}

#endif
