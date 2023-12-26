#ifndef NPBNLP_SEED_H
#define NPBNLP_SEED_H

#include<random>
#include<memory>
#include<mutex>
namespace npbnlp {
	class seed {
		public:
			static std::shared_ptr<seed> create();
			seed(seed&&) = delete;
			seed(const seed&) = delete;
			seed& operator=(const seed&) = delete;
			seed& operator=(seed&&) = delete;
			unsigned int operator()();
			virtual ~seed();
		private:
			seed();
			static std::shared_ptr<seed> _sd;
			static std::mutex _mutex;
			std::random_device _seed;

	};
}

#endif
