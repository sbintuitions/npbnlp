#include<getopt.h>
#include<cstdlib>
#include<cstdio>
#include"npylm.h"
#include"util.h"
#include"rd.h"
#ifdef _OPENMP
#include<omp.h>
#endif

#define check(opt,arg) (strcmp(opt,arg) == 0)
#define PBSTR "||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||"
#define PBWIDTH 60

using namespace std;
using namespace npbnlp;

static int n = 2;
static int m = 20;
static int threads = 4;
static int epoch = 500;
static int dmp = 0;
static int vocab = 5000;
static string train;
static string test;
static string model("npylm.model");
static string dic("word.dic");

void progress(int i, double pct) {
	int val = (int) (pct * 100);
	int lpad = (int) (pct * PBWIDTH);
	int rpad = PBWIDTH - lpad;
	printf("\repoch %4d %3d%% [%.*s%*s]", i, val, lpad, PBSTR, rpad, "");
	fflush(stdout);
}

void usage(int argc, char **argv) {
	cout << "[Usage]" << *argv << " [options]\n";
	cout << "[example]\n";
	cout << *argv << " --train file --model file_to_save --dic dicfile\n";
	cout << *argv << " --parse file --model modelfile --dic dicfile\n";
	cout << "[options]\n";
	cout << "-n, --word_order=int(default 2)\n";
	cout << "-m, --letter_order=int(default 20)\n";
	cout << "-e, --epoch=int(default 500)\n";
	cout << "-t, --threads=int(default 4)\n";
	cout << "-v, --vocab=int(means letter variations. default 5000)\n";
	exit(1);
}

int read_long_param(const char *opt, const char *arg) {
	if (check(opt, "train")) {
		train = arg;
	} else if (check(opt, "parse")) {
		test = arg;
	} else if (check(opt, "model")) {
		model = arg;
	} else if (check(opt, "dic")) {
		dic = arg;
	} else if (check(opt, "word_order")) {
		n = atoi(arg);
	} else if (check(opt, "letter_order")) {
		m = atoi(arg);
	} else if (check(opt, "epoch")) {
		epoch = atoi(arg);
	} else if (check(opt, "threads")) {
		threads = atoi(arg);
	} else if (check(opt, "dump")) {
		dmp = atoi(arg);
	} else if (check(opt, "vocab")) {
		vocab = atoi(arg);
	} else {
		return 1;
	}
	return 1;
}

int read_param(int argc, char **argv) {
	if (argc < 2) {
		usage(argc, argv);
		return 1;
	}
	int c;
	while (1) {
		static struct option long_options[] =
		{
			{"train", required_argument, 0, 0},
			{"parse", required_argument, 0, 0},
			{"dic", required_argument, 0, 0},
			{"model", required_argument, 0, 0},
			{"word_order", required_argument, 0, 0},
			{"letter_order", required_argument, 0, 0},
			{"epoch", required_argument, 0, 0},
			{"threads", required_argument, 0, 0},
			{"dump", required_argument, 0, 0},
			{"vocab", required_argument, 0, 0},
			{0, 0, 0, 0}
		};
		int option_index = 0;
		c = getopt_long(argc, argv, "n:m:e:t:v:", long_options, &option_index);
		if (c == -1)
			break;
		switch (c) {
			case 0:
				if (long_options[option_index].flag != 0)
					break;
				read_long_param(long_options[option_index].name, optarg);
				break;
			case 'n':
				n = atoi(optarg);
				break;
			case 'm':
				m = atoi(optarg);
				break;
			case 'e':
				epoch = atoi(optarg);
				break;
			case 't':
				threads = atoi(optarg);
				break;
			case 'v':
				vocab = atoi(optarg);
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

void dump(sentence& s) {
	for (auto i = 0; i < s.size(); ++i)
		cout << s.wd(i) << endl;
	cout << endl;
}

int mcmc() {
	io f(train.c_str());
	vector<sentence> corpus;
	util::store_sentences(f, corpus);
	npylm lm(n, m);
	lm.set(vocab);
#ifdef _OPENMP
	threads = min(omp_get_max_threads(), threads);
	omp_set_num_threads(threads);
#endif
	for (auto i = 0; i < epoch; ++i) {
		int rd[corpus.size()] = {0};
		rd::shuffle(rd, corpus.size());
		int j = 0;
		while (j < (int)corpus.size()) {
			// remove
			if (i > 0) {
				for (auto t = 0; t < threads; ++t) {
					if (j+t < (int)corpus.size()) {
						lm.remove(corpus[rd[j+t]]);
					}
				}
#ifdef _OPENMP
#pragma omp parallel
				{ // sample segmentations
					auto t = omp_get_thread_num();
					if (j+t < (int)corpus.size()) {
						try {
							sentence s = lm.sample(f, rd[j+t]);
							corpus[rd[j+t]] = s;
						} catch (const char *ex) {
							cerr << ex << endl;
						}
					}
				}
#else
				for (auto t = 0; t < threads; ++t) {
					if (j+t < (int)corpus.size()) {
						try {
							sentence s = lm.sample(f, rd[j+t]);
							corpus[rd[j+t]] = s;
						} catch (const char *ex) {
							cerr << ex << endl;
						}
					}
				}
#endif
			}
			// add
			for (auto t = 0; t < threads; ++t) {
				if (j+t < (int)corpus.size()) {
					lm.add(corpus[rd[j+t]]);
					/*
					   if (dmp && i%dmp == 0)
					   dump(corpus[rd[j+t]]);
					 */
				}
			}
			j += threads;
			progress(i, (double)(j+1)/corpus.size());
		}
		// estimate hyperparameter
		lm.estimate(20);
		if (i)
			lm.poisson_correction(1000);
		if (dmp && (i+1)%dmp == 0) {
			cout << endl;
			for (auto s = corpus.begin(); s != corpus.end(); ++s)
				dump(*s);
		}
	}
	cout << endl;
	lm.save(model.c_str());
	shared_ptr<wid> d = wid::create();
	d->save(dic.c_str());
	return 0;
}

int parse() {
	io f(test.c_str());
	shared_ptr<wid> d = wid::create();
	d->load(dic.c_str());
	npylm lm(n, m);
	lm.load(model.c_str());
#ifdef _OPENMP
	omp_set_num_threads(threads);
#pragma omp parallel for ordered schedule(dynamic)
#endif
	for (auto i = 0; i < (int)f.head.size()-1; ++i) {
		sentence s = lm.parse(f, i);
#ifdef _OPENMP
#pragma omp ordered
#endif
		dump(s);
	}
	return 0;
}

int main(int argc, char **argv) {
	try {
		read_param(argc, argv);
		if (!train.empty()) {
			mcmc();
		}
		if (!test.empty()) {
			parse();
		}
	} catch (const char *ex) {
		cerr << ex << endl;
		return 1;
	}
	return 0;
}
