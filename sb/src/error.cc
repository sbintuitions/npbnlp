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

void dump(io& f, int h, int t, const char *comment) {
	cout << comment;
	for (int i = h; i < t; ++i) {
		char buf[5] = {0};
		io::i2c((*f.raw)[i], buf);
		cout << buf;
	}
	cout << endl;
}

int error() {
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
	for (auto i = 0; i < (int)t.chunk->size(); ++i) {
		int j = 0;
		int k = 0;
		if ((*c.chunk)[i].head.size() != (*t.chunk)[i].head.size()) {
			while (1) {
				if ((*c.chunk)[i].head[j] == (*t.chunk)[i].head[k]) {
					++j; ++k;
				} else if ((*c.chunk)[i].head[j] < (*t.chunk)[i].head[k]) {
					cout << "correct head:" << (*c.chunk)[i].head[j] << " predict head:" << (*t.chunk)[i].head[k] << endl;
					dump((*c.chunk)[i], (*c.chunk)[i].head[j-1], (*c.chunk)[i].head[j], "correct:");
					dump((*t.chunk)[i], (*t.chunk)[i].head[k-1], (*t.chunk)[i].head[k], "predict:");
					++j;
				} else if ((*c.chunk)[i].head[j] > (*t.chunk)[i].head[k]) {
					cout << "correct head:" << (*c.chunk)[i].head[j] << " predict head:" << (*t.chunk)[i].head[k] << endl;
					dump((*c.chunk)[i], (*c.chunk)[i].head[j-1], (*c.chunk)[i].head[j], "correct:");
					dump((*t.chunk)[i], (*t.chunk)[i].head[k-1], (*t.chunk)[i].head[k], "predict:");
					++k;
				}
				if (j >= (*c.chunk)[i].head.size() || k >= (*t.chunk)[i].head.size())
					break;
			}
		}
	}
	return 0;
}

int main(int argc, char **argv) {
	try {
		if (read_param(argc, argv))
			return 1;
		error();
	} catch (const char *ex) {
		cerr << ex << endl;
		return 1;
	}
	return 0;
}
