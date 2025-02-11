#include"kn.h"
#include"io.h"
#include"util.h"
#include"rd.h"
#include<cmath>

using namespace std;
using namespace npbnlp;

double perplexity(vector<sentence>& c, kn& lm) {
	double ll = 0;
	double v = 0;
	for (auto& s : c) {
		ll += lm.lp(s);
		v += s.size();
	}
	return exp(-ll/v);
}

int main(int argc, char **argv) {
	int n = 3;
	kn lm(n);
	io f(*(argv+1));

	vector<sentence> data;
	npbnlp::count c(3);
	util::store_sentences(f, data);
	for (auto i = 0; i < 100; ++i) {
		//cout << "\riter:" << i;
		//fflush(stdout);
		int rd[data.size()] = {0};
		rd::shuffle(rd, data.size());
		for (auto j = 0; j < (int)data.size(); ++j) {
			if (i > 0)
				lm.remove(data[rd[j]]);
			lm.add(data[rd[j]]);
		}
		cout << i << " train ppl:" << perplexity(data, lm) << endl;
	}
	lm.save("kn.model");

	return 0;
}
