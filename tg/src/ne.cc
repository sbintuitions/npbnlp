#include"nnpylm.h"
#include"nphsmm.h"
#include"npylm.h"
#include"rd.h"
#include"util.h"
#include<getopt.h>
#ifdef _OPENMP
#include<omp.h>
#endif

#define check(opt,arg) (strcmp(opt,arg) == 0)
#define PBSTR "||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||"
#define PBWIDTH 60
#define NNPYLM_EPOCH 100

using namespace std;
using namespace npbnlp;

static int n = 1;
static int m = 3;
static int l = 20;
static int k = 10;
static int K = 50;
static int threads = 4;
static int epoch = 500;
static int dmp = 0;
static int tokenized = 0;
static int vocab = 5000;
static double a = 1;
static double b = 5;
static string train;
static string test;
static string model("nphsmm.model");
static string cdic("ne.dic");
static string tokenizer("npylm.model");
static string wdic("word.dic");

void progress(const char *s, int i, double pct) {
	int val = (int) (pct * 100);
	int lpad = (int) (pct * PBWIDTH);
	int rpad = PBWIDTH - lpad;
	printf("\r%s %4d %3d%% [%.*s%*s]", s, i, val, lpad, PBSTR, rpad, "");
	fflush(stdout);
}

void usage(int argc, char **argv) {
	cout << "[Usage]" << *argv << " [options]\n";
	cout << "[example]\n";
	cout << *argv << " --train file --tokenizer npylm.model --wdic word.dic --model file_to_save --cdic chunk.dic\n";
	cout << *argv << " --parse file --tokenizer npylm.model --wdic word.dic --model modelfile --cdic chunk.dic\n";
	cout << "[options]\n";
	cout << "-n, --chunk_order=int(default 1)\n";
	cout << "-m, --word_order=int(default 3)\n";
	cout << "-l, --letter_order=int(default 20)\n";
	cout << "-k, --class=int(default 500)\n";
	cout << "-e, --epoch=int(default 500)\n";
	cout << "-t, --threads=int(default 4)\n";
	cout << "-v, --vocab=int(means letter variations. default 5000)\n";
	cout << "--tokenized=bool(default 0)\n";
	cout << "-a double(default 1), parameter of beta distribution for slice" << endl;
	cout << "-b double(default 5), parameter of beta distribution for slice" << endl;
	exit(1);
}

