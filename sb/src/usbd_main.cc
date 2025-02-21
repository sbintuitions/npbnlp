#include"usbd.h"
#include"util.h"
#include"rd.h"
#include<getopt.h>
#include<cstdlib>
#include<cstdio>
#ifdef _OPENMP
#include<omp.h>
#endif

#define check(opt,arg) (strcmp(opt,arg) == 0)
#define PBSTR "||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||"
#define PBWIDTH 60

using namespace std;
using namespace npbnlp;

static int n = 5;
static int epoch = 100;
static int threads = 4;
static double a = 1;
static double b = 1;
static double c = 9;
static double d = 1;
static double e = 9;
static double f = 1;
static string pretrain;
static string train;
static string test;
static string valid;
static string model("usbd.model");
static string dic("usbd.dic");
static string punc("。！？");
static double default_prior = 0.1;
static double cr_prior = 0.9;
static double pnc_prior = 0.9;
static sequence_type type = sequence_type::letter;
static smoothing lm = smoothing::hpy;
static int dmp = 0;
static int f_set_default_prior = 0;
static int f_set_cr_prior = 0;
static int f_set_punc_prior = 0;

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
	cout << *argv << " --train file --model file_to_save\n";
	cout << *argv << " --parse file --model trained_model_file\n";
	cout << "[options]\n";
	cout << "--validation =file(use as validation dataset in training\n";
	cout << "--pretrain =file(use as pretraining dataset in training\n";
	cout << "--sequence_type token_type(letter or word, default:letter)\n";
	cout << "--smoothing lm_type(kn or hpy, default:hpy)\n";
	cout << "--epoch =int(default:" << epoch << ")\n";
	cout << "--threads =int(default:" << threads << ")\n";
	cout << "--punc =string(default:" << punc << ")\n";
	cout << "--default_prior =float(default prior, default:" << default_prior << ")\n";
	cout << "--cr_prior =float(cr prior, default:" << cr_prior << ")\n";
	cout << "--punc_prior =float(punc prior, default:" << pnc_prior << ")\n";
	cout << "-n =int n-gram order(default " << n << ")\n";
	cout << "-a hyperparameter for bernoulli distribution for general prior(default " << a << ")\n";
	cout << "-b hyperparameter for bernoulli distribution for general prior(default " << b << ")\n";
	cout << "-c hyperparameter for bernoulli distribution for cr prior(default " << c << ")\n";
	cout << "-d hyperparameter for bernoulli distribution for cr prior(default " << d << ")\n";
	cout << "-e hyperparameter for bernoulli distribution for punc prior(default " << e << ")\n";
	cout << "-f hyperparameter for bernoulli distribution for punc prior(default " << f << ")\n";
	exit(1);
}

