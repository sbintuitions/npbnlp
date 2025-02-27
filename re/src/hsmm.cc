#include"ylattice.h"
#include"vtable.h"
#include"rd.h"
#include"hsmm.h"

#define C 5000
#define P 5000

using namespace std;
using namespace npbnlp;

hsmm::hsmm(const char *dic):_n(10),_m(3),_word(new hpyp(1)),_letter(new vpyp(_n)),_phonetic(new hpyp(_m)),_dic(new trie(3)) {
	_dic->load(dic, hsmm::vv_reader, hsmm::uint_reader);
	_letter->set_v(C);
	_phonetic->set_v(P);
	_word->set_base(_letter.get());
}

hsmm::hsmm(int n, int m, const char *dic):_n(n),_m(m),_word(new hpyp(1)),_letter(new vpyp(_n)),_phonetic(new hpyp(_m)),_dic(new trie(3)) {
	_dic->load(dic, hsmm::vv_reader, hsmm::uint_reader);
	_letter->set_v(C);
	_phonetic->set_v(P);
	_word->set_base(_letter.get());
}

hsmm::~hsmm() {
}

int hsmm::n() {
	return _n;
}

int hsmm::m() {
	return _m;
}

void hsmm::set(int v) {
	_v = v;
	_letter->set_v(_v);
}

void hsmm::estimate(int iter) {
	_word->gibbs(iter);
	_word->estimate(iter);
	_letter->estimate(iter);
	_phonetic->estimate(iter);
}

void hsmm::poisson_correction(int n) {
	_word->poisson_correction(n);
}

void hsmm::add(vector<pair<word, vector<unsigned int> > >& s) {
	int rd[s.size()] = {0};
	rd::shuffle(rd, s.size());
	for (auto i = 0; i < (int)s.size(); ++i) {
		word& w = s[rd[i]].first;
		_word->add(w, _word->h());
	}
	vector<unsigned int> p;
	for (auto& x : s) {
		for (auto& i : x.second) {
			p.emplace_back(i);
		}
		/*
		// bos of reading for word unigram
		int j = 0;
		context *b = _phonetic->h();
		for (auto k = j-1; k > j-_m; --k)
			b = b->make(0);
		_phonetic->add(x.second[j], b);
		// eos of reading for ward unigram
		j = x.second.size()-1;
		context *e = _phonetic->h();
		for (auto k = j; k > j-_m+1; --k) {
			int l = (k >= 0)? x.second[k]: 0;
			e = e->make(l);
		}
		_phonetic->add(0, e);
		*/
	}
	int prd[p.size()] = {0};
	rd::shuffle(prd, p.size());
	for (auto i = 0; i < (int)p.size(); ++i) {
		context *c = _phonetic->h();
		for (auto j = prd[i]-1; j > prd[i]-_m; --j) {
			int k = (j >= 0)?p[j]:0;
			c = c->make(k);
		}
		_phonetic->add(p[prd[i]], c);
	}
}

void hsmm::remove(vector<pair<word, vector<unsigned int> > >& s) {
	for (auto i = 0; i < (int)s.size(); ++i) {
		_word->remove(s[i].first, _word->h());
	}
	vector<unsigned int> p;
	for (auto& x : s) {
		for (auto& i : x.second) {
			p.emplace_back(i);
		}
		/*
		// bos of reading for word unigram
		int j = 0;
		context *b = _phonetic->h();
		for (auto k = j-1; k > j-_m; --k)
			b = b->find(0);
		_phonetic->remove(x.second[j], b);
		// eos of reading for word unigram
		j = x.second.size()-1;
		context *e = _phonetic->h();
		for (auto k = j; k > j-_m+1; --k) {
			int l = (k >= 0)? x.second[k]: 0;
			e = e->find(l);
		}
		_phonetic->remove(0, e);
		*/
	}
	for (auto i = 0; i < (int)p.size(); ++i) {
		context *c = _phonetic->h();
		for (auto j = i-1; j > i-_m; --j) {
			int k = (j >= 0)? p[j] : 0;
			c = c->find(k);
		}
		_phonetic->remove(p[i], c);
	}
}

