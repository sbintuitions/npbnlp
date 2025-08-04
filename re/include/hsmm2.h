#ifndef NPBNLP_HSMM2_H
#define NPBNLP_HSMM2_H

#include"io.h"
#include"da2.h"
#include"ylattice2.h"
#include"vtable.h"
#include"hpyp.h"
#include"vpyp.h"
#include<memory>

namespace npbnlp {
	class hmm {
		class lattice {
			public:
				lattice(int k, std::vector<std::pair<word, std::vector<unsigned int> > >& s, std::vector<std::shared_ptr<hpyp> >& word, std::vector<std::shared_ptr<hpyp> >& phonetic, hpyp& pos);
				virtual ~lattice();
				word* wp(int i);
				std::vector<unsigned int>* rp(int i);
				int size(int i);
				std::vector<int>::iterator begin(int i);
				std::vector<int>::iterator end(int i);
				std::vector<std::vector<int> > k;
			protected:
				std::vector<std::pair<word, std::vector<unsigned int> > > *_sequence;
				double lpr(word& w, std::vector<unsigned int>& r, hpyp& wd, hpyp& phn);


		};
		public:
			hmm(int n, int m, int k);
			virtual ~hmm();
			void sample(std::vector<std::pair<word, std::vector<unsigned int> > >& s);
			void add(std::vector<std::pair<word, std::vector<unsigned int> > >& s);
			void remove(std::vector<std::pair<word, std::vector<unsigned int> > >& s);
		protected:
			int _n;
			int _m;
			int _k;
			std::shared_ptr<std::vector<std::shared_ptr<hpyp> > > _word;
			std::shared_ptr<std::vector<std::shared_ptr<vpyp> > > _letter;
			std::shared_ptr<std::vector<std::shared_ptr<hpyp> > > _phonetic;
			std::shared_ptr<hpyp> _pos;
			friend class hsmm;
	};

	class hsmm {
		public:
			static void uint_writer(unsigned int& c, FILE *fp) {
				fwrite(&c, sizeof(unsigned int), 1, fp);
			}
			static void uint_reader(unsigned int& c, FILE *fp) {
				fread(&c, sizeof(unsigned int), 1, fp);
			}
			static void vv_writer(std::vector<std::vector<unsigned int> >& v, FILE *fp) {
				size_t size = v.size();
				fwrite(&size, sizeof(size_t), 1, fp);
				for (auto& i : v) {
					size_t s = i.size();
					fwrite(&s, sizeof(size_t), 1, fp);
					fwrite(&i[0], sizeof(unsigned int), s, fp);
				}
			}
			static void vv_reader(std::vector<std::vector<unsigned int> >& v, FILE *fp) {
				size_t size = 0;
				fread(&size, sizeof(size_t), 1, fp);
				v.resize(size);
				for (auto i = 0; i < (int)size; ++i) {
					size_t s = 0;
					fread(&s, sizeof(size_t), 1, fp);
					v[i].resize(s);
					fread(&v[i][0], sizeof(unsigned int), s, fp);
				}
			}
			hsmm(const char *dic, const char *unit = NULL);
			hsmm(int n, int m, int k, const char *dic, const char *unit = NULL);
			virtual ~hsmm();
			virtual void add(std::vector<std::pair<word, std::vector<unsigned int> > >& seq);
			virtual void remove(std::vector<std::pair<word, std::vector<unsigned int> > >& seq);
			virtual void init(io& f, std::vector<std::vector<std::pair<word, std::vector<unsigned int> > > >& corpus);
			virtual void pretrain(int iter, std::vector<std::vector<std::pair<word, std::vector<unsigned int> > > >& corpus);
			virtual std::vector<std::pair<word, std::vector<unsigned int> > > sample(io& f, int i);
			virtual std::vector<std::pair<word, std::vector<unsigned int> > > parse(io& f, int i);
			virtual void set(int v);
			virtual int n();
			virtual int m();
			virtual void estimate(int iter);
			virtual void poisson_correction(int n = 3000);
			virtual void save(const char *file);
			virtual void load(const char *file);
		protected:
			using trie = da<unsigned int, std::vector<std::vector<unsigned int> > >;
			int _n;
			int _m;
			int _v;
			int _k;
			std::shared_ptr<std::vector<std::shared_ptr<hpyp> > > _word;
			std::shared_ptr<std::vector<std::shared_ptr<vpyp> > > _letter;
			std::shared_ptr<std::vector<std::shared_ptr<hpyp> > > _phonetic;
			std::shared_ptr<hpyp> _pos;
			std::shared_ptr<trie> _dic;
			std::shared_ptr<trie> _unit;
			double _emission(ynode& n, int k);
			double _transition(int p, int s);
			//void _precalc(ynode& n);
			//double _transition(ynode& prev, ynode& cur);
	};
}

#endif
