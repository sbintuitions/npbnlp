#ifndef NPBNLP_KNLM_H
#define NPBNLP_KNLM_H

#include"dalm.h"
#include"da2.h"
#include"sda.h"
#include<memory>

namespace npbnlp {
	//class count : public da<unsigned int, int> {
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
			//count(unsigned int terminal):da<unsigned int,int>(terminal) {
			count(unsigned int terminal):sda<int>(terminal) {
			}
			virtual ~count() {
			}
			/*
			da::rst cs_search(sentence& s, int i, int n) {
				rst r;
				long b = 0;
				int len = 0;
				for (auto j = i; j > i-n; --j) {
					long c = _base[b] + 1;
					if (_check[c] == b && _base[c] < 0)
						r.emplace_back(std::make_pair(len, -_base[c]));
					b = _traverse(b, s[j]);
					++len;
					if (b < 0)
						break;
				}
				return r;
			}
			da::rst cs_search(word& w, int i, int n) {
				rst r;
				long b = 0;
				int len = 0;
				for (auto j = i; j > i-n; --j) {
					long c = _base[b] + 1;
					if (_check[c] == b && _base[c] < 0)
						r.emplace_back(std::make_pair(len, -_base[c]));
					b = _traverse(b, w[j]);
					++len;
					if (b < 0)
						break;
				}
				return r;
			}
			long rexactmatch(sentence& s, int i, int n) {
				long b = 0;
				for (auto j = i; j > i-n; --j) {
					b = _traverse(b, s[j]);
					if (b < 0)
						return -1;
				}
				b = _traverse(b, 3); // terminal
				if (b >= 0 && _base[b] < 0)
					return -_base[b];
				return -1;
			}
			long rexactmatch(word& w, int i, int n) {
				long b = 0;
				for (auto j = i; j > i-n; --j) {
					b = _traverse(b, w[j]);
					if (b < 0)
						return -1;
				}
				b = _traverse(b, 3); // terminal
				if (b >= 0 && _base[b] < 0)
					return -_base[b];
				return -1;
			}
			std::optional<int> getval(long id) {
				if (id >= (long)_value.size() || id < 0 || std::find(_erased.begin(),_erased.end(),id) != _erased.end())
					return std::nullopt;
				return _value[id];
			}
			*/
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
			/*
			void insert_subtree(node<unsigned int>& t, long b) {
				for (auto& s : t.sibling) {
					if (_add(b, s.id))
						_modify(b, s.id);
				}
				for (auto& s : t.sibling) {
					long n = _base[b] + _c[s.id];
					insert_subtree(s, n);
				}
			}
			void make_subtree(node<unsigned int>& t, sentence& s, int i, int n) {
				node<unsigned int> *p = &t;
				node<unsigned int> terminal(3); terminal.parent = p;
				p->sibling.emplace_back(terminal);
				for (auto j = i; j > i-n; --j) {
					node<unsigned int> n(s[j]); n.parent = p;
					p->sibling.emplace_back(n);
					auto next = p->sibling.size()-1;
					node<unsigned int> m(3); m.parent = &p->sibling[next];
					p->sibling[next].sibling.emplace_back(m);
					p = & p->sibling[next];
				}
			}
			void make_subtree(node<unsigned int>& t, word& w, int i, int n) {
				node<unsigned int> *p = &t;
				node<unsigned int> terminal(3); terminal.parent = p;
				p->sibling.emplace_back(terminal);
				for (auto j = i; j > i-n; --j) {
					node<unsigned int> n(w[j]); n.parent = p;
					p->sibling.emplace_back(n);
					auto next = p->sibling.size()-1;
					node<unsigned int> m(3); m.parent = &p->sibling[next];
					p->sibling[next].sibling.emplace_back(m);
					p = & p->sibling[next];
				}
			}
			*/
	};
	class kn {
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
			bool add(word& w, int i, int n);
			bool remove(word& w);
			bool remove(word& w, int i, int n);
			bool add(sentence& s);
			bool add(sentence& s, int i, int n);
			bool remove(sentence& s);
			bool remove(sentence& s, int i, int n);
			void set_discount(double d);
			int save(const char *file);
			int load(const char *file);
		protected:
			int _n;
			double _d;
			std::shared_ptr<count> _nc;
			std::shared_ptr<count> _nz;
			std::shared_ptr<count> _nk;
	};
}

#endif
