#include"cio.h"
#include"util.h"
#include<cstdlib>
#include<cstdio>

using namespace std;
using namespace npbnlp;

vector<string> files;

void usage(int argc, char **argv) {
	cout << "[Usage]" << *argv << " file1 file2 ..." << endl;
	exit(1);
}

int read_param(int argc, char **argv) {
	if (argc < 3) {
		usage(argc, argv);
		return 1;
	}
	for (auto i = 1; i < argc; ++i) {
		files.emplace_back(string(*(argv+i)));
	}
	return 0;
}

int eval() {
	vector<cio> f;
	for (auto& s : files) {
		f.emplace_back(cio(s.c_str()));
	}
	int size = f[0].chunk->size();
	for (auto& c : f) {
		if ((int)c.chunk->size() != size) {
			cout << "invalid chunk_size is found: " << c.chunk->size() << " != " << size << endl;
			return 1;
		}
	}
	vector<vector<int> > n;
	for (auto& c : f) {
		int i = 0;
		for (auto& d : (*c.chunk)) {
			if ((int)n.size() < i+1)
				n.resize(i+1, vector<int>());
			int size = d.raw->size();
			if ((int)n[i].size() < size+1)
				n[i].resize(size+1, 0);
			for (auto& h : d.head) {
				n[i][h]++;
			}
			++i;
		}
	}
	int reviewer = f.size();
	double P = 0;
	double Pb = 0;
	int N = 0;
	for (auto& i : n) {
		N += i.size();
		for (auto& j : i) {
			int nb = reviewer - j;
			P += (double)(j*j + nb*nb - reviewer)/(reviewer*(reviewer-1));
			Pb += j;
		}
	}
	P /= N;
	Pb /= (N*reviewer);
	double Pe = Pb*Pb + (1.-Pb)*(1.-Pb);
	double kappa = (P-Pe)/(1.-Pe);
	cout << "fleiss' kappa = " << kappa << endl;
	return 0;
}

int main(int argc, char **argv) {
	try {
		if (read_param(argc, argv))
			return 1;
		eval();
	} catch (const char *ex) {
		cerr << ex << endl;
		return 1;
	}
	return 0;
}
