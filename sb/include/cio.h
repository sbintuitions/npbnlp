#ifndef NPBNLP_CHUNK_IO
#define NPBNLP_CHUNK_IO

#include"io.h"
namespace npbnlp {
	class cio {
		public:
			cio(const char *f);
			cio(std::istream& in);
			virtual ~cio();
			std::shared_ptr<std::vector<io> > chunk;
	};
}

#endif
