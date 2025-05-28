#include"hpy.h"
#include"vpy.h"
#include"io.h"
#include"util.h"
#include"rd.h"
#include<cmath>

using namespace std;
using namespace npbnlp;

double perplexity(vector<sentence>& c, hpy& lm) {
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
	int m = 10;
	hpy wlm(n);
	vpy clm(m);
	wlm.set_base(&clm);
	io f(*(argv+1));

	vector<sentence>data;
	util::store_sentences(f, data);
	for (auto i = 0; i < 100; ++i) {
		int rd[data.size()] = {0};
		rd::shuffle(rd, data.size());
		for (auto j = 0; j < (int)data.size(); ++j) {
			if (i > 0)
				wlm.remove(data[rd[j]]);
			wlm.add(data[rd[j]]);
		}
		wlm.estimate(20);
		clm.estimate(20);
		cout << i << " train ppl:" << perplexity(data, wlm) << endl;
	}
	wlm.save("hpy.model");
	clm.save("vpy.model");

	return 0;
}
