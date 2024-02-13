#ifndef NPBNLP_CYK_H
#define NPBNLP_CYK_H

#include"sentence.h"
#include"word.h"
#include<vector>
#include<set>
namespace npbnlp {
	class cyk {
		public:
			cyk(io& f, int i);
			virtual ~cyk();
			word& wd(int i);
			word* wp(int i);
			std::set<int>::iterator begin(int i, int j);
			std::set<int>::iterator end(int i, int j);
			int size(int i, int j);
			sentence s;
			std::vector<std::vector<std::set<int> > > k;
			std::vector<std::vector<double> > mu;
	};
}
#endif
