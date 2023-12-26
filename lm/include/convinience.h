#ifndef NPBNLP_CONVINIENCE_H
#define NPBNLP_CONVINIENCE_H

#include"rd.h"
#include"word.h"
#include"chunk.h"
#include"hpyp.h"
#include"vpyp.h"
#include"hdp.h"
#include<typeindex>
namespace npbnlp {
	class wrap {
		public:
			static void add_h(word& w, hdp *lm) {
				int rd[w.len+1] = {0};
				rd::shuffle(rd, w.len+1);
				for (int i = 0; i < w.len+1; ++i) {
					context *h = lm->make(w, rd[i]);
					lm->add(w[rd[i]], h);
				}
			}
			static void add_h(chunk& c, hdp *lm) {
				int rd[c.len+1] = {0};
				rd::shuffle(rd, c.len+1);
				for (int i = 0; i < c.len+1; ++i) {
					context *h = lm->make(c, rd[i]);
					lm->add(c[rd[i]], h);
				}
			}
			static void add(word& w, hpyp *lm) {
				int rd[w.len+1] = {0};
				rd::shuffle(rd, w.len+1);
				for (int i = 0; i < w.len+1; ++i) {
					context *h = lm->make(w, rd[i]);
					lm->add(w[rd[i]], h);
				}
			}
			static void add(chunk& c, hpyp *lm) {
				int rd[c.len+1] = {0};
				rd::shuffle(rd, c.len+1);
				for (int i = 0; i < c.len+1; ++i) {
					context *h = lm->make(c, rd[i]);
					lm->add(c[rd[i]], h);
				}
			}
			static void remove_h(word& w, hdp *lm) {
				for (int i = 0; i < w.len+1; ++i) {
					context *h = lm->find(w, i);
					lm->remove(w[i], h);
				}
			}
			static void remove_h(chunk& c, hdp *lm) {
				for (int i = 0; i < c.len+1; ++i) {
					context *h = lm->find(c, i);
					lm->remove(c[i], h);
				}
			}
			static void remove(word& w, hpyp *lm) {
				for (int i = 0; i < w.len+1; ++i) {
					context *h = lm->find(w, i);
					lm->remove(w[i], h);
				}
			}
			static void remove(chunk& c, hpyp *lm) {
				for (int i = 0; i < c.len+1; ++i) {
					context *h = lm->find(c, i);
					lm->remove(c[i], h);
				}
			}
			static void add_h(sentence& s, hdp *lm) {
				int rd[s.size()+1] = {0};
				rd::shuffle(rd, s.size()+1);
				for (int i = 0; i < s.size()+1; ++i) {
					context *h = lm->make(s, rd[i]);
					lm->add(s.wd(rd[i]), h);
				}
			}
			static void add_h(nsentence& s, hdp *lm) {
				int rd[s.size()+1] = {0};
				rd::shuffle(rd, s.size()+1);
				for (int i = 0; i < s.size()+1; ++i) {
					context *h = lm->make(s, rd[i]);
					lm->add(s.ch(rd[i]), h);
				}
			}
			static void add(sentence& s, hpyp *lm) {
				int rd[s.size()+1] = {0};
				rd::shuffle(rd, s.size()+1);
				for (int i = 0; i < s.size()+1; ++i) {
					context *h = lm->make(s, rd[i]);
					lm->add(s.wd(rd[i]), h);
				}
			}
			static void add(nsentence& s, hpyp *lm) {
				int rd[s.size()+1] = {0};
				rd::shuffle(rd, s.size()+1);
				for (int i = 0; i < s.size()+1; ++i) {
					context *h = lm->make(s, rd[i]);
					lm->add(s.ch(rd[i]), h);
				}
			}
			static void remove_h(sentence& s, hdp *lm) {
				for (int i = 0; i < s.size()+1; ++i) {
					context *h = lm->find(s, i);
					lm->remove(s.wd(i), h);
				}
			}
			static void remove_h(nsentence& s, hdp *lm) {
				for (int i = 0; i < s.size()+1; ++i) {
					context *h = lm->find(s, i);
					lm->remove(s.ch(i), h);
				}
			}
			static void remove(sentence& s, hpyp *lm) {
				for (int i = 0; i < s.size()+1; ++i) {
					context *h = lm->find(s, i);
					lm->remove(s.wd(i), h);
				}
			}
			static void remove(nsentence& s, hpyp *lm) {
				for (int i = 0; i < s.size()+1; ++i) {
					context *h = lm->find(s, i);
					lm->remove(s.ch(i), h);
				}
			}
			static void add_v(word& w, hpyp *lm) {
				int rd[w.len+1] = {0};
				rd::shuffle(rd, w.len+1);
				for (int i = 0; i < w.len+1; ++i) {
					int n = lm->draw_n(w, rd[i]);
					context *h = lm->make(w, rd[i], n);
					lm->add(w[rd[i]], h);
					w.m[rd[i]] = n;
				}
			}
			static void add_v(chunk& c, hpyp *lm) {
				int rd[c.len+1] = {0};
				rd::shuffle(rd, c.len+1);
				for (int i = 0; i < c.len+1; ++i) {
					int n = lm->draw_n(c, rd[i]);
					context *h = lm->make(c, rd[i], n);
					lm->add(c[rd[i]], h);
					c.n[rd[i]] = n;
				}
			}
			static void remove_v(word& w, hpyp *lm) {
				for (int i = 0; i < w.len+1; ++i) {
					int n = w.m[i];
					context *h = lm->find(w, i, n);
					lm->remove(w[i], h);
				}
			}
			static void remove_v(chunk& c, hpyp *lm) {
				for (int i = 0; i < c.len+1; ++i) {
					int n = c.n[i];
					context *h = lm->find(c, i, n);
					lm->remove(c[i], h);
				}
			}
			static void add_v(sentence& s, hpyp *lm) {
				int rd[s.size()+1] = {0};
				rd::shuffle(rd, s.size()+1);
				for (int i = 0; i < s.size()+1; ++i) {
					int n = lm->draw_n(s, rd[i]);
					context *h = lm->make(s, rd[i], n);
					lm->add(s.wd(rd[i]), h);
					s.n[rd[i]] = n;
				}
			}
			static void add_v(nsentence& s, hpyp *lm) {
				int rd[s.size()+1] = {0};
				rd::shuffle(rd, s.size()+1);
				for (int i = 0; i < s.size()+1; ++i) {
					int n = lm->draw_n(s, rd[i]);
					context *h = lm->make(s, rd[i], n);
					lm->add(s.ch(rd[i]), h);
					s.n[rd[i]] = n;
				}
			}
			static void remove_v(sentence& s, hpyp *lm) {
				for (int i = 0; i < s.size()+1; ++i) {
					int n = s.n[i];
					context *h = lm->find(s, i, n);
					lm->remove(s.wd(i), h);
				}
			}
			static void remove_v(nsentence& s, hpyp *lm) {
				for (int i = 0; i < s.size()+1; ++i) {
					int n = s.n[i];
					context *h = lm->find(s, i, n);
					lm->remove(s.ch(i), h);
				}
			}
			static void add_a(word& w, lm *lm) {
				std::type_index type = typeid(*lm);
				if (type == typeid(hpyp)) {
					add(w, (hpyp*)lm);
				} else if (type == typeid(vpyp)) {
					add_v(w, (vpyp*)lm);
				} else if (type == typeid(hdp)) {
					add_h(w, (hdp*)lm);
				}
			}
			static void add_a(chunk& c, lm *lm) {
				std::type_index type = typeid(*lm);
				if (type == typeid(hpyp)) {
					add(c, (hpyp*)lm);
				} else if (type == typeid(vpyp)) {
					add_v(c, (vpyp*)lm);
				} else if (type == typeid(hdp)) {
					add_h(c, (hdp*)lm);
				}
			}
			static void add_a(sentence& s, lm *lm) {
				std::type_index type = typeid(*lm);
				if (type == typeid(hpyp)) {
					add(s, (hpyp*)lm);
				} else if (type == typeid(vpyp)) {
					add_v(s, (vpyp*)lm);
				} else if (type == typeid(hdp)) {
					add_h(s, (hdp*)lm);
				}
			}
			static void add_a(nsentence& s, lm *lm) {
				std::type_index type = typeid(*lm);
				if (type == typeid(hpyp)) {
					add(s, (hpyp*)lm);
				} else if (type == typeid(vpyp)) {
					add_v(s, (vpyp*)lm);
				} else if (type == typeid(hdp)) {
					add_h(s, (hdp*)lm);
				}
			}
			static void remove_a(word& w, lm *lm) {
				std::type_index type = typeid(*lm);
				if (type == typeid(hpyp)) {
					remove(w, (hpyp*)lm);
				} else if (type == typeid(vpyp)) {
					remove_v(w, (vpyp*)lm);
				} else if (type == typeid(hdp)) {
					remove_h(w, (hdp*)lm);
				}
			}
			static void remove_a(chunk& c, lm *lm) {
				std::type_index type = typeid(*lm);
				if (type == typeid(hpyp)) {
					remove(c, (hpyp*)lm);
				} else if (type == typeid(vpyp)) {
					remove_v(c, (vpyp*)lm);
				} else if (type == typeid(hdp)) {
					remove_h(c, (hdp*)lm);
				}
			}
			static void remove_a(sentence& s, lm *lm) {
				std::type_index type = typeid(*lm);
				if (type == typeid(hpyp)) {
					remove(s, (hpyp*)lm);
				} else if (type == typeid(vpyp)) {
					remove_v(s, (vpyp*)lm);
				} else if (type == typeid(hdp)) {
					remove_h(s, (hdp*)lm);
				}
			}
			static void remove_a(nsentence& s, lm *lm) {
				std::type_index type = typeid(*lm);
				if (type == typeid(hpyp)) {
					remove(s, (hpyp*)lm);
				} else if (type == typeid(vpyp)) {
					remove_v(s, (vpyp*)lm);
				} else if (type == typeid(hdp)) {
					remove_h(s, (hdp*)lm);
				}
			}
		private:
	};
}

#endif
