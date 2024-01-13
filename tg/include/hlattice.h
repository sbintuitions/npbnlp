#ifndef NPBNLP_HLATTICE_H
#define NPBNLP_HLATTICE_H

#include"io.h"
#include"sentence.h"
#include<vector>
#include<memory>
namespace npbnlp {
	class hlattice {
		public:
			hlattice(io& f, int i);
			virtual ~hlattice();
			word& wd(int i);
			word* wp(int i);
			int size(int i);
			void slice(int i, double u);
			double u(int i);
			std::vector<int>::iterator begin(int i);
			std::vector<int>::iterator end(int i);
			std::vector<double> mu;
			std::vector<std::vector<int> > k;
			sentence s;
		private:
	};
}

#endif
