#ifndef NPBNLP_KNLM_H
#define NPBNLP_KNLM_H

#include"dalm.h"
#include"sda.h"
#include<memory>

namespace npbnlp {
	class count : public sda<int> {
		public:
			static void uint_writer(unsigned int& c, FILE *fp) {
				fwrite(&c, sizeof(unsigned int), 1, fp);
			}
			static void uint_reader(unsigned int& c, FILE *fp) {
				fread(&c, sizeof(unsigned int), 1, fp);
			}
			static void int_writer(int& c, FILE *fp) {
				fwrite(&c, sizeof(int), 1, fp);
			}
			static void int_reader(int& c, FILE *fp) {
				fread(&c, sizeof(int), 1, fp);
			}
			count(unsigned int terminal):sda<int>(terminal) {
			}
			virtual ~count() {
			}
			void insert(sentence& s, int i, int n) {
				node<unsigned int> tree;
				make_subtree(tree, s, i, n);
				insert_subtree(tree, 0);
				if (_value.size() < _nid) {
					_value.resize(_nid, 0);
				}
			}
			void insert(word& w, int i, int n) {
				node<unsigned int> tree;
				make_subtree(tree, w, i, n);
				insert_subtree(tree, 0);
				if (_value.size() < _nid) {
					_value.resize(_nid, 0);
				}
			}
			void incr(sentence& s, int i, int n) {
				long id = rexactmatch(s, i, n);
				if (id < 0) {
					insert(s, i, n);
					id = rexactmatch(s, i, n);
				}
				_value[id]++;
			}
			void incr(word& w, int i, int n) {
				long id = rexactmatch(w, i, n);
				if (id < 0) {
					insert(w, i, n);
					id = rexactmatch(w, i, n);
				}
				_value[id]++;
			}
			void decr(sentence& s, int i, int n) {
				long id = rexactmatch(s, i, n);
				_value[id]--;
				if (_value[id] == 0)
					erase(s, i, n);
			}
			void decr(word& w, int i, int n) {
				long id = rexactmatch(w, i, n);
				_value[id]--;
				if (_value[id] == 0)
					erase(w, i, n);
			}
			void erase(sentence& s, int i, int n) {
				long b = 0;
				for (auto j = i; j > i-n; --j) {
					b = _traverse(b, s[j]);
					if (b < 0)
						return;
				}
				b = _traverse(b, 3); // terminal
				if (b >= 0 && _base[b] < 0 && _check[b] >= 0) {
					_erased.emplace_back(-_base[b]);
					_value[-_base[b]] = 0;
					auto p = _check[b];
					_link(b);
					while (p > 0) {
						std::vector<long> sib;
						_get_sibling(p, sib);
						if (sib.empty()) {
							auto n = _check[p];
							_link(p);
							p = n;
						} else {
							break;
						}
					}
				}
			}
			void erase(word& w, int i, int n) {
				long b = 0;
				for (auto j = i; j > i-n; --j) {
					b = _traverse(b, w[j]);
					if (b < 0)
						return;
				}
				b = _traverse(b, 3); // terminal
				if (b >= 0 && _base[b] < 0 && _check[b] >= 0) {
					_erased.emplace_back(-_base[b]);
					_value[-_base[b]] = 0;
					auto p = _check[b];
					_link(b);
					while (p > 0) {
						std::vector<long> s;
						_get_sibling(p, s);
						if (s.empty()) {
							auto n = _check[p];
							_link(p);
							p = n;
						} else {
							break;
						}
					}
				}
			}
	};
	class kn : public dalm {
		public:
			kn();
			kn(int n);
			kn(const kn& lm);
			kn& operator=(const kn& lm);
			virtual ~kn();
			double lp(word& w, int i);
			double lp(sentence& s, int i);
			double lp(word& w);
			double lp(sentence& s);
			bool add(word& w);
			bool remove(word& w);
			bool add(sentence& s);
			bool remove(sentence& s);
			void estimate(int iter){};
			void set_discount(double d);
			int save(const char *file);
			int load(const char *file);
		protected:
			int _n;
			double _d;
			std::shared_ptr<count> _nc;
			std::shared_ptr<count> _nz;
			std::shared_ptr<count> _nk;
			bool _add(word& w, int i, int n);
			bool _remove(word& w, int i, int n);
			bool _add(sentence& s, int i, int n);
			bool _remove(sentence& s, int i, int n);
	};
}

#endif
