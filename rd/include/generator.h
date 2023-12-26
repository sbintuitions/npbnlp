#ifndef NPBNLP_GENERATOR_H
#define NPBNLP_GENERATOR_H

#include"seed.h"
#include<random>
#include<mutex>
#ifdef _OPENMP
#include<omp.h>
#endif
namespace npbnlp {
	class generator {
		public:
			using gen_t = std::mt19937;
#ifdef _OPENMP
			using sp = std::vector<gen_t>;
#else
			using sp = gen_t;
#endif
			static std::shared_ptr<generator> create();
			generator(const generator&) = delete;
			generator(generator&&) = delete;
			generator& operator=(const generator&) = delete;
			generator& operator=(generator&&) = delete;
			gen_t& operator()();
			virtual ~generator();
		private:
			static std::mutex _mutex;
			generator();
			static std::shared_ptr<generator> _gn;
			std::shared_ptr<sp> _g;
	};
}

#endif