void hsmm::save(const char *file) {
	FILE *fp = NULL;
	if ((fp = fopen(file, "wb")) == NULL)
		throw "failed to open save file in hsmm::save";
	try {
		if (fwrite(&_n, sizeof(int), 1, fp) != 1)
			throw "failed to write _n in hsmm::save";
		if (fwrite(&_m, sizeof(int), 1, fp) != 1)
			throw "failed to write _m in hsmm::save";
		if (fwrite(&_v, sizeof(int), 1, fp) != 1)
			throw "failed to write _v in hsmm::save";
		_word->save(fp);
		_letter->save(fp);
		_phonetic->save(fp);
	} catch (const char *ex) {
		throw ex;
	}
	fclose(fp);
}

void hsmm::load(const char *file) {
	FILE *fp = NULL;
	if ((fp = fopen(file, "rb")) == NULL)
		throw "failed to open save file in hsmm::load";
	try {
		if (fread(&_n, sizeof(int), 1, fp) != 1)
			throw "failed to write _n in hsmm::load";
		if (fread(&_m, sizeof(int), 1, fp) != 1)
			throw "failed to write _m in hsmm::load";
		if (fread(&_v, sizeof(int), 1, fp) != 1)
			throw "failed to write _v in hsmm::load";
		_word->load(fp);
		_letter->load(fp);
		_phonetic->load(fp);
		_letter->set_v(_v);
	} catch (const char *ex) {
		throw ex;
	}
	fclose(fp);
}

void hsmm::init(io& f, vector<vector<pair<word, vector<unsigned int> > > >& c) {
}

vector<pair<word, vector<unsigned int> > > hsmm::sample(io& f, int i) {
	ylattice l(f, i, *_dic);
	vt dp;
	for (auto t = 0; t < (int)l.size(); ++t) {
		for (auto j = 0; j < l.size(t); ++j) {
			ynode& cur = l.get(t, j);
			_precalc(cur);
			double lp_w = _word->lp(cur.w, _word->h());
			for (auto k = 0; k < l.size(t-cur.w.len); ++k) {
				ynode& prev = l.get(t-cur.w.len, k);
				dp[t][j].v = math::lse(dp[t][j].v, dp[t-cur.w.len][k].v+lp_w+_transition(prev, cur), !dp[t][j].is_init());
				if (!dp[t][j].is_init())
					dp[t][j].set(true);
			}	
		}
	}
	vector<pair<word, vector<unsigned int> > > s;
	ynode *n = l.getp(l.size(), 0); // eos
	s.emplace_back(make_pair(n->w, n->phonetic));
	auto t = (int)l.size()-n->w.len;
	while (t >= 0) {
		vector<double> table;
		for (auto j = 0; j < l.size(t); ++j) {
			ynode& prev = l.get(t, j);
			table.emplace_back(dp[t][j].v+_transition(prev, *n));
		}
		int id = rd::ln_draw(table);
		n = l.getp(t, id);
		s.emplace_back(make_pair(n->w, n->phonetic));
		t -= n->w.len;
	}
	reverse(s.begin(), s.end());
	return s;
}

