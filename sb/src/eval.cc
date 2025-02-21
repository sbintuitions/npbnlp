#include"cio.h"
#include"util.h"
#include<getopt.h>
#include<cstdlib>
#include<cstdio>
#define check(opt,arg) (strcmp(opt,arg) == 0)

using namespace std;
using namespace npbnlp;

static string correct;
static string target;

void usage(int argc, char **argv) {
	cout << "[Usage]" << *argv << " --target file_to_eval --correct correct_segmented_file" << endl;
	exit(1);
}

int read_long_param(const char *opt, const char *arg) {
	if (check(opt, "target")) {
		target = arg;
	} else if (check(opt, "correct")) {
		correct = arg;
	} else {
		return 1;
	}
	return 0;
}

int read_param(int argc, char **argv) {
	if (argc < 3) {
		usage(argc, argv);
		return 1;
	}
	int c;
	while (1) {
		static struct option long_options[] = {
			{"target", required_argument, 0, 0},
			{"correct", required_argument, 0, 0},
			{0, 0, 0, 0}
		};
		int option_index = 0;
		c = getopt_long(argc, argv, "t:c:", long_options, &option_index);
		if (c == -1)
			break;
		switch (c) {
			case 0:
				if (long_options[option_index].flag != 0)
					break;
				read_long_param(long_options[option_index].name, optarg);
				break;
			case 't':
				target = optarg;
				break;
			case 'c':
				correct = optarg;
				break;
			case '?':
			default:
				usage(argc, argv);
		}
	}
	if (optind < argc) {
		cerr << "non-option ARGV-elements: ";
		while (optind < argc) {
			cerr << argv[optind++] << " ";
		}
		cerr << endl;
		usage(argc, argv);
		return 1;
	}
	return 0;
}

int eval() {
	if (target.empty() || correct.empty()) {
		cout << "ERR: missing file" << endl;
		cout << "target:" << target << endl;
		cout << "correct:" << correct << endl;
		return 1;
	}
	cio t(target.c_str());
	cio c(correct.c_str());
	if (t.chunk->size() != c.chunk->size()) {
		cout << "target_size:" << t.chunk->size() << " != " << "correct_size:" << c.chunk->size() << endl;
		return 1;
	}
	int letters = 0;
	int correct_seg = 0;
	int target_seg = 0;
	int n = 0;
	int fp = 0;
	int fn = 0;
	for (auto i = 0; i < (int)t.chunk->size(); ++i) {
		correct_seg += (*c.chunk)[i].head.size();
		target_seg += (*t.chunk)[i].head.size();
		letters += (*t.chunk)[i].raw->size();
		int j = 0;
		int k = 0;
		while (1) {
			if ((*c.chunk)[i].head[j] == (*t.chunk)[i].head[k]) {
				++n;
				++j;
				++k;
			} else if ((*c.chunk)[i].head[j] < (*t.chunk)[i].head[k]) {
				++j;
				++fn;
			} else if ((*c.chunk)[i].head[j] > (*t.chunk)[i].head[k]) {
				++k;
				++fp;
			}
			if (j >= (int)(*c.chunk)[i].head.size() && k < (int)(*t.chunk)[i].head.size()) {
				int check = (*c.chunk)[i].head[(*c.chunk)[i].head.size()-1];
				for (; k < (int)(*t.chunk)[i].head.size(); ++k) {
					int h = (*t.chunk)[i].head[k];
					if (check == h) {
						++n;
					} else {
						++fp;
					}
				}
			} else if (j < (int)(*c.chunk)[i].head.size() && k >= (int)(*t.chunk)[i].head.size()) {
				int check = (*t.chunk)[i].head[(*t.chunk)[i].head.size()-1];
				for (; j < (int)(*c.chunk)[i].head.size(); ++j) {
					int h = (*c.chunk)[i].head[j];
					if (check == h) {
						++n;
					} else {
						++fn;
					}
				}
			}
			if (j >= (int)(*c.chunk)[i].head.size() && k >= (int)(*t.chunk)[i].head.size()) {
				break;
			}
		}
	}
	//cout << "precision:" << (double)n/target_seg << endl;
	//cout << "recall:" << (double)n/correct_seg << endl;
	//cout << "f1:" << ((double)2*(double)n/target_seg*(double)n/correct_seg)/((double)n/target_seg+(double)n/correct_seg) << endl;
	int tp = n;
	//int tn = letters-correct_seg-fp;
	int tn = letters-correct_seg;
	double prec = (double)tp/((double)tp+fp);
	double rec = (double)tp/((double)tp+fn);
	double f1 = (double)2*prec*rec/(prec+rec);
	cout << "prec:" << (double)tp/((double)tp+fp) << endl;
	cout << "rec:" << (double)tp/((double)tp+fn) << endl;
	//cout << "f1:" << f1 << endl;
	cout << "f1:" << ((double)2*(double)n/target_seg*(double)n/correct_seg)/((double)n/target_seg+(double)n/correct_seg) << endl;
	cout << "acc:" << (double)(tp+tn)/(double)(tp+fp+tn+fn) << endl;
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
