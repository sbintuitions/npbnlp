#ifndef NPBNLP_HPYP_H
#define NPBNLP_HPYP_H

#include"cache.h"
#include"nsentence.h"
#include"sentence.h"
#include"chunk.h"
#include"word.h"
#include"lm.h"
#include"context.h"
#include<memory>
#include<mutex>
namespace npbnlp {
	class hpyp : public lm {
		public:
			hpyp();
			hpyp(int n, double a=1, double b=1);
			hpyp(const hpyp& lm);
			hpyp(hpyp&& lm);
			hpyp& operator=(const hpyp& lm);
			hpyp& operator=(const hpyp&& lm) noexcept;
			virtual ~hpyp();
			double pr(chunk& c, const context *h);
			double pr(word& w, const context *h);
			double pr(int k, const context *h);
			double lp(chunk& c, const context *h);
			double lp(word& w, const context *h);
			double lp(int k, const context *h);
			double discount(int n) const;
			double strength(int n) const;
			double alpha(int n) const;
			context* h() const;
			context* find(nsentence& s, int i)const;
			context* find(sentence& s, int i)const;
			context* find(chunk& c, int i) const;
			context* find(word& s, int i) const;
			context* make(nsentence& s, int i);
			context* make(sentence& s, int i);
			context* make(chunk& c, int i);
			context* make(word& s, int i);
			context* find(nsentence& s, int i, int n) const;
			context* find(sentence& s, int i, int n) const;
			context* find(chunk& c, int i, int n) const;
			context* find(word& w, int i, int n) const;
			context* make(nsentence& s, int i, int n);
			context* make(sentence& s, int i, int n);
			context* make(chunk& c, int i, int n);
			context* make(word& w, int i, int n);
			int draw_n(word& w, int i);
			int draw_n(chunk& c, int i);
			int draw_n(sentence& s, int i);
			int draw_n(nsentence& s, int i);
			int draw_k(context *h);
			int n() const;
			int v() const;
			void add(chunk& c, context *h);
			void add(word& w, context *h);
			bool add(int k, context *h);
			void remove(chunk& c, context *h);
			void remove(word& w, context *h);
			bool remove(int k, context *h);
			void set_base(hpyp *b);
			void set_v(int v);
			void estimate(int iter);
			void poisson_correction(int n = 3000);
			void gibbs(int iter);
			void save(const char *file);
			void load(const char *file);
			void save(FILE *fp);
			void load(FILE *fp);
		protected:
			using base_corpus = std::unordered_map<int, std::vector<word> >;
			using cbase_corpus = std::unordered_map<int, std::vector<chunk> >;
			int _n;
			double _a;
			double _b;
			hpyp *_base;
			int _v;
			std::shared_ptr<context> _h;
			std::shared_ptr<std::vector<double> > _discount;
			std::shared_ptr<std::vector<double> > _strength;
			std::shared_ptr<base_corpus> _bc;
			std::shared_ptr<cbase_corpus> _cbc;
			std::shared_ptr<std::vector<double> > _poisson;
			std::shared_ptr<std::vector<double> > _lambda;
			int _f;
			std::shared_ptr<std::vector<double> > _length;
			double _prb(chunk& c) const;
			double _prb(word& w) const;
			double _lpb(chunk& c) const;
			double _lpb(word& w) const;
			double _correct(word& w) const;
			void _estimate_poisson();
			void _estimate_length(int n);
			void _sample(std::vector<unsigned int>& w);
			std::mutex _mutex;
			cache _cache;
	};
}
#endif
