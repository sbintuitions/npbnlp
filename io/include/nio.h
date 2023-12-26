#ifndef NPBNLP_NIO_H
#define NPBNLP_NIO_H

#include"sentence.h"
namespace npbnlp {
	class nio {
		public:
			nio(std::vector<sentence>& s);
			virtual ~nio();
			std::shared_ptr<std::vector<word> > raw;
			std::vector<int> head;
		private:
	};
}

#endif
