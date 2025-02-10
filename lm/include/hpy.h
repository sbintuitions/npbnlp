#ifndef NPBNLP_DA_HPYP_H
#define NPBNLP_DA_HPYP_H

#include"dalm.h"
#include"sda.h"
#include<memory>
namespace npbnlp {
	class arrangement {
		public:
			static void key_writer(unsigned int& c, FILE *fp) {
				fwrite(&c, sizeof(unsigned int), 1, fp);
			}
			static void key_reader(unsigned int& c, FILE *fp) {
				fread(&c, sizeof(unsigned int), 1, fp);
			}
			static void val_writer(arrangement& a, FILE *fp) {
				fwrite(&a.n, sizeof(int), 1, fp);
				fwrite(&a.customer, sizeof(long), 1, fp);
				int size = a.table->size();
				fwrite(&size, sizeof(int), 1, fp);
				fwrite(a.table->data(), sizeof(int), size, fp);
			}
			static void val_reader(arrangement& a, FILE *fp) {
				fread(&a.n, sizeof(int), 1, fp);
				fread(&a.customer, sizeof(long), 1, fp);
				int size = 0;
				fread(&size, sizeof(int), 1, fp);
				a.table->resize(size);
				fread(&(*a.table)[0], sizeof(int), size, fp);
			}
			arrangement();
			arrangement(int n);
			virtual ~arrangement();
			arrangement(const arrangement& a);
			arrangement& operator=(const arrangement& a);
			int n;
			long customer;
			std::shared_ptr<std::vector<int> > table;
	};
	class restaurant {
		public:
			static void key_writer(unsigned int& c, FILE *fp) {
				fwrite(&c, sizeof(unsigned int), 1, fp);
			}
			static void key_reader(unsigned int& c, FILE *fp) {
				fread(&c, sizeof(unsigned int), 1, fp);
			}
			static void val_writer(restaurant& r, FILE *fp) {
				fwrite(&r.n, sizeof(int), 1, fp);
				fwrite(&r.table, sizeof(int), 1, fp);
				fwrite(&r.customer, sizeof(long), 1, fp);
			}
			static void val_reader(restaurant& r, FILE *fp) {
				fread(&r.n, sizeof(int), 1, fp);
				fread(&r.table, sizeof(int), 1, fp);
				fread(&r.customer, sizeof(long), 1, fp);
			}
			restaurant();
			restaurant(int n);
			virtual ~restaurant();
			restaurant(const restaurant& r);
			restaurant& operator=(const restaurant& r);
			int n;
			int table;
			long customer;
	};
	class hpy : public dalm {
		public:
			hpy();
			hpy(int n);
			hpy(const hpy& lm);
			hpy& operator=(const hpy& lm);
			virtual ~hpy();
			double lp(word& w, int i);
			double lp(sentence& s, int i);
			double lp(word& w);
			double lp(sentence& s);
			void set_base(hpy *b);
			bool add(word& w);
			bool remove(word& w);
			bool add(sentence& s);
			bool remove(sentence& s);
			void estimate(int iter);
			int save(const char *file);
			int load(const char *file);
		protected:
			int _n;
			int _v;
			hpy *_base;
			std::shared_ptr<std::vector<double> > _discount;
			std::shared_ptr<std::vector<double> > _strength;
			std::shared_ptr<sda<arrangement> > _nc;
			std::shared_ptr<sda<restaurant> > _nz;
			bool _add(word& w, int i, int n);
			bool _remove(word& w, int i, int n);
			bool _add(sentence& s, int i, int n);
			bool _remove(sentence& s, int i, int n);
			double _lp(sentence& s, int i, int n);
			double _lp(word& w, int i, int n);
			void _estimate_d(std::vector<double>& a, std::vector<double>& b);
			void _estimate_t(std::vector<double>& a, std::vector<double>& b);
	};
}
#endif
