#ifndef NPBNLP_USBD_H
#define NPBNLP_USBD_H

#include"io.h"
#include"cio.h"
#include<memory>
namespace npbnlp {
	enum class sequence_type {
		letter,
		word
	};
	enum class smoothing {
		kn,
		hpy
	};

	class usbd {
		public:
			//usbd(int n, smoothing s) = 0;
			virtual void init(cio& f, int iter) = 0;
			virtual void pretrain(io& f, int iter) = 0;
			virtual void set_punc(unsigned int c) = 0;
			virtual void set_general_prior(double general) = 0;
			virtual void set_cr_prior(double cr) = 0;
			virtual void set_punc_prior(double punc) = 0;
			virtual void set_prior(double general, double cr, double punc) = 0;
			virtual void set_hyper_general(double a, double b) = 0;
			virtual void set_hyper_cr(double c, double d) = 0;
			virtual void set_hyper_punc(double e, double f) = 0;
			virtual void add(io& f, std::vector<int>& head) = 0;
			virtual void remove(io& f, std::vector<int>& head) = 0;
			virtual void estimate_lm_hyper(int iter) = 0;
			virtual void estimate_prior(cio& c, std::vector<std::vector<int> >& boundaries) = 0;
			virtual void save(const char *file) = 0;
			virtual void load(const char *file) = 0;
			virtual void sample(io& f, std::vector<int>& b) = 0;
			virtual void parse(io& f, std::vector<int>& b) = 0;
			virtual void eval(cio& target, cio& correct, std::vector<double>& c) = 0;
	};

	class bd_wrap {
		public:
			bd_wrap();
			virtual ~bd_wrap();
			bool create(int n = 3, sequence_type type = sequence_type::letter, smoothing lm = smoothing::hpy);
			virtual void init(cio& f, int iter = 20);
			virtual void pretrain(io& f, int iter = 20);
			virtual void set_punc(unsigned int c);
			virtual void set_prior(double general, double cr, double punc);
			virtual void set_general_prior(double p);
			virtual void set_cr_prior(double p);
			virtual void set_punc_prior(double p);
			virtual void set_hyper_general(double a, double b);
			virtual void set_hyper_cr(double c, double d);
			virtual void set_hyper_punc(double e, double f);
			virtual void add(io& f, std::vector<int>& head);
			virtual void remove(io& f, std::vector<int>& head);
			virtual void estimate_lm_hyper(int iter = 20);
			virtual void estimate_prior(cio& c, std::vector<std::vector<int> >& boundaries);
			virtual void sample(io& f, std::vector<int>& b);
			virtual void parse(io& f, std::vector<int>& b);
			virtual void eval(cio& target, cio& correct, std::vector<double>& c);
			virtual void save(const char *file);
			virtual void load(const char *file);
		protected:
			std::shared_ptr<usbd> _d; // detector
	};
}
#endif
