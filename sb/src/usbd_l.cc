#include"usbd_l.h"
#include"kn.h"
#include"hpy.h"
#include"beta.h"
#include"rd.h"
#include<random>
#define BUFSIZE 256

using namespace std;
using namespace npbnlp;

usbd_l::usbd_l():usbd_l(3, smoothing::hpy) {
}

usbd_l::usbd_l(int n, smoothing s):_n(n),_lm(nullptr), _A(1),_B(1),_C(9),_D(1),_E(9),_F(1) {
	switch (s) {
		case smoothing::kn:
			_lm = shared_ptr<dalm>(new kn(_n));
			break;
		case smoothing::hpy:
			_lm = shared_ptr<dalm>(new hpy(_n));
			break;
		default:
			break;
	}
	beta_distribution be;
	_general_prior = be(_A, _B);
	_cr_prior = be(_C, _D);
	_pnc_prior = be(_E, _F);
}

usbd_l::~usbd_l() {
}

usbd_l::usbd_l(const usbd_l& d) {
	_n = d._n;
	_lm = d._lm;
	_A = d._A;
	_B = d._B;
	_C = d._C;
	_D = d._D;
	_E = d._E;
	_F = d._F;
	_punc.clear();
	for (auto& i : d._punc) {
		_punc.emplace_back(i);
	}
	_general_prior = d._general_prior;
	_cr_prior = d._cr_prior;
	_pnc_prior = d._pnc_prior;
}

usbd_l& usbd_l::operator=(const usbd_l& d) {
	_n = d._n;
	_lm = d._lm;
	_A = d._A;
	_B = d._B;
	_C = d._C;
	_D = d._D;
	_E = d._E;
	_F = d._F;
	_punc.clear();
	for (auto& i : d._punc) {
		_punc.emplace_back(i);
	}
	_general_prior = d._general_prior;
	_cr_prior = d._cr_prior;
	_pnc_prior = d._pnc_prior;
	return *this;
}

void usbd_l::set_punc(unsigned int c) {
	if (find(_punc.begin(),_punc.end(),c) == _punc.end())
		_punc.emplace_back(c);
}


void usbd_l::set_prior(double g, double c, double p) {
	if (g > 0)
		_general_prior = g;
	if (c > 0)
		_cr_prior = c;
	if (p > 0)
		_pnc_prior = p;
}

void usbd_l::set_general_prior(double p) {
	if (p > 0)
		_general_prior = p;
}

void usbd_l::set_cr_prior(double p) {
	if (p > 0)
		_cr_prior = p;
}

void usbd_l::set_punc_prior(double p) {
	if (p > 0)
		_pnc_prior = p;
}

void usbd_l::set_hyper_general(double a, double b) {
	if (a >= 0 && b >= 0) {
		_A = a;
		_B = b;
	}
	beta_distribution be;
	_general_prior = be(_A, _B);
}

void usbd_l::set_hyper_cr(double c, double d) {
	if (c >= 0 && d >= 0) {
		_C = c;
		_D = d;
	}
	beta_distribution be;
	_general_prior = be(_C, _D);
}

void usbd_l::set_hyper_punc(double e, double f) {
	if (e >= 0 && f >= 0) {
		_E = e;
		_F = f;
	}
	beta_distribution be;
	_general_prior = be(_E, _F);
}

void usbd_l::estimate_lm_hyper(int iter) {
	_lm->estimate(iter);
}

void usbd_l::estimate_prior(cio& corpus, vector<vector<int> >& boundaries) {
	_estimate_gen_cr_prior(corpus, boundaries);
	// for punctuation
	_estimate_punc_prior(corpus, boundaries);
}

void usbd_l::add(io& d, vector<int>& head) {
	int size = head.size()-1;
	int rd[size] = {0};
	rd::shuffle(rd, size);
	for (auto i = 0; i < size; ++i) {
		int h = head[rd[i]];
		int t = head[rd[i]+1];
		word w(*d.raw, h, t-h);
		_lm->add(w);
	}
}

void usbd_l::remove(io& d, vector<int>& head) {
	int size = head.size()-1;
	for (auto i = 0; i < size; ++i) {
		int h = head[i];
		int t = head[i+1];
		word w(*d.raw, h, t-h);
		_lm->remove(w);
	}
}