int read_long_param(const char *opt, const char *arg) {
	if (check(opt, "train")) {
		train = arg;
	} else if (check(opt, "parse")) {
		test = arg;
	} else if (check(opt, "model")) {
		model = arg;
	} else if (check(opt, "wdic")) {
		wdic = arg;
	} else if (check(opt, "cdic")) {
		cdic = arg;
	} else if (check(opt, "tokenizer")) {
		tokenizer = arg;
	} else if (check(opt, "chunk_order")) {
		n = atoi(arg);
	} else if (check(opt, "word_order")) {
		m = atoi(arg);
	} else if (check(opt, "letter_order")) {
		l = atoi(arg);
	} else if (check(opt, "class")) {
		K = atoi(arg);
		k = min(k, K);
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
			{"cdic", required_argument, 0, 0},
			{"wdic", required_argument, 0, 0},
			{"model", required_argument, 0, 0},
			{"tokenizer", required_argument, 0,0},
			{"chunk_order", required_argument, 0, 0},
			{"word_order", required_argument, 0, 0},
			{"letter_order", required_argument, 0, 0},
			{"class", required_argument, 0, 0},
			{"epoch", required_argument, 0, 0},
			{"threads", required_argument, 0, 0},
			{"dump", required_argument, 0, 0},
			{"vocab", required_argument, 0, 0},
			{"tokenized", no_argument, &tokenized, 1},
			{0, 0, 0, 0}
		};
		int option_index = 0;
		c = getopt_long(argc, argv, "n:m:l:k:e:t:v:a:b:", long_options, &option_index);
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
			case 'l':
				l = atoi(optarg);
				break;
			case 'k':
				K = atoi(optarg);
				k = min(k, K);
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
			case 'a':
				a = atof(optarg);
				break;
			case 'b':
				b = atof(optarg);
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
	threads = min(omp_get_max_threads(), threads);
	omp_set_num_threads(threads);
#pragma omp parallel for ordered schedule(dynamic)
#endif
	for (auto i = 0; i < (int)f.head.size()-1; ++i) {
		c[i] = lm.parse(f, i);
#ifdef _OPENMP
#pragma omp ordered
#endif
		progress("tokenizing",i,(double)(i+1)/(f.head.size()-1));
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
	int rpad = 2*PBWIDTH;
	printf("\r%*s", rpad,"");
	d->save(wdic.c_str());
	return 0;
}

int init(nio& f, vector<nsentence>& corpus) {
	nnpylm chunker(n, m, l);
	if (corpus.size() < f.head.size()-1) {
		corpus.resize(f.head.size()-1);
	}
#ifdef _OPENMP
	omp_set_num_threads(threads);
#endif
	for (int i = 0; i < NNPYLM_EPOCH; ++i) {
		int rd[corpus.size()] = {0};
		rd::shuffle(rd, corpus.size());
		int j = 0;
		while (j < (int)corpus.size()) {
			if (i > 0) {
				for (auto t = 0; t < threads; ++t) {
					if (j+t < (int)corpus.size())
						chunker.remove(corpus[rd[j+t]]);
				}
			}
#ifdef _OPENMP
#pragma omp parallel
			{
				auto t = omp_get_thread_num();
				if (j+t < (int)corpus.size()) {
					try {
						nsentence s  = chunker.sample(f, rd[j+t]);
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
						nsentence s = chunker.sample(f, rd[j+t]);
						corpus[rd[j+t]] = s;
					} catch (const char *ex) {
						cerr << ex << endl;
					}
				}
			}
#endif
			for (auto t = 0; t < threads; ++t) {
				if (j+t < (int)corpus.size()) {
					chunker.add(corpus[rd[j+t]]);
				}
			}
			j += threads;
#ifdef _OPENMP
#pragma omp ordered
#endif
			progress("init", i, (double)(i+1)/NNPYLM_EPOCH);
		}
		chunker.estimate(20);
		if (i)
			chunker.poisson_correction(5000);
	}
	int rpad = 2*PBWIDTH;
	printf("\r%*s", rpad, "");
	return 0;
}

int mcmc(nio& f, vector<nsentence>& corpus) {
	nphsmm lm(n, m, l, k);
	lm.set(vocab, K);
	lm.slice(a, b);
#ifdef _OPENMP
	omp_set_num_threads(threads);
#endif
	int rid[corpus.size()] = {0};
	rd::shuffle(rid, corpus.size());
	for (auto i = 0; i < (int)corpus.size(); ++i)
		lm.init(corpus[rid[i]]);
	lm.estimate(20);
	lm.poisson_correction(100);
	for (auto i = 0; i < epoch; ++i) {
		int rd[corpus.size()] = {0};
		rd::shuffle(rd, corpus.size());
		int j = 0;
		while (j < (int)corpus.size()) {
			for (auto t = 0; t < threads; ++t) {
				if (j+t < (int)corpus.size())
					lm.remove(corpus[rd[j+t]]);
			}
#ifdef _OPENMP
#pragma omp parallel
			{
				auto t = omp_get_thread_num();
				if (j+t < (int)corpus.size()) {
					try {
						nsentence s = lm.sample(f, rd[j+t]);
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
						nsentence s = lm.sample(f, rd[j+t]);
						corpus[rd[j+t]] = s;
					} catch (const char *ex) {
						cerr << ex << endl;
					}
				}
			}
#endif
			for (auto t = 0; t < threads; ++t) {
				if (j+t < (int)corpus.size()) {
					lm.add(corpus[rd[j+t]]);
				}
			}
			j += threads;
#ifdef _OPENMP
#pragma omp ordered
#endif
			progress("epoch",i, (double)(j+1)/corpus.size());
		}
		lm.estimate(20);
		lm.poisson_correction(5000);
		if (dmp && (i+1)%dmp == 0) {
			cout << endl;
			for (auto s = corpus.begin(); s != corpus.end(); ++s)
				dump(*s);
		}
	}
	cout << endl;
	lm.save(model.c_str());
	shared_ptr<cid> d = cid::create();
	d->save(cdic.c_str());
	return 0;
}

int parse(nio& f) {
	shared_ptr<cid> d = cid::create();
	d->load(cdic.c_str());
	nphsmm lm;
	try {
		lm.load(model.c_str());
		lm.set(vocab, K);
	} catch (const char *ex) {
		throw ex;
	}
#ifdef _OPENMP
	omp_set_num_threads(threads);
#pragma omp parallel for ordered schedule(dynamic)
#endif
	for (auto i = 0; i < (int)f.head.size()-1; ++i) {
		nsentence s = lm.parse(f, i);
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
			io g(train.c_str());
			vector<sentence> ws;
			if (tokenized) {
				util::store_sentences(g, ws);
			} else {
				tokenize(g, ws);
			}
			nio f(ws);
			vector<nsentence> corpus(f.head.size()-1);
			init(f, corpus);
			mcmc(f, corpus);
		}
		if (!test.empty()) {
			io g(test.c_str());
			vector<sentence> ws;
			if (tokenized) {
				util::store_sentences(g, ws);
			} else {
				tokenize(g, ws);
			}
			nio f(ws);
			parse(f);
		}
	} catch (const char *ex) {
		cerr << ex << endl;
		return 1;
	}
	return 0;
}