int read_long_param(const char *opt, const char *arg) {
	if (check(opt, "train")) {
		train = arg;
		return 0;
	} else if (check(opt, "parse")) {
		test = arg;
		return 0;
	} else if (check(opt, "validation")) {
		valid = arg;
		return 0;
	} else if (check(opt, "pretrain")) {
		pretrain = arg;
		return 0;
	} else if (check(opt, "model")) {
		model = arg;
		return 0;
	} else if (check(opt, "threads")) {
		threads = atoi(arg);
		return 0;
	} else if (check(opt, "epoch")) {
		epoch = atoi(arg);
		return 0;
	} else if (check(opt, "punctuation")) {
		punc = arg;
		return 0;
	} else if (check(opt, "sequence_type")) {
		if (check(arg, "letter")) {
			type = sequence_type::letter;
		} else if (check(arg, "word")) {
			type = sequence_type::word;
		} else {
			return 1;
		}
		return 0;
	} else if (check(opt, "smoothing")) {
		if (check(arg, "kn")) {
			lm = smoothing::kn;
		} else if (check(arg, "hpy")) {
			lm = smoothing::hpy;
		} else {
			return 1;
		}
		return 0;
	} else if (check(opt, "dump")) {
		dmp = atoi(arg);
		return 0;
	} else if (check(opt, "default_prior")) {
		default_prior = atof(arg);
		f_set_default_prior = 1;
		return 0;
	} else if (check(opt, "cr_prior")) {
		cr_prior = atof(arg);
		f_set_cr_prior = 1;
		return 0;
	} else if (check(opt, "punc_prior")) {
		pnc_prior = atof(arg);
		f_set_punc_prior = 1;
		return 0;
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
	int g;
	while (1) {
		static struct option long_options[] = {
			{"train", required_argument, 0, 0},
			{"parse", required_argument, 0, 0},
			{"pretrain", required_argument, 0, 0},
			{"validation", required_argument, 0, 0},
			{"model", required_argument, 0, 0},
			{"threads", required_argument, 0, 0},
			{"epoch", required_argument, 0, 0},
			{"punctuation", required_argument, 0, 0},
			{"sequence_type", required_argument, 0, 0},
			{"smoothing", required_argument, 0, 0},
			{"dump", required_argument, 0, 0},
			{"default_prior", required_argument, 0, 0},
			{"cr_prior", required_argument, 0, 0},
			{"punc_prior", required_argument, 0, 0},
			{0, 0, 0, 0}
		};
		int option_index = 0;
		g = getopt_long(argc, argv, "n:a:b:c:d:e:f:", long_options, &option_index);
		if (g == -1)
			break;
		switch (g) {
			case 0:
				if (long_options[option_index].flag != 0)
					break;
				read_long_param(long_options[option_index].name, optarg);
				break;
			case 'n':
				n = atoi(optarg);
				break;
			case 'a':
				a = atof(optarg);
				break;
			case 'b':
				b = atof(optarg);
				break;
			case 'c':
				c = atof(optarg);
				break;
			case 'd':
				d = atof(optarg);
				break;
			case 'e':
				e = atof(optarg);
				break;
			case 'f':
				f = atof(optarg);
				break;
			case '?':
			default:
				usage(argc, argv);

		}
	}
	if (optind < argc) {
		cerr << "non-option argv-elements: ";
		while (optind < argc) {
			cerr << argv[optind++] << " ";
		}
		cerr << endl;
		usage(argc, argv);
		return 1;
	}
	return 0;
}

void delete_nl(io& f) {
	if (type == sequence_type::word)
		return;
	vector<int> head;
	head.emplace_back(f.head[0]);
	head.emplace_back(f.head[f.head.size()-1]);
	f.head = move(head);
}

void dump_word(io& f, vector<int>& head) {
	sentence s;
	for (int i = 0; i < (int)f.head.size()-1; ++i) {
		int h = f.head[i];
		int t = f.head[i+1];
		sentence ss;
		ss.init_without_indexing(*f.raw, h, t);
		s.cat(ss);
	}
	int id = 0;
	for (int i = 0; i < (int)head.size()-1; ++i) {
		int t = head[i+1];
		for (; id < s.size() && s.wd(id).head < t; ++id) {
			word& w = s.wd(id);
			for (int j = 0; j < w.len; ++j) {
				char buf[5] = {0};
				io::i2c(w[j], buf);
				cout << buf;
			}
			if (id < s.size()-1 && s.wd(id+1).head < t)
				cout << " ";
		}
		cout << endl;
	}
	cout << endl;
}

void dump_letter(io& f, vector<int>& head) {
	for (auto i = 0; i < (int)head.size()-1; ++i) {
		int h = head[i];
		int t = head[i+1];
		for (auto j = h; j < t; ++j) {
			char buf[5] = {0};
			io::i2c((*f.raw)[j], buf);
			cout << buf;
		}
		cout << endl;
	}
	cout << endl;
}

void dump(cio& f, vector<vector<int> >& boundaries) {
	for (auto i = 0; i < (int)f.chunk->size(); ++i) {
		if (type == sequence_type::word) {
			dump_word((*f.chunk)[i], boundaries[i]);
		} else {
			dump_letter((*f.chunk)[i], boundaries[i]);
		}
	}
}

void mcmc() {
	cio file(train.c_str());
	bd_wrap bd;
	if (!bd.create(n, type)) {
		throw "failed to create detector";
	}
	bd.set_hyper_general(a,b);
	bd.set_hyper_cr(c,d);
	bd.set_hyper_punc(e,f);
	if (!punc.empty()) {
		vector<unsigned int> p;
		io::s2i(punc.c_str(), p);
		for (auto& i : p)
			bd.set_punc(i);
	}
	if (f_set_default_prior)
		bd.set_general_prior(default_prior);
	if (f_set_cr_prior)
		bd.set_cr_prior(cr_prior);
	if (f_set_punc_prior)
		bd.set_punc_prior(pnc_prior);
	io *pre = NULL;
	if (!pretrain.empty()) {
		pre = new io(pretrain.c_str());
		bd.pretrain(*pre);
	}
	cio *val = NULL;
	if (!valid.empty()) {
		val = new cio(valid.c_str());
	}
	double score = 0;

#ifdef _OPENMP
	omp_set_num_threads(threads);
#endif
	vector<vector<int> > boundaries(file.chunk->size());
	vector<double> criteria;

#ifdef _OPENMP
	for (auto i = 0; i < epoch; ++i) {
		int size = file.chunk->size();
		int rd[size] = {0};
		rd::shuffle(rd, size);
		int j = 0;
		while (j < size) {
			if (i > 0) {
				for (auto t = 0; t < threads; ++t) {
					if (j+t < size)
						bd.remove((*file.chunk)[rd[j+t]], boundaries[rd[j+t]]);
				}
			}
#pragma omp parallel
			{
				auto t = omp_get_thread_num();
				if (j+t < size) {
					vector<int> head;
					bd.sample((*file.chunk)[rd[j+t]], head);
					boundaries[rd[j+t]] = move(head);
				}
			}
			for (auto t = 0; t < threads; ++t) {
				if (j+t < size)
					bd.add((*file.chunk)[rd[j+t]], boundaries[rd[j+t]]);
			}
			j += threads;
#pragma omp ordered
			progress(i, (double)(min(j+1,size))/size);
		}
		bd.estimate_lm_hyper(20);
		bd.estimate_prior(file, boundaries);
		if (!valid.empty()) {
			cio t(valid.c_str());
#pragma omp parallel for
			for (auto k = 0; k < (int)t.chunk->size(); ++k) {
				if (type == sequence_type::word) {
					delete_nl((*t.chunk)[k]);
				}
				vector<int> c;
				bd.parse((*t.chunk)[k], c);
				(*t.chunk)[k].head = move(c);
			}
			bd.eval(t, *val, criteria);
			printf("\nepoch:%04d\tprec:%f\trec:%f\tf1:%f\n",i,criteria[0],criteria[1],criteria[2]);
			if (criteria[2] > score) {
				score = criteria[2];
				char snapshot[512] = {0};
				sprintf(snapshot, "epoch%03d_a%fb%fc%fd%fe%ff%f.model",i,a,b,c,d,e,f);
				bd.save(snapshot);
			}
		}
		if (i%dmp == 0)
			dump(file, boundaries);
	}
#else
	for (auto i = 0; i < epoch; ++i) {
		int size = file.chunk->size();
		int rd[size] = {0};
		rd::shuffle(rd, size);
		for (auto j = 0; j < size; ++j) {
			if (i > 0) {
				bd.remove((*file.chunk)[rd[j]], boundaries[rd[j]]);
			}
			vector<int> head;
			bd.sample((*file.chunk)[rd[j]], head);
			bd.add((*file.chunk)[rd[j]], head);

			boundaries[rd[j]] = move(head);
			progress(i, (double)(min(j+1,size))/size);
		}
		bd.estimate_lm_hyper(20);
		bd.estimate_prior(file, boundaries);
		if (!valid.empty()) {
			cio t(valid.c_str());
			for (auto k = 0; k < (int)t.chunk->size(); ++k) {
				if (type == sequence_type::word) {
					delete_nl((*t.chunk)[k]);
				}
				vector<int> c;
				bd.parse((*t.chunk)[k], c);
				(*t.chunk)[k].head = move(c);
			}
			bd.eval(t, *val, criteria);
			printf("\nepoch:%04d\tprec:%f\trec:%f\tf1:%f\n",i,criteria[0],criteria[1],criteria[2]);
			if (criteria[2] > score) {
				score = criteria[2];
				char snapshot[512] = {0};
				sprintf(snapshot, "epoch%03d_a%fb%fc%fd%fe%ff%f.model",i,a,b,c,d,e,f);
				bd.save(snapshot);
			}
		}
		if (i%dmp == 0)
			dump(file, boundaries);
	}
#endif

	bd.save(model.c_str());
	if (type == sequence_type::word) {
		shared_ptr<wid> d = wid::create();
		d->save(dic.c_str());
	}
	if (pre)
		delete pre;
	if (val)
		delete val;
}

void parse() {
	cio f(test.c_str());
	bd_wrap bd;
	if (!bd.create(n, type)) {
		throw "failed to create detector";
	}
	bd.load(model.c_str());
	if (f_set_default_prior)
		bd.set_general_prior(default_prior);
	if (f_set_cr_prior)
		bd.set_cr_prior(cr_prior);
	if (f_set_punc_prior)
		bd.set_punc_prior(pnc_prior);
	if (type == sequence_type::word) {
		shared_ptr<wid> d = wid::create();
		d->load(dic.c_str());
	}
#ifdef _OPENMP
	omp_set_num_threads(threads);
#pragma omp parallel for ordered schedule(dynamic)
#endif
	for (auto i = 0; i < (int)f.chunk->size(); ++i) {
		vector<int> c;
		bd.parse((*f.chunk)[i], c);
#ifdef _OPENMP
#pragma omp ordered
#endif
		if (type == sequence_type::word) {
			dump_word((*f.chunk)[i], c);
		} else {
			dump_letter((*f.chunk)[i], c);
		}
	}
}

int main(int argc, char **argv) {
	try {
		if (read_param(argc, argv))
			return 1;
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
