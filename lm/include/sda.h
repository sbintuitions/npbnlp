#ifndef NPBNLP_SUFFIX_DA_H
#define NPBNLP_SUFFIX_DA_H

#include"da2.h"

namespace npbnlp {
	template<class V>
		class sda : public da<unsigned int, V> {
			public:
				using rst = typename da<unsigned int, V>::rst;
				sda(unsigned int terminal):da<unsigned int, V>(terminal) {
				}
				virtual ~sda() {
				}
				rst cs_search(sentence& s, int i, int n) {
					rst r;
					long b = 0;
					int len = 0;
					for (auto j = i; j > i-n; --j) {
						long c = da<unsigned int,V>::_base[b] + 1;
						if (da<unsigned int,V>::_check[c] == b && da<unsigned int,V>::_base[c] < 0)
							r.emplace_back(std::make_pair(len, -da<unsigned int,V>::_base[c]));
						b = da<unsigned int,V>::_traverse(b, s[j]);
						++len;
						if (b < 0)
							break;
					}
					return r;
				}
				rst cs_search(word& w, int i, int n) {
					rst r;
					long b = 0;
					int len = 0;
					for (auto j = i; j > i-n; --j) {
						long c = da<unsigned int,V>::_base[b] + 1;
						if (da<unsigned int,V>::_check[c] == b && da<unsigned int,V>::_base[c] < 0)
							r.emplace_back(std::make_pair(len, -da<unsigned int,V>::_base[c]));
						b = da<unsigned int,V>::_traverse(b, w[j]);
						++len;
						if (b < 0)
							break;
					}
					return r;
				}
				long rexactmatch(sentence& s, int i, int n) {
					long b = 0;
					for (auto j = i; j > i-n; --j) {
						b = da<unsigned int,V>::_traverse(b, s[j]);
						if (b < 0)
							return -1;
					}
					b = da<unsigned int,V>::_traverse(b, 3); // terminal code
					if (b >= 0 && da<unsigned int,V>::_base[b] < 0)
						return -da<unsigned int,V>::_base[b];
					return -1;
				}
				long rexactmatch(word& w, int i, int n) {
					long b = 0;
					for (auto j = i; j > i-n; --j) {
						b = da<unsigned int,V>::_traverse(b, w[j]);
						if (b < 0)
							return -1;
					}
					b = da<unsigned int,V>::_traverse(b, 3);
					if (b >= 0 && da<unsigned int,V>::_base[b] < 0)
						return -da<unsigned int,V>::_base[b];
					return -1;
				}
				std::optional<V> getval(long id) {
					if (id >= (long)da<unsigned int,V>::_value.size() || id < 0 || std::find(da<unsigned int,V>::_erased.begin(), da<unsigned int,V>::_erased.end(), id) != da<unsigned int,V>::_erased.end())
						return std::nullopt;
					return da<unsigned int,V>::_value[id];
				}
				/*
				V& getval(long id) {
					if (id >= (long)_value.size() || id < 0 || std::find(_erased.begin(), _erased.end(), id) != _erased.end())
						throw "found invalid id";
					return _value[id];
				}
				*/
				void insert(sentence& s, int i, int n) {
					node<unsigned int> tree;
					make_subtree(tree, s, i, n);
					insert_subtree(tree, 0);
					if (da<unsigned int,V>::_value.size() < da<unsigned int,V>::_nid) {
						da<unsigned int,V>::_value.resize(da<unsigned int,V>::_nid, V(n));
					}
				}
				void insert(word& w, int i, int n) {
					node<unsigned int> tree;
					make_subtree(tree, w, i, n);
					insert_subtree(tree, 0);
					if (da<unsigned int,V>::_value.size() < da<unsigned int,V>::_nid) {
						da<unsigned int,V>::_value.resize(da<unsigned int,V>::_nid, V(n));
					}
				}
				void erase(sentence& s, int i, int n) {
					long b = 0;
					for (auto j = i; j > i-n; --j) {
						b = da<unsigned int,V>::_traverse(b, s[j]);
						if (b < 0)
							return;
					}
					b = da<unsigned int,V>::_traverse(b, 3);
					if (b >= 0 && da<unsigned int,V>::_base[b] < 0 && da<unsigned int,V>::_check[b] >= 0) {
						da<unsigned int,V>::_erased.emplace_back(-da<unsigned int,V>::_base[b]);
						da<unsigned int,V>::_value[-da<unsigned int,V>::_base[b]] = V(n);
						auto p = da<unsigned int,V>::_check[b];
						da<unsigned int,V>::_link(b);
						while (p > 0) {
							std::vector<long> sib;
							da<unsigned int,V>::_get_sibling(p, sib);
							if (sib.empty()) {
								auto n = da<unsigned int,V>::_check[p];
								da<unsigned int,V>::_link(p);
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
						b = da<unsigned int,V>::_traverse(b, w[j]);
						if (b < 0)
							return;
					}
					b = da<unsigned int,V>::_traverse(b, 3);
					if (b >= 0 && da<unsigned int,V>::_base[b] < 0 && da<unsigned int,V>::_check[b] >= 0) {
						da<unsigned int,V>::_erased_emplace_back(-da<unsigned int,V>::_base[b]);
						da<unsigned int,V>::_value[-da<unsigned int,V>::_base[b]] = V(n);
						auto p = da<unsigned int,V>::_check[b];
						da<unsigned int,V>::_link(b);
						while (p > 0) {
							std::vector<long> sib;
							da<unsigned int,V>::_get_sibling(p, sib);
							if (sib.empty()) {
								auto n = da<unsigned int,V>::_check[p];
								da<unsigned int,V>::_link(p);
								p = n;
							} else {
								break;
							}
						}
					}
				}
				void insert_subtree(node<unsigned int>& t, long b) {
					for (auto& s : t.sibling) {
						if (da<unsigned int,V>::_add(b, s.id))
							da<unsigned int,V>::_modify(b, s.id);
					}
					for (auto& s : t.sibling) {
						long n = da<unsigned int,V>::_base[b] + da<unsigned int,V>::_c[s.id];
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
						node<unsigned int> m(3); m.parent= &p->sibling[next];
						p->sibling[next].sibling.emplace_back(m);
						p = &p->sibling[next];
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
						node<unsigned int> m(3); m.parent= &p->sibling[next];
						p->sibling[next].sibling.emplace_back(m);
						p = &p->sibling[next];
					}
				}
		};
}

#endif
