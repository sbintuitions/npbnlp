#include"ipcfg.h"
#include"rd.h"
#include"util.h"
#include<cstdio>
#include<cstring>
#include<cstdlib>
#include<getopt.h>
#ifdef _OPENMP
#include<omp.h>
#endif

#define check(opt,arg) (strcmp(opt,arg) == 0)
#define PBSTR "||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||"
#define PBWIDTH 60
#define NPYLM_EPOCH 100

using namespace std;
using namespace npbnlp;

static int m = 20;
static int k = 20;
static int K = 100; // base measure for transition
static int threads = 4;
static int epoch = 500;
static int dmp = 0;
static int vocab = 50000;
static double a = 1;
static double b = 1;
static int dot = 0;
static string train;
static string test;
static string model("ipcfg.model");
static string dic("pa.dic");
static int node_id = 0;

class dot_node {
	public:
		int id;
		int k;
		int left;
		int right;
		string label;
};

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
	cout << *argv << " --train file --model file_to_save --dic dicfile\n";
	cout << *argv << " --parse file --model modelfile --dic dicfile\n";
	cout << "[options]\n";
	cout << "-m, --letter_order=int(default 20)\n";
	cout << "-k, --max_category=int(default 100)\n";
	cout << "-e, --epoch=int(default 500)\n";
	cout << "-t, --threads=int(default 4)\n";
	cout << "-v, --vocab=int(means letter variations. default 5000)\n";
	cout << "--dot=flag output in dot format for graphviz" << endl;
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
	} else if (check(opt, "letter_order")) {
		m = atoi(arg);
	} else if (check(opt, "max_category")) {
		K = atoi(arg);
		k = min(k, K);
	} else if (check(opt, "epoch")) {
		epoch = atoi(arg);
	} else if (check(opt, "threads")) {
		threads = atoi(arg);
	} else if (check(opt, "dump")) {
		dmp = atoi(arg);
	} else if (check(opt, "dot")) {
		dot = 1;
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
			{"letter_order", required_argument, 0, 0},
			{"max_category", required_argument, 0, 0},
			{"epoch", required_argument, 0, 0},
			{"threads", required_argument, 0, 0},
			{"dump", required_argument, 0, 0},
			{"vocab", required_argument, 0, 0},
			// flag option
			{"dot", no_argument, &dot, 1},
			{0, 0, 0, 0}
		};
		int option_index = 0;
		c = getopt_long(argc, argv, "m:k:e:t:v:", long_options, &option_index);
		if (c == -1)
			break;
		switch (c) {
			case 0:
				if (long_options[option_index].flag != 0)
					break;
				read_long_param(long_options[option_index].name, optarg);
				break;
			case 'm':
				m = atoi(optarg);
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

void dump_node(tree& t, int i) {
	node& c = t[i];
	if (c.i != c.j) {
		cout << "(";
		int left = t.s.size()*c.i+c.b-c.i*(1.+c.i)/2;
		int right = t.s.size()*(c.b+1)+c.j-(1.+c.b)*(2+c.b)/2;
		cout << c.k << " ";
		dump_node(t, left);
		dump_node(t, right);
		cout << ")";
	} else if (c.k > 0 && c.i == c.j) {
		cout << "(" << c.k << " " << t.wd(c.i) << ")";
	}
}

void tree_node(tree& t, int i, vector<dot_node>& n) {
	node& c = t[i];
	if (c.i != c.j) {
		int left = t.s.size()*c.i+c.b-c.i*(1.+c.i)/2;
		int right = t.s.size()*(c.b+1)+c.j-(1.+c.b)*(2+c.b)/2;
		dot_node nd;
		nd.id = i;
		nd.k = c.k;
		nd.left = left;
		nd.right = right;
		char buf[1024] = {0};
		sprintf(buf, "%d", c.k);
		nd.label = buf;
		n.push_back(nd);
		tree_node(t, left, n);
		tree_node(t, right, n);
	} else if (c.k > 0 && c.i == c.j) {
		dot_node nd;
		nd.id = i;
		nd.k = c.k;
		nd.left = -1;
		nd.right = -1;
		word& w = t.wd(c.i);
		char k[1024] = {0};
		sprintf(k, "%d:", c.k);
		nd.label = k;
		for (auto i = 0; i < w.len; ++i) {
			char buf[5] = {0};
			io::i2c(w[i], buf);
			nd.label += buf;
		}
		n.push_back(nd);
	}
}

void dump_dot(tree& t, int n) {
	string str;
	for (auto i = 0; i < t.s.size(); ++i) {
		word& w = t.s.wd(i);
		for (auto j = 0; j < w.len; ++j) {
			char buf[5] = {0};
			io::i2c(w[j], buf);
			if (strcmp(buf, "#") == 0)
				str += "\\";
			str += buf;
		}
	}
	vector<dot_node> nodes;
	//cout << "digraph {" << endl;
	//cout << "node [fontname=IPAPGothic]" << endl;
	//cout << "edge [fontname=IPAPGothic]" << endl;
	cout << "subgraph cluster_" << n << "{" << endl;
	cout << "label=" << str << endl;
	tree_node(t, t.s.size()-1, nodes);
	// label
	for (auto i = nodes.begin(); i != nodes.end(); ++i) {
		cout << "n" << n << "_" << i->id << " [label=\"" << i->label << "\"]" << endl;
	}
	// edge
	for (auto i = nodes.begin(); i != nodes.end(); ++i) {
		if (i->left >= 0)
			cout << "n" << n << "_" << i->id << " -> " << "n" << n << "_" << i->left << endl;
		if (i->right >= 0)
			cout << "n" << n << "_" << i->id << " -> " << "n" << n << "_" << i->right << endl;
	}
	cout << "}" << endl;
	//cout << "}" << endl;
}

void dump(tree& t, int n) {
	if (dot) {
		dump_dot(t, n);
	} else {
		dump_node(t, t.s.size()-1);
		cout << endl;
	}
}

void dump_all(vector<tree>& corpus) {
	if (dot) {
		cout << "digraph {" << endl;
		cout << "node [fontname=IPAPGothic]" << endl;
		cout << "edge [fontname=IPAPGothic]" << endl;
	}
	for (auto i = 0; i < corpus.size(); ++i) {
		dump(corpus[i], i);
	}
	if (dot)
		cout << "}" << endl;
}

int mcmc() {
	io f(train.c_str());
	vector<tree> corpus;
	corpus.resize(f.head.size()-1);
	ipcfg g(m);
	g.set(vocab, K);
	g.slice(a, b);
#ifdef _OPENMP
	omp_set_num_threads(threads);
#endif
	for (auto i = 0; i < epoch; ++i) {
		int rd[corpus.size()] = {0};
		rd::shuffle(rd, corpus.size());
		int j = 0;
		while (j < corpus.size()) {
			// remove
			if (i > 0) {
				for (auto t = 0; t < threads; ++t) {
					if (j+t < corpus.size()) {
						g.remove(corpus[rd[j+t]]);
					}
				}
			}
#ifdef _OPENMP
#pragma omp parallel
			{ // sample segmentations
				auto t = omp_get_thread_num();
				if (j+t < corpus.size()) {
					try {
						tree tr = g.sample(f, rd[j+t]);
						corpus[rd[j+t]] = tr;
					} catch (const char *ex) {
						cerr << ex << endl;
					}
				}
			}
#else
			for (auto t = 0; t < threads; ++t) {
				if (j+t < corpus.size()) {
					try {
						tree tr = g.sample(f, rd[j+t]);
						corpus[rd[j+t]] = tr;
					} catch (const char *ex) {
						cerr << ex << endl;
					}
				}
			}
#endif
			// add
			for (auto t = 0; t < threads; ++t) {
				if (j+t < corpus.size()) {
					g.add(corpus[rd[j+t]]);
				}
			}
			j += threads;
#ifdef _OPENMP
#pragma omp ordered
#endif
			progress("epoch", i, (double)(j+1)/corpus.size());
		}
		// estimate hyperparameter
		g.estimate(20);
		g.poisson_correction(1000);
		if (dmp && (i+1)%dmp == 0) {
			cout << endl;
			//for (auto s = corpus.begin(); s != corpus.end(); ++s)
			/*
			for (auto s = 0; s < corpus.size(); ++s)
				dump(corpus[s], s);
				*/
			dump_all(corpus);
		}
	}
	cout << endl;
	g.save(model.c_str());
	shared_ptr<wid> d = wid::create();
	d->save(dic.c_str());
	return 0;
}

int parse() {
	io f(test.c_str());
	shared_ptr<wid> d = wid::create();
	d->load(dic.c_str());
	ipcfg g(m);
	try {
		g.load(model.c_str());
		g.set(vocab, K);
	} catch (const char *ex) {
		throw ex;
	}
	if (dot) {
		cout << "digraph {" << endl;
		cout << "node [fontname=IPAPGothic]" << endl;
		cout << "edge [fontname=IPAPGothic]" << endl;
	}
#ifdef _OPENMP
#pragma omp parallel for ordered schedule(dynamic)
#endif
	for (auto i = 0; i < f.head.size()-1; ++i) {
		tree t = g.parse(f, i);
#ifdef _OPENMP
#pragma omp ordered
#endif
		dump(t, i);
	}
	if (dot)
		cout << "}" << endl;
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