void usbd_l::_estimate_gen_cr_prior(cio& corpus, vector<vector<int> >& boundaries) {
	double a = 0, b = 0, c = 0, d = 0;
	for (auto i : boundaries) {
		a += i.size()-1;
	}
	for (auto i = 0; i < (int)corpus.chunk->size(); ++i) {
		io& doc = (*corpus.chunk)[i];
		b += doc.raw->size();
		int match = 0;
		int j = 0, k = 0;
		while (1) {
			if (doc.head[j] == boundaries[i][k]) {
				++j, ++k, ++match;
			} else if (doc.head[j] < boundaries[i][k]) {
				++j;
			} else if (doc.head[j] > boundaries[i][k]) {
				++k;
			}
			if (j >= (int)doc.head.size() && k >= (int)boundaries[i].size())
				break;
		}
		c += match - 1; // discount match at bos
		d += doc.head.size() - 1; // discount head[0]
	}
	b -= a;
	d -= c;
	beta_distribution be;
	_general_prior = be(_A+a, _B+b);
	_cr_prior = be(_C+c, _D+d);
}

void usbd_l::_estimate_punc_prior(cio& corpus, vector<vector<int> >& boundaries) {
	double e = 0, f = 0;
	for (auto i = 0; i < (int)corpus.chunk->size(); ++i) {
		io& doc = (*corpus.chunk)[i];
		int k = 0;
		int head = boundaries[i][k];
		int match = 0;
		int count = 0;
		for (auto j = 0; j < (int)doc.raw->size(); ++j) {
			auto it = find(_punc.begin(), _punc.end(), (*doc.raw)[j]);
			if (it == _punc.end()) {
				continue;
			}
			if (j+1 == head) {
				++match;
				head = boundaries[i][++k];
			} else {
				++count;
				if (j >= head)
					head = boundaries[i][++k];
			}
		}
		e += match;
		f += count;
	}
	beta_distribution be;
	_pnc_prior = be(_E+e, _F+f);
}

void usbd_l::pretrain(io& f, int iter) {
	int size = f.head.size() - 1;
	for (auto i = 0; i < iter; ++i) {
		int rd[size] = {0};
		rd::shuffle(rd, size);
		for (auto j = 0; j < size; ++j) {
			int head = f.head[rd[j]];
			int tail = f.head[rd[j]+1];
			word w(*f.raw, head, tail-head);
			if (i > 0)
				_lm->remove(w);
			_lm->add(w);
		}
		_lm->estimate(20);
	}
}

void usbd_l::init(cio& c, int n) {
	return;
}

void usbd_l::sample(io& d, vector<int>& b) {
	if (d.raw->size() > BUFSIZE) {
		auto i = d.head[0];
		while (i < (int)d.raw->size()) {
			io g;
			g.head.clear();
			g.raw = d.raw;
			g.head.emplace_back(i);
			for (auto j = 0; j < (int)d.head.size() && d.head[j] < i+BUFSIZE; ++j) {
				if (d.head[j] > i)
					g.head.emplace_back(d.head[j]);
			}
			if (g.head[g.head.size()-1] != d.head[d.head.size()-1]) {
				g.head.emplace_back(min((int)d.raw->size(), i+BUFSIZE));
			}
			vector<int> c;
			_sample(g, c);
			if (i == 0) {
				for (auto& j : c) {
					b.emplace_back(j);
				}
			} else {
				if (!b.empty())
					b.pop_back();
				for (auto k = 1; k < (int)c.size(); ++k) {
					b.emplace_back(c[k]);
				}
			}
			if (c.size() > 2) {
				i = c[c.size()-2];
			} else {
				//i = min((int)d.raw->size(), i+BUFSIZE/2);
				i = min((int)d.raw->size(), i+BUFSIZE-_n);
			}
		}
	} else {
		_sample(d, b);
	}
}

