#ifndef NPBNLP_HDP_H
#define NPBNLP_HDP_H

#include"cache.h"
#include"word.h"
#include"sentence.h"
#include"lm.h"
#include"hdp_context.h"
#include<memory>
#include<mutex>

namespace npbnlp {
	class hdp : public lm {
		public:
			hdp();
			hdp(int n, double a=1, double b=1);
			virtual ~hdp();
			double pr(chunk& c, const context *h);
			double pr(word& w, const context *h);
			double pr(int k, const context *h);
			double lp(chunk& c, const context *h);
			double lp(word& w, const context *h);
			double lp(int k, const context *h);
			double alpha(int n) const;
			double discount(int n) const;
			double strength(int n) const;
			bool add(int k, context *h);
			bool remove(int k, context *h);
			void add(word& w, context *h);
			void remove(word& w, context *h);
			void add(chunk& c, context *h);
			void remove(chunk& c, context *h);
			void set_base(lm *m);
			void set_v(int v);
			int v() const;
			int n() const;
			void estimate(int iter);
			context* h() const;
			context* find(nsentence& s, int i) const;
			context* find(sentence& s, int i) const;
			context* find(chunk& c, int i) const;
			context* find(word& w, int i) const;
			context* make(nsentence& s, int i);
			context* make(sentence& s, int i);
			context* make(chunk& c, int i);
			context* make(word& w, int i);
			void save(const char *file);
			void load(const char *file);
			int draw_n(nsentence& s, int i);
			int draw_n(sentence& s, int i);
			int draw_n(chunk& c, int i);
			int draw_n(word& w, int i);
			int draw_k(context *h);
		private:
			using base_corpus = std::unordered_map<int, std::vector<word> >;
			using cbase_corpus = std::unordered_map<int, std::vector<chunk> >;
			//using cache = std::unordered_map<const context*, std::unordered_map<int, double>, contexthash, contexteq>;
			int _n;
			double _a;
			double _b;
			lm *_base;
			int _v;
			std::shared_ptr<hdp_context> _h;
			std::shared_ptr<std::vector<double> > _alpha;
			std::shared_ptr<base_corpus> _bc;
			std::shared_ptr<cbase_corpus> _cbc;
			std::mutex _mutex;
			double _prb(word& w) const;
			double _prb(chunk& c) const;
			double _lpb(word& w) const;
			double _lpb(chunk& c) const;
			/*
			double _find_cache(int k, const context *c);
			double _find_cache(word& w, const context *c);
			double _set_cache(int k, const context *c, double lpr);
			double _set_cache(word& w, const context *c, double lpr);
			*/
			cache _cache;

	};
}

#endif
