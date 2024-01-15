#include"hlattice.h"
#include<cmath>
#ifdef _OPENMP
#include<omp.h>
#endif

#define ZERO 1e-36
using namespace std;
using namespace npbnlp;

static word eos;
static vector<int> bos(1, 0);

hlattice::hlattice(io& f, int i) {
	int head = f.head[i];
	int tail = f.head[i+1];
	s = sentence(*f.raw, head, tail);
	mu.resize(s.size(), 0);
	k.resize(s.size());
}

hlattice::~hlattice() {
}

word& hlattice::wd(int i) {
	if (i < 0 || i >= s.size())
		return eos;
	return s.wd(i);
}

word* hlattice::wp(int i) {
	if (i < 0 || i >= s.size())
		return &eos;
	return &s.w[i];
}

int hlattice::size(int i) {
	if (i < 0 || i >= k.size())
		return 1;
	return k[i].size();
}

void hlattice::slice(int i, double u) {
	if (i >= 0 && i < s.size())
		mu[i] = u;
}

double hlattice::u(int i) {
	if (i < 0 || i >= s.size())
		return log(ZERO);
	return mu[i];
}

vector<int>::iterator hlattice::begin(int i) {
	if (i < 0 || i >= s.size())
		return bos.begin();
	return k[i].begin();
}

vector<int>::iterator hlattice::end(int i) {
	if (i < 0 || i >= s.size())
		return bos.end();
	return k[i].end();
}