void usbd_l::_sample(io& f, vector<int>& b) {
	vt dp;
	vector<double> cum;
	vector<vector<double> > bos;
	vector<vector<double> > eos;
	_cumurative(f, cum);
	_bos(f, bos);
	_eos(f, eos);
	int size = f.head.size()-1;
	int head = f.head[0];
	int tail = f.head[size];
	int len = tail-head;
	unordered_map<int, double> alpha;
	for (int t = 0; t < len; ++t) {
		double a = 0;
		for (int l = 0; l < t; ++l) {
			a = math::lse(a, dp[t-1][l].v, (l==0));
		}
		alpha[t-1] = a;
		for (int j = 1; j <= t+1; ++j) {
			int b = min(j-1, _n-2);
			int e = min(j, _n-1);
			double lp_bos = bos[t-j+1][b];
			double lp_eos = eos[t+1][e];
			double lp = cum[t]-cum[t-j+b+1]+lp_bos+lp_eos;
			dp[t][j-1].v = lp+alpha[t-j];
		}
	}
	b.emplace_back(tail);
	int t = len-1;
	while (t >= 0) {
		vector<double> table;
		for (int k = 0; k <= t; ++k) {
			table.emplace_back(dp[t][k].v);
		}
		int l = 1+rd::ln_draw(table);
		t -= l;
		if (t > 0)
			b.emplace_back(head+t+1);
	}
	b.emplace_back(head);
	reverse(b.begin(), b.end());
}

void usbd_l::parse(io& d, vector<int>& b) {
	if (d.raw->size() > BUFSIZE) {
		auto i = d.head[0];
		while (i < (int)d.raw->size()) {
			io g;
			g.head.clear();
			g.raw = d.raw;
			g.head.emplace_back(i);
			for (auto j = 0; j < (int)d.head.size() && d.head[j] < i+BUFSIZE; ++j) {
				if (d.head[j] > i)
					g.head.emplace_back(d.head[j]);
			}
			if (g.head[g.head.size()-1] != d.head[d.head.size()-1]) {
				g.head.emplace_back(min((int)d.raw->size(), i+BUFSIZE));
			}
			vector<int> c;
			_parse(g, c);
			if (i == 0) {
				for (auto& j : c) {
					b.emplace_back(j);
				}
			} else {
				if (!b.empty())
					b.pop_back();
				for (auto k = 1; (int)c.size(); ++k) {
					b.emplace_back(c[k]);
				}
			}
			if (c.size() > 2) {
				i = c[c.size()-2];
			} else {
				i = min((int)d.raw->size(), i+BUFSIZE/2);
			}
		}
	} else {
		_parse(d, b);
	}
}

void usbd_l::_parse(io& f, vector<int>& b) {
	vt dp;
	vector<double> cum;
	vector<vector<double> > bos;
	vector<vector<double> > eos;
	_cumurative(f, cum);
	_bos(f, bos);
	_eos(f, eos);
	int size = f.head.size()-1;
	int head = f.head[0];
	int tail = f.head[size];
	int len = tail-head;
	unordered_map<int, double> alpha;
	for (int t = 0; t < len; ++t) {
		double a = 0;
		for (int l = 0; l < t; ++l) {
			a = math::lse(a, dp[t-1][l].v, (l==0));
		}
		alpha[t-1] = a;
		for (int j = 1; j <= t+1; ++j) {
			int b = min(j-1, _n-2);
			int e = min(j, _n-1);
			double lp_bos = bos[t-j+1][b];
			double lp_eos = eos[t+1][e];
			double lp = cum[t]-cum[t-j+b+1]+lp_bos+lp_eos;
			dp[t][j-1].v = lp+alpha[t-j];
		}
	}
	b.emplace_back(tail);
	int t = len-1;
	while (t >= 0) {
		vector<double> table;
		for (int k = 0; k <= t; ++k) {
			table.emplace_back(dp[t][k].v);
		}
		int l = 1+rd::best(table);
		t -= l;
		if (t > 0)
			b.emplace_back(head+t+1);
	}
	b.emplace_back(head);
	reverse(b.begin(), b.end());
}