vector<pair<word, vector<unsigned int> > > hsmm::parse(io& f, int i) {
	ylattice l(f, i, *_dic);
	vt dp;
	for (auto t = 0; t < (int)l.size(); ++t) {
		for (auto j = 0; j < l.size(t); ++j) {
			ynode& cur = l.get(t, j);
			_precalc(cur);
			double lp_w = _word->lp(cur.w, _word->h());
			for (auto k = 0; k < l.size(t-cur.w.len); ++k) {
				ynode& prev = l.get(t-cur.w.len, k);
				dp[t][j].v = math::lse(dp[t][j].v, dp[t-cur.w.len][k].v+lp_w+_transition(prev, cur), !dp[t][j].is_init());
				if (!dp[t][j].is_init())
					dp[t][j].set(true);
			}	
		}
	}
	vector<pair<word, vector<unsigned int> > > s;
	ynode *n = l.getp(l.size(), 0); // eos
	s.emplace_back(make_pair(n->w, n->phonetic));
	auto t = (int)l.size()-n->w.len;
	while (t >= 0) {
		vector<double> table;
		for (auto j = 0; j < l.size(t); ++j) {
			ynode& prev = l.get(t, j);
			table.emplace_back(dp[t][j].v+_transition(prev, *n));
		}
		int id = rd::best(table);
		n = l.getp(t, id);
		s.emplace_back(make_pair(n->w, n->phonetic));
		t -= n->w.len;
	}
	reverse(s.begin(), s.end());
	return s;
}

double hsmm::_transition(ynode& prev, ynode& cur) {
	double diff = 0;
	for (auto i = 0; i < min(_m-1, (int)cur.phonetic.size()); ++i) {
		context *c = _phonetic->h();
		for (auto j = i-1; i > i-_m; --j) {
			int p = 0;
			if (j >= 0) {
				p = cur.phonetic[j];
			} else if (prev.phonetic.size()+j >= 0) {
				p = prev.phonetic[prev.phonetic.size()+j];
			}
			context *h = c->find(p);
			if (!h)
				break;
			c = h;
		}
		diff += _phonetic->lp(cur.phonetic[i],c);
	}
	double lp = prev.bos + prev.prod + diff;
	if ((int)cur.phonetic.size() >= _m) {
		lp += cur.prod + cur.eos;
	} else {
		int size = cur.phonetic.size();
		for (auto i = _m-1; i < size; ++i) {
			context *c = _phonetic->h();
			for (auto j = i-1; i > i-_m; --j) {
				int p = 0;
				if (j >= 0) {
					p = cur.phonetic[j];
				} else if (prev.phonetic.size()+j >= 0) {
					p = prev.phonetic[prev.phonetic.size()+j];
				}
				context *h = c->find(p);
				if (!h)
					break;
				c = h;
			}
			lp += _phonetic->lp(cur.phonetic[i], c);
		}
		context *e = _phonetic->h();
		for (auto j = cur.phonetic.size()-1; j > cur.phonetic.size()-_m; --j) {
			int p = 0;
			if (j >= 0) {
				p = cur.phonetic[j];
			} else if (prev.phonetic.size()+j >= 0) {
				p = prev.phonetic[prev.phonetic.size()+j];
			}
			context *h = e->find(p);
			if (!h)
				break;
			e = h;
		}
		lp += _phonetic->lp(0, e);
	}
	return lp/(prev.phonetic.size()+cur.phonetic.size());
}


void hsmm::_precalc(ynode& n) {
	// bos
	for (auto i = 0; i < min(_m-1,(int)n.phonetic.size()); ++i) {
		context *c = _phonetic->h();
		for (auto j = i-1; j > i-_m; --j) {
			int p = (j >= 0)? n.phonetic[j]:0;
			context *h = c->find(p);
			if (!h)
				break;
			c = h;
		}
		n.bos += _phonetic->lp(n.phonetic[i], c);
	}
	// eos
	context *e = _phonetic->h();
	for (auto j = n.phonetic.size()-1; j > n.phonetic.size()-_m; --j) {
		int p = (j >= 0)? n.phonetic[j]:0;
		context *h = e->find(p);
		if (!h)
			break;
		e = h;
	}
	n.eos = _phonetic->lp(0, e);
	// product
	for (auto i = _m-1; i < (int)n.phonetic.size(); ++i) {
		context *c = _phonetic->h();
		for (auto j = i-1; j > i-_m; --j) {
			context *h = c->find(n.phonetic[j]);
			if (!h)
				break;
			c = h;
		}
		n.prod += _phonetic->lp(n.phonetic[i], c);
	}
}
