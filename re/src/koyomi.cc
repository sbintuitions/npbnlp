#include<getopt.h>
#include<cstdio>
#include<cstdlib>
#include"util.h"
#include"rd.h"
#include"hsmm.h"
#ifdef _OPENMP
#include<omp.h>
#endif

#define check(opt,arg) (strcmp(opt,arg) == 0)
#define PBSTR "||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||"
#define PBWIDTH 60

using namespace std;
using namespace npbnlp;

static int n = 10;
static int m = 3;
static int threads = 4;
static int epoch = 100;
static int dmp = 0;
static int vocab = 5000;
static string supervised;
static string train;
static string test;
static string model("koyomi.model");
static string wdic("koyomi.wdic");
static string triedic("trie.idx");
static string unitdic("");

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
	//cout << *argv << " --train file --model file_to_save --dic word_dic --trie phonetic_dic --unk character_dic\n";
	//cout << *argv << " --parse file --model modelfile --dic word_dic --trie phonetic_dic --unk character_dic\n";
	cout << *argv << " --train file --model file_to_save --dic word_dic --trie phonetic_dic\n";
	cout << *argv << " --parse file --model modelfile --dic word_dic --trie phonetic_dic\n";
	cout << "[options]\n";
	cout << "--unit=file(phonetic_dic for unit word)\n";
	cout << "-n, --letter_order=int(default 10)\n";
	cout << "-m, --phonetic_order=int(default 3)\n";
	cout << "-e, --epoch=int(default 100)\n";
	cout << "-t, --threads=int(default 4)\n";
	cout << "-v, --vocab=int(means letter variations. default 5000)\n";
	cout << "-s, --supervised=file(labeled_data in kkci format)\n";
	exit(1);
}

int read_long_param(const char *opt, const char *arg) {
	if (check(opt, "train"))
		train = arg;
	else if (check(opt, "parse"))
		test = arg;
	else if (check(opt, "model"))
		model = arg;
	else if (check(opt, "dic"))
		wdic = arg;
	else if (check(opt, "trie"))
		triedic = arg;
	else if (check(opt, "unit"))
		unitdic = arg;
	else if (check(opt, "supervised"))
		supervised = arg;
	else if (check(opt, "letter_order"))
		n = atoi(arg);
	else if (check(opt, "phonetic_order"))
		m = atoi(arg);
	else if (check(opt, "epoch"))
		epoch = atoi(arg);
	else if (check(opt, "threads"))
		threads = atoi(arg);
	else if (check(opt, "dump"))
		dmp = atoi(arg);
	else if (check(opt, "vocab"))
		vocab = atoi(arg);
	else
		return 1;
	return 0;
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
			{"trie", required_argument, 0, 0},
			{"unit", required_argument, 0, 0},
			{"supervised", required_argument, 0, 0},
			{"model", required_argument, 0, 0},
			{"letter_order", required_argument, 0, 0},
			{"phonetic_order", required_argument, 0, 0},
			{"epoch", required_argument, 0, 0},
			{"threads", required_argument, 0, 0},
			{"dump", required_argument, 0, 0},
			{"vocab", required_argument, 0, 0},
			{0, 0, 0, 0}
		};
		int option_index = 0;
		c = getopt_long(argc, argv, "n:m:e:t:v:s:", long_options, &option_index);
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
			case 's':
				supervised = optarg;
				break;
			case'?':
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

void dump(vector<pair<word, vector<unsigned int> > >& s) {
	for (auto& i : s) {
		word& w = i.first;
		for (auto j = 0; j < w.len; ++j) {
			char buf[5] = {0};
			io::i2c(w[j], buf);
			cout << buf;
		}
		cout << "/";
		for (auto& j : i.second) {
			char buf[5] = {0};
			io::i2c(j, buf);
			cout << buf;
		}
		cout << " ";
	}
	cout << endl;
}

