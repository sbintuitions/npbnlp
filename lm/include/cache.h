#ifndef NPBNLP_LM_CACHE_H
#define NPBNLP_LM_CACHE_H

#include"chunk.h"
#include"word.h"
#include"lm.h"
#ifdef _OPENMP
#include<omp.h>
#endif
#include<mutex>
namespace npbnlp {
	struct contexthash {
		size_t operator() (const context *c) const {
			size_t i = *(size_t*)c;
			return std::hash<size_t>()(i);
		}
	};
	struct contexteq {
		bool operator() (const context *a, const context *b) const {
			return (a == b);
		}
	};
	class cache {
		public:
			cache();
			virtual ~cache();
			double get(chunk& b, const context *c, bool& chk);
			double get(word& w, const context *c, bool& chk);
			double get(int k, const context *c, bool& chk);
			double set(chunk& b, const context *c, double lp);
			double set(word& w, const context *c, double lp);
			double set(int k, const context *c, double lp);
			void clear();
		protected:
#ifdef _OPENMP
			using lm_cache = std::vector<std::unordered_map<const context*, std::unordered_map<int, double>, contexthash, contexteq> >;
#else
			using lm_cache = std::unordered_map<const context*, std::unordered_map<int, double>, contexthash, contexteq>;
#endif
			lm_cache _c;
	};
}

#endif
