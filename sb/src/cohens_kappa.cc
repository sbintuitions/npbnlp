#include"cio.h"
#include"util.h"
#include<getopt.h>
#include<cstdlib>
#include<cstdio>
#define check(opt,arg) (strcmp(opt,arg) == 0)

using namespace std;
using namespace npbnlp;

static string file1;
static string file2;

void usage(int argc, char **argv) {
	cout << "[Usage]" << *argv << " file1 file2" << endl;
	exit(1);
}

int read_param(int argc, char **argv) {
	if (argc < 3) {
		usage(argc, argv);
		return 1;
	}
	file1 = *(argv+1);
	file2 = *(argv+2);
	return 0;
}

int eval() {
	if (file1.empty() || file2.empty()) {
		cout << "ERR: missing file" << endl;
		cout << "file1:" << file1 << endl;
		cout << "file2:" << file2 << endl;
		return 1;
	}
	cio f1(file1.c_str());
	cio f2(file2.c_str());
	if (f1.chunk->size() != f2.chunk->size()) {
		cout << "file1_size:" << f1.chunk->size() << " != " << "file2_size:" << f2.chunk->size() << endl;
		return 1;
	}
	int chunksize = f1.chunk->size();
	int letters = 0;
	int f1_seg = 0;
	int f2_seg = 0;
	int n = 0;
	for (auto i = 0; i < chunksize; ++i) {
		f1_seg += (*f1.chunk)[i].head.size();
		f2_seg += (*f2.chunk)[i].head.size();
		letters += (*f1.chunk)[i].raw->size();
		int j = 0;
		int k = 0;
		while (1) {
			if ((*f1.chunk)[i].head[j] == (*f2.chunk)[i].head[k]) {
				++n;
				++j;
				++k;
			} else if ((*f1.chunk)[i].head[j] < (*f2.chunk)[i].head[k]) {
				++j;
			} else if ((*f1.chunk)[i].head[j] > (*f2.chunk)[i].head[k]) {
				++k;
			}
			if (j >= (int)(*f1.chunk)[i].head.size() || k >= (int)(*f2.chunk)[i].head.size()) {
				break;
			}
		}
	}
	//double observed = (double)n/letters+(double)(letters-(f1_seg-n)-(f2_seg-n))/letters;
	double observed = (double)n/letters+
		(double)(letters-n-((f1_seg-n)+(f2_seg-n)))/letters;
	double expected = (double)f1_seg/letters*(double)f2_seg/letters+(1.-(double)f1_seg/letters)*(1.-(double)f2_seg/letters);
	double kappa = (observed-expected)/(1.-expected);
	cout << "f1_seg:" << f1_seg << endl;
	cout << "f2_seg:" << f2_seg << endl;
	cout << "n:" << n << endl;
	cout << "letters:" << letters << endl;
	cout << "observed:" << observed << endl;
	cout << "expected:" << expected << endl;
	cout << "cohen's kappa = " << kappa << endl;
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
