#ifndef NPBNLP_HSMM_H
#define NPBNLP_HSMM_H

#include"io.h"
#include"da2.h"
#include"ylattice.h"
#include"vtable.h"
#include"hpyp.h"
#include"vpyp.h"
#include<memory>

namespace npbnlp {
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
			hsmm(const char *dic);
			hsmm(int n, int m, const char *dic);
			virtual ~hsmm();
			virtual void add(std::vector<std::pair<word, std::vector<unsigned int> > >& seq);
			virtual void remove(std::vector<std::pair<word, std::vector<unsigned int> > >& seq);
			virtual void init(io& f, std::vector<std::vector<std::pair<word, std::vector<unsigned int> > > >& corpus);
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
			std::shared_ptr<hpyp> _word;
			std::shared_ptr<vpyp> _letter;
			std::shared_ptr<hpyp> _phonetic;
			std::shared_ptr<trie> _dic;
			void _precalc(ynode& n);
			double _transition(ynode& prev, ynode& cur);
	};
}

#endif
