#ifndef NPBNLP_SENTENCE_H
#define NPBNLP_SENTENCE_H

#include"io.h"
#include"word.h"
namespace npbnlp {
	class sentence {
		public:
			sentence();
			sentence(const sentence& s);
			sentence(std::vector<unsigned int>& d, int head, int tail);
			sentence& operator=(const sentence& s);
			sentence(sentence&& s);
			sentence& operator=(sentence&& s) noexcept;
			virtual ~sentence();
			// return word.id
			int operator[](int i);
			int size();
			word& wd(int i);
			std::vector<word> w;
			std::vector<int> n;
		private:
	};
}
#endif