void usbd_l::_cumurative(io& d, vector<double>& cum) {
	int head = d.head[0];
	int tail = d.head[d.head.size()-1];
	double c = 0;
	word w(*d.raw, head, tail-head);
	int len = tail-head+1;
	double ln_gen_prior = log(1.-_general_prior);
	double ln_cr_prior = log(1.-_cr_prior);
	double ln_pnc_prior = log(1.-_pnc_prior);
	int h = head;
	int p = 0;
	for (auto i = head; i < tail; ++i) {
		while(i > h)
			h = d.head[++p];
		bool match = false;
		if (h == i)
			match = true;
		bool is_punct = false;
		if (i > 0 && find(_punc.begin(), _punc.end(), (*d.raw)[i-1]) != _punc.end()) {
			is_punct = true;
		}
		c += _lm->lp(w, i-head);
		if (match) {
			c += ln_cr_prior;
		} else if (is_punct) {
			c += ln_pnc_prior;
		} else {
			c += ln_gen_prior;
		}
		cum.emplace_back(c);
	}
}

void usbd_l::_bos(io& d, vector<vector<double> >& bos) {
	int head = d.head[0];
	int tail = d.head[d.head.size()-1];
	double ln_gen_prior = log(1.-_general_prior);
	double ln_cr_prior = log(1.-_cr_prior);
	double ln_pnc_prior = log(1.-_pnc_prior);
	for (int i = head; i < tail; ++i) {
		bos.emplace_back(vector<double>());
		word w(*d.raw, i, _n);
		double c = 0;
		int h = head;
		int p = 0;
		for (int n = 0; n < _n-1; ++n) {
			if (i+n > tail)
				break;
			while (i+n > h)
				h = d.head[++p];
			bool match = false;
			if (n > 0 && h == i+n)
				match = true;
			bool is_punct = false;
			if (n > 0 && find(_punc.begin(), _punc.end(), (*d.raw)[i+n-1]) != _punc.end()) {
				is_punct = true;
			}
			double lp = _lm->lp(w, n);
			c += lp;
			if (match) {
				c += ln_cr_prior;
			} else if (is_punct) {
				c += ln_pnc_prior;
			} else {
				c += ln_gen_prior;
			}
			bos[i-head].emplace_back(c);
		}
	}
}

void usbd_l::_eos(io& d, vector<vector<double> >& eos) {
	int head = d.head[0];
	int tail = d.head[d.head.size()-1];
	double ln_gen_prior = log(_general_prior);
	double ln_cr_prior = log(_cr_prior);
	double ln_pnc_prior = log(_pnc_prior);
	int h = head;
	int p = 0;
	for (int i = head; i < tail+1; ++i) {
		eos.emplace_back(vector<double>());
		while (i > h)
			h = d.head[++p];
		bool match = false;
		if (h == i)
			match = true;
		bool is_punct = false;
		if (i > 0 && find(_punc.begin(), _punc.end(), (*d.raw)[i-1]) != _punc.end())
			is_punct = true;
		for (int n = 0; n < _n; ++n) {
			int e = max(0, i-n);
			word w(*d.raw, e, n);
			double lp = _lm->lp(w, n);
			if (match) {
				lp += ln_cr_prior;
			} else if (is_punct) {
				lp += ln_pnc_prior;
			} else {
				lp += ln_gen_prior;
			}
			eos[i-head].emplace_back(lp);
		}
	}
}

void usbd_l::eval(cio& target, cio& correct, vector<double>& c) {
	if (target.chunk->size() != correct.chunk->size())
		return;
	if (!c.empty())
		c.clear();
	int letters = 0;
	int correct_seg = 0;
	int target_seg = 0;
	int tp = 0;
	int fp = 0;
	int fn = 0;
	for (auto i = 0; i < (int)target.chunk->size(); ++i) {
		correct_seg += (*correct.chunk)[i].head.size();
		target_seg += (*target.chunk)[i].head.size();
		letters += (*target.chunk)[i].raw->size();
		int j = 0;
		int k = 0;
		while (1) {
			if ((*correct.chunk)[i].head[j] == (*target.chunk)[i].head[k]) {
				++tp, ++j, ++k;
			} else if ((*correct.chunk)[i].head[j] > (*target.chunk)[i].head[k]) {
				++j;
				++fn;
			} else if ((*correct.chunk)[i].head[j] < (*target.chunk)[i].head[k]) {
				++k;
				++fp;
			}
			if (j >= (int)(*correct.chunk)[i].head.size() && k >= (int)(*target.chunk)[i].head.size())
				break;
		}
	}
	int tn = letters-correct_seg;
	double prec = (double)tp/((double)tp+fp);
	double rec = (double)tp/((double)tp+fn);
	double f1 = prec*rec*2./(prec+rec);
	double acc = (double)(tp+tn)/(double)(tp+fp+tn+fn);
	c.emplace_back(prec);
	c.emplace_back(rec);
	c.emplace_back(f1);
	c.emplace_back(acc);
}

