#include"hdp.h"
#include"hpyp.h"
#include"io.h"
#include"sentence.h"
#include"util.h"
#include"convinience.h"
#include<iostream>

using namespace std;
using namespace npbnlp;

double perplexity(vector<sentence>& c, hdp *lm) {
	double ll = 0;
	int v = 0;
	int n = lm->n();
	for (auto it = c.begin(); it != c.end(); ++it) {
		v += it->size();
		for (int i = 0; i < it->size(); ++i) {
			context *h = lm->find(*it, i);
			ll += lm->lp(it->wd(i), h);
		}
	}
	return exp(-ll/v);
}

int main(int argc, char **argv) {
	hdp l(3);
	vpyp m(10);
	l.set_base(&m);
	l.set_v(10000);
	m.set_v(5000);
	bool test_flg = false;
	try {
		shared_ptr<wid> dic = wid::create();
		if ((test_flg = dic->load("hdp_test.dic"))) {
			l.load("hdp.model");
			m.load("base.model");
		}
		io f(*(argv+1));
		io *g = NULL;
		vector<sentence> d;
		if (argc > 2) {
			g = new io(*(argv+2));
			util::store_sentences(*g, d);
		}
		vector<sentence> c;
		util::store_sentences(f, c);
		if (test_flg) {
			cout << "load V:" << l.v() << " load C:" << m.v() << " train ppl:" << perplexity(c, &l);
			if (g) {
				cout << " test ppl:" << perplexity(d, &l);
					delete g;
			}
			cout << endl;
			return 0;
		}
		for (auto i = 0; i < 100; ++i) {
			int rd[c.size()] = {0};
			rd::shuffle(rd, c.size());
			for (auto j = 0; j < c.size(); ++j) {
				if (i > 0) {
					wrap::remove_a(c[rd[j]], &l);
				}
				wrap::add_a(c[rd[j]], &l);
			}
			l.estimate(20);
			m.estimate(1);
			cout << "V:" << l.v() << " C:" << m.v() << " ";
			cout << i << " train ppl:" << perplexity(c, &l);
			if (g)
				cout << " test ppl:" << perplexity(d, &l);
			cout << endl;
		}
		l.save("hdp.model");
		m.save("base.model");
		dic->save("hdp_test.dic");
		if (g)
			delete g;
	} catch (const char *ex) {
		cerr << ex << endl;
	}
	return 0;
}
