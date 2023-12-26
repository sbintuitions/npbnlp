#ifndef NPBNLP_LATTICE_H
#define NPBNLP_LATTICE_H

#include"io.h"
#include"chartype.h"
#include"word.h"
#include<vector>
#include<set>
namespace npbnlp {
	enum segsize {
		S_MISC = 5,
		S_ARABIC = 13,
		S_LATIN = 13,
		S_THAI = 6,
		S_GREEK = 13,
		S_MYANMAR = 13,
		S_HEBREW = 13,
		S_DIGIT = 10,
		S_HANGUL = 5,
		S_HIRAGANA = 5,
		S_KATAKANA = 21,
		S_HIRA_KATA = 7,
		S_HIRA_HANJI = 5,
		S_KATA_HANJI = 10,
		S_HIRA_KATA_HANJI = 7,
		S_HANJI = 6,
		S_KATA_OR_HIRA = 7,
		S_SYNBOL = 10
	};
	class lattice {
		public:
			lattice(io& f, int i);
			virtual ~lattice();
			word& wd(int i, int len);
			word* wp(int i, int len);
			void slice(int i, double u);
			double u(int i);
			int size(int i); 
			//bool skip(int i, int j);
			std::vector<int>::iterator sbegin(int i, int j);
			std::vector<int>::iterator send(int i, int j);
			std::vector<std::vector<word> > w;
			std::vector<double> mu;
			std::vector<std::vector<std::vector<int> > > k;
			//std::vector<std::vector<int> > check;
		protected:
			int _segsize(type& t, type& u);
	};

}

#endif