void usbd_l::save(const char *file) {
	FILE *fp = NULL;
	if ((fp = fopen(file, "wb")) == NULL)
		throw "failed to open file in usbd_l::save";
	try {
		if (fwrite(&_n, sizeof(int), 1, fp) != 1)
			throw "failed to write _n in usbd_l::save";
		if (fwrite(&_A, sizeof(double), 1, fp) != 1)
			throw "failed to write _A in usbd_l::save";
		if (fwrite(&_B, sizeof(double), 1, fp) != 1)
			throw "failed to write _B in usbd_l::save";
		if (fwrite(&_C, sizeof(double), 1, fp) != 1)
			throw "failed to write _C in usbd_l::save";
		if (fwrite(&_D, sizeof(double), 1, fp) != 1)
			throw "failed to write _D in usbd_l::save";
		if (fwrite(&_E, sizeof(double), 1, fp) != 1)
			throw "failed to write _E in usbd_l::save";
		if (fwrite(&_F, sizeof(double), 1, fp) != 1)
			throw "failed to write _F in usbd_l::save";
		if (fwrite(&_general_prior, sizeof(double), 1, fp) != 1)
			throw "failed to write _general_prior in usbd_l::save";
		if (fwrite(&_cr_prior, sizeof(double), 1, fp) != 1)
			throw "failed to write _cr_prior in usbd_l::save";
		if (fwrite(&_pnc_prior, sizeof(double), 1, fp) != 1)
			throw "failed to write _pnc_prior in usbd_l::save";
		int size = _punc.size();
		if (fwrite(&size, sizeof(int), 1, fp) != 1)
			throw "failed to write punc_size in usbd_l::save";
		if (fwrite(&_punc[0], sizeof(unsigned int), size, fp) != size)
			throw "failed to write _punc in usbd_l::save";
		string lm(file);
		lm += ".lm";
		_lm->save(lm.c_str());
	} catch (const char *ex) {
		throw ex;
	}
	fclose(fp);
}

void usbd_l::load(const char *file) {
	FILE *fp = NULL;
	if ((fp = fopen(file, "rb")) == NULL)
		throw "failed to open file in usbd_l::load";
	try {
		if (fread(&_n, sizeof(int), 1, fp) != 1)
			throw "failed to load _n in usbd_l::load";
		if (fread(&_A, sizeof(double), 1, fp) != 1)
			throw "failed to load _A in usbd_l::load";
		if (fread(&_B, sizeof(double), 1, fp) != 1)
			throw "failed to load _B in usbd_l::load";
		if (fread(&_C, sizeof(double), 1, fp) != 1)
			throw "failed to load _C in usbd_l::load";
		if (fread(&_D, sizeof(double), 1, fp) != 1)
			throw "failed to load _D in usbd_l::load";
		if (fread(&_E, sizeof(double), 1, fp) != 1)
			throw "failed to load _E in usbd_l::load";
		if (fread(&_F, sizeof(double), 1, fp) != 1)
			throw "failed to load _F in usbd_l::load";
		if (fread(&_general_prior, sizeof(double), 1, fp) != 1)
			throw "failed to load _general_prior in usbd_l::load";
		if (fread(&_cr_prior, sizeof(double), 1, fp) != 1)
			throw "failed to load _cr_prior in usbd_l::load";
		if (fread(&_pnc_prior, sizeof(double), 1, fp) != 1)
			throw "failed to load _pnc_prior in usbd_l::load";
		int size = 0;
		if (fread(&size, sizeof(int), 1, fp) != 1)
			throw "failed to load punc_size in usbd_l::load";
		_punc.resize(size);
		if (fread(&_punc[0], sizeof(unsigned int), size, fp) != size)
			throw "failed to load _punc in usbd_l::load";
		string lm(file);
		lm += ".lm";
		_lm->load(lm.c_str());
	} catch (const char *ex) {
		throw ex;
	}
	fclose(fp);
}