int pretrain(hsmm& lm, io& f) {
	//io f(supervised.c_str());
	int size = f.head.size()-1;
	vector<vector<pair<word, vector<unsigned int> > > > corpus(size);
	const char *delim = "/";
	unsigned int d = io::c2i(delim, io::u8size(delim));
	unsigned int s = 32; // mean ' '
	cout << "pretraining..." << endl;
	for (auto i = 0; i < size; ++i) {
		int h = f.head[i];
		int p = util::find(s, *f.raw, f.head[i], f.head[i+1]);
		do {
			int split = util::rfind(d, *f.raw, h, p);
			word w(*f.raw, h, split-h);
			vector<unsigned int> read;
			for (auto j = split+1; j < p; ++j) {
				read.emplace_back((*f.raw)[j]);
			}
			corpus[i].emplace_back(make_pair(w, read));
			h = p+1;
			p = util::find(s, *f.raw, p+1, f.head[i+1]);
		} while (p < f.head[i+1]);
	}
	for (auto i = 0; i < epoch; ++i) {
		vector<int> *rd = new vector<int>(corpus.size(), 0);
		//int rd[size] = {0};
		rd::shuffle(rd->data(), corpus.size());
		for (auto j = 0; j < (int)corpus.size(); ++j) {
			//dump(corpus[(*rd)[j]]);
			if (i > 0)
				lm.remove(corpus[(*rd)[j]]);
			lm.add(corpus[(*rd)[j]]);
			progress(i, (double)(min(j+1,(int)corpus.size()))/corpus.size());
		}
		delete rd;
		lm.estimate(1);
		lm.poisson_correction(1000);
	}
	return 0;
}

int mcmc() {
	io f(train.c_str());
	int size = f.head.size()-1;
	//vector<vector<pair<word, vector<unsigned int> > > > corpus(size);
	//vector<vector<pair<word, vector<unsigned int> > > > corpus(size);
	vector<vector<pair<word, vector<unsigned int> > > > corpus;
	hsmm lm(n, m, triedic.c_str(), (unitdic.empty()?NULL:unitdic.c_str()));
	//hsmm lm(n, m, triedic.c_str(), (unkdic.empty()? NULL: unkdic.c_str()));
	lm.set(vocab);
	io *g = NULL;
	if (!supervised.empty()) {
		g = new io(supervised.c_str());
		pretrain(lm, *g);
	}
	lm.init(f, corpus);
	lm.estimate(1);
#ifdef _OPENMP
	threads = min(omp_get_max_threads(), threads);
	omp_set_num_threads(threads);
#endif
	cout << "training..." << endl;
	for (auto i = 0; i < epoch; ++i) {
		vector<int> *rd = new vector<int>(size, 0);
		//int rd[size] = {0};
		rd::shuffle(rd->data(), size);
		int j = 0;
		while (j < size) {
			for (auto t = 0; t < threads; ++t) {
				if (j+t < (int)size) {
					//lm.remove(corpus[rd[j+t]]);
					lm.remove(corpus[(*rd)[j+t]]);
				}
			}
#ifdef _OPENMP
#pragma omp parallel
			{
				auto t = omp_get_thread_num();
				if (j+t < size) {
					try {
						//corpus[rd[j+t]] = lm.sample(f, rd[j+t]);
						corpus[(*rd)[j+t]] = lm.sample(f, (*rd)[j+t]);
					} catch (const char *ex) {
						cerr << ex << endl;
					}
				}
			}
#else
			for (auto t = 0; t < threads; ++t) {
				if (j+t < size) {
					try {
						//corpus[rd[j+t]] = lm.sample(f, rd[j+t]);
						corpus[(*rd)[j+t]] = lm.sample(f, (*rd)[j+t]);
					} catch (const char *ex) {
						cerr << ex << endl;
					}
				}
			}
#endif
			// add
			for (auto t = 0; t < threads; ++t) {
				if (j+t < size) {
					//lm.add(corpus[rd[j+t]]);
					lm.add(corpus[(*rd)[j+t]]);
				}
			}
			j += threads;
			progress(i, (double)(min(j+1,size))/size);
		}
		lm.estimate(1);
		lm.poisson_correction(1000);
		if (dmp && (i+1)%dmp == 0) {
			cout << endl;
			for (auto& s : corpus) {
				dump(s);
			}
		}
		delete rd;
	}
	cout << endl;
	lm.save(model.c_str());
	shared_ptr<wid> d = wid::create();
	d->save(wdic.c_str());
	if (g)
		delete g;
	return 0;
}

int parse() {
	io f(test.c_str());
	shared_ptr<wid> d = wid::create();
	d->load(wdic.c_str());
	//hsmm lm(n, m, triedic.c_str(), (unkdic.empty()? NULL: unkdic.c_str()));
	hsmm lm(n, m, triedic.c_str(), (unitdic.empty()?NULL:unitdic.c_str()));
	lm.load(model.c_str());
#ifdef _OPENMP
	threads = min(omp_get_max_threads(), threads);
	omp_set_num_threads(threads);
#pragma omp parallel for ordered schedule(dynamic)
#endif
	for (auto i = 0; i < (int)f.head.size()-1; ++i) {
		vector<pair<word, vector<unsigned int> > > s = lm.parse(f, i);
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
		if (!train.empty())
			mcmc();
		if (!test.empty())
			parse();
	} catch (const char *ex) {
		cerr << ex << endl;
		return 1;
	}
	return 0;
}

