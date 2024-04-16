#ifndef NPBNLP_CONTEXT_H
#define NPBNLP_CONTEXT_H
#include"lm.h"
#include<memory>
#include<unordered_map>
#include<mutex>
#include<vector>
namespace npbnlp {
	using children = std::unordered_map<int, std::shared_ptr<context> >;
	using arrangement = std::unordered_map<int, std::shared_ptr<std::vector<int> > >;
	class context {
		public:
			context();
			context(int k, context *h);
			virtual ~context();
			bool add(int k, lm *m);
			bool remove(int k);
			context* parent() const;
			context* find(int k) const;
			context* make(int k);
			void cleanup();
			int n() const;
			int a() const;
			int b() const;
			int c() const;
			int cu(int k) const;
			int t() const;
			int tu(int k) const;
			int v() const; // restaurant size
			int stop() const;
			int pass() const;
			int sample(lm *m);
			void incr_stop();
			void incr_pass();
			void decr_stop();
			void decr_pass();
			void set(int a, int b);
			void estimate_d(std::vector<double>& a, std::vector<double>& b, lm *m);
			void estimate_t(std::vector<double>& a, std::vector<double>& b, lm *m);
			void estimate_l(std::vector<double>& a, std::vector<double>& b, std::unordered_map<int, std::vector<word> > *corpus);
			void save(FILE *file);
			void load(FILE *file);
		protected:
			int _k;
			int _n;
			int _customer;
			int _table;
			int _a;
			int _b;
			int _stop;
			int _pass;
			context *_parent;
			std::shared_ptr<children> _child;
			std::shared_ptr<arrangement> _restaurant;
			std::mutex _mutex;
			bool _crp_add(int k, lm *m);
			bool _crp_remove(int k);
			std::shared_ptr<std::vector<int> > _get_restaurant(int k);
	};
}

#endif
