#include<getopt.h>
#include<cstdlib>
#include<cstdio>
#include"npylm.h"
#include"nnpylm.h"
#include"util.h"
#include"rd.h"

#define check(opt,arg) (strcmp(opt,arg) == 0)
#define PBSTR "||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||"
#define PBWIDTH 60

using namespace npbnlp;
using namespace std;

static int n = 2;
static int thread = 4;
static int epoch = 500;
static int dmp = 0;
static string train;
static string test;
static string tokenizer("npylm.model");
static string model("nnpylm.model");
static string cdic("chunk.dic");
static string wdic("word.dic");

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
	cout << *argv << " --train file --tokenizer npylm.model --model file_to_save --cdic dicfile --wdic word.dic\n";
	cout << *argv << " --parse file --tokenizer npylm.model --model modelfile --cdic dicfile --wdic word.dic\n";
	cout << "[options]\n";
	cout << "-n, --order=int(default 2)\n";
	cout << "-e, --epoch=int(default 500)\n";
	cout << "-t, --thread=int(default 4)\n";
	exit(1);
}


int read_long_param(const char *opt, const char *arg) {
	if (check(opt, "train")) {
		train = arg;
	} else if (check(opt, "parse")) {
		test = arg;
	} else if (check(opt, "model")) {
		model = arg;
	} else if (check(opt, "cdic")) {
		cdic = arg;
	} else if (check(opt, "wdic")) {
		wdic = arg;
	} else if (check(opt, "order")) {
		n = atoi(arg);
	} else if (check(opt, "epoch")) {
		epoch = atoi(arg);
	} else if (check(opt, "thread")) {
		thread = atoi(arg);
	} else if (check(opt, "dump")) {
		dmp = atoi(arg);
	} else if (check(opt, "tokenizer")) {
		tokenizer = arg;
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
			{"model", required_argument, 0, 0},
			{"cdic", required_argument, 0,0},
			{"wdic", required_argument, 0,0},
			{"order", required_argument, 0,0},
			{"epoch", required_argument, 0,0},
			{"thread", required_argument, 0,0},
			{"dump", required_argument, 0,0},
			{"tokenizer", required_argument, 0,0},
			{0, 0, 0, 0}
		};
		int option_index = 0;
		c = getopt_long(argc, argv, "n:e:t:d:", long_options, &option_index);
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
			case 'e':
				epoch = atoi(optarg);
				break;
			case 't':
				thread = atoi(optarg);
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

void dump(nsentence& s) {
	for (auto i = 0; i < s.size(); ++i)
		cout << s.ch(i) << endl;
	cout << endl;
}

int tokenize(io& f, vector<sentence>& c) {
	shared_ptr<wid> d = wid::create();
	d->load(wdic.c_str());
	npylm lm;
	lm.load(tokenizer.c_str());
	c.resize(f.head.size()-1);
#ifdef _OPENMP
	omp_set_num_threads(thread);
#pragma omp parallel for ordered schedule(dynamic)
#endif
	for (auto i = 0; i < f.head.size()-1; ++i) {
		c[i] = lm.parse(f, i);
	}
	// indexing
	for (auto s = c.begin(); s != c.end(); ++s) {
		for (auto j = 0; j < s->size(); ++j) {
			word& w = s->wd(j);
			if (w.id == 1) {
				w.id = d->index(w);
			}
		}
	}
	d->save(wdic.c_str());
	return 0;
}

int mcmc() {
	io g(train.c_str());
	vector<sentence> ws;
	tokenize(g, ws);
	nio f(ws);
	/*
	for (auto i = 0; i < f.head.size()-1; ++i) {
		for (auto j = f.head[i]; j < f.head[i+1]; ++j) {
			cout << (*f.raw)[j] << endl;
		}
	}
	*/
	vector<nsentence> corpus(f.head.size()-1);

	npylm lm;
	lm.load(tokenizer.c_str());
	nnpylm chunker(n, lm.n(), lm.m());
#ifdef _OPENMP
	omp_set_num_threads(thread);
#endif
	for (auto i = 0; i < epoch; ++i) {
		int rd[corpus.size()] = {0};
		rd::shuffle(rd, corpus.size());;
		int j = 0;
		while (j < corpus.size()) {
			if (i > 0) {
				for (auto t = 0; t < thread; ++t) {
					if (j+t < corpus.size())
						chunker.remove(corpus[rd[j+t]]);
				}
			}
#ifdef _OPENMP
#pragma omp parallel
			{
				auto t = omp_get_thread_num();
				if (j+t < corpus.size()) {
					try {
						nsentence s = chunker.sample(f, rd[j+t]);
						corpus[rd[j+t]] = s;
					} catch (const char *ex) {
						cerr << ex << endl;
					}
				}
			}
#else
			for (auto t = 0; t < thread; ++t) {
				if (j+t < corpus.size()) {
					try {
						nsentence s = chunker.sample(f, rd[j+t]);
						corpus[rd[j+t]] = s;
					} catch (const char *ex) {
						cerr << ex << endl;
					}
				}
			}
#endif
			for (auto t = 0; t < thread; ++t) {
				if (j+t < corpus.size()) {
					chunker.add(corpus[rd[j+t]]);
				}
			}
			j += thread;
#ifdef _OPENMP
#pragma omp ordered
#endif
			progress(i, (double)j/corpus.size());
		}
		chunker.estimate(20);
		if (i)
			chunker.poisson_correction();
		if (dmp && i%dmp == 0) {
			cout << endl;
			for (auto s = corpus.begin(); s != corpus.end(); ++s)
				dump(*s);
		}
	}
	cout << endl;
	chunker.save(model.c_str());
	shared_ptr<cid> d = cid::create();
	d->save(cdic.c_str());
	return 0;
}

int parse() {
	io g(test.c_str());
	vector<sentence> ws;
	tokenize(g, ws);
	shared_ptr<cid> d = cid::create();
	d->load(cdic.c_str());
	nio f(ws);
	npylm lm;
	lm.load(tokenizer.c_str());
	nnpylm chunker(n, lm.n(), lm.m());
	chunker.load(model.c_str());
#ifdef _OPENMP
	omp_set_num_threads(thread);
#pragma omp parallel for ordered schedule(dynamic)
#endif
	for (auto i = 0; i < f.head.size()-1; ++i) {
		nsentence s = chunker.parse(f, i);
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
