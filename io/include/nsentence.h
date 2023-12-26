#ifndef NPBNLP_NSENTENCE_H
#define NPBNLP_NSENTENCE_H

#include"chunk.h"
namespace npbnlp {
	class nsentence {
		public:
			nsentence();
			nsentence(const nsentence& s);
			//nsentence(std::vector<word>& d, int head, int tail);
			nsentence& operator=(const nsentence& s);
			nsentence(nsentence&& s) = default;
			nsentence& operator=(nsentence&& s) noexcept;
			virtual ~nsentence();
			int operator[](int i);
			int size();
			chunk& ch(int i);
			std::vector<chunk> c;
			std::vector<int> n;
	};
}
#endif
