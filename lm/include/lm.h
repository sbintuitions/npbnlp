#ifndef NPBNLP_LM_H
#define NPBNLP_LM_H

#include"word.h"
#include"chunk.h"
#include"sentence.h"
#include"nsentence.h"
namespace npbnlp {
	class context;
	class lm {
		public:
			virtual context* h() const = 0;
			virtual context* find(word& w, int i) const = 0;
			virtual context* find(chunk& c, int i) const = 0;
			virtual context* find(sentence& s, int i) const = 0;
			virtual context* find(nsentence& s, int i) const = 0;
			virtual double pr(word& w, const context *h) = 0;
			virtual double pr(chunk& c, const context *h) = 0;
			virtual double pr(int k, const context *h) = 0;
			virtual double lp(int k, const context *h) = 0;
			virtual double lp(word& w, const context *h) = 0;
			virtual double lp(chunk& c, const context *h) = 0;
			virtual int n() const = 0;
			virtual void add(word& w, context *h) = 0;
			virtual void add(chunk& c, context *h) = 0;
			virtual void remove(word& w, context *h) = 0;
			virtual void remove(chunk& c, context *h) = 0;
			virtual bool add(int k, context *h) = 0;
			virtual bool remove(int k, context *h) = 0;
			virtual void save(const char *file) = 0;
			virtual void load(const char *file) = 0;
			virtual double alpha(int n) const = 0;
			virtual double discount(int n) const = 0;
			virtual double strength(int n) const = 0;
			virtual int draw_n(nsentence& s, int i) = 0;
			virtual int draw_n(sentence& s, int i) = 0;
			virtual int draw_n(chunk& c, int i) = 0;
			virtual int draw_n(word& w, int i) = 0;
			virtual int draw_k(context *h) = 0;
	};
}
#endif
