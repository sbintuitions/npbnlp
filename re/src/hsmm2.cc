#include"hsmm2.h"
#include"vtable.h"
#include"rd.h"
#include"beta.h"

#define C 5000
#define P 5000
#define K 100

using namespace std;
using namespace npbnlp;

static word w_eos;
static vector<unsigned int> p_eos(1, 0);
static vector<int> bos(1, 0);

hmm::lattice::lattice(int z, vector<pair<word, vector<unsigned int> > >& s, vector<shared_ptr<hpyp> >& wd, vector<shared_ptr<hpyp> >& phn, hpyp& pos):_sequence(&s) {
	beta_distribution be;
	shared_ptr<generator> g = generator::create();
	int len = _sequence->size();
	k.resize(len);
	for (auto t = 0; t < len; ++t) {
		context *h = pos.h();
		word *prev_w = wp(t-1);
		context *u = h->find(prev_w->pos);
		if (u)
			h = u;
		word *cur_w = wp(t);
		vector<unsigned int> *cur_r = rp(t);
		vector<double> table;
		for (auto k = 1; k < z; ++k) {
			double lp_em = lpr(*cur_w, *cur_r, *wd[k], *phn[k]);
			double lp_pos = pos.lp(k, h);
			table.emplace_back(lp_em+lp_pos);
		}
		int id = rd::ln_draw(table);
		double mu = log(be(1,1))+table[id];
		cur_w->pos = id+1;
		for (auto i = 0; i < (int)table.size(); ++i) {
			if (table[i] >= mu)
				k[t].emplace_back(i+1);
		}
	}
}

hmm::lattice::~lattice() {
}

vector<int>::iterator hmm::lattice::begin(int i) {
	if (i < 0 || i >= (int)_sequence->size())
		return bos.begin();
	return k[i].begin();
}

vector<int>::iterator hmm::lattice::end(int i) {
	if (i < 0 || i >= (int)_sequence->size())
		return bos.end();
	return k[i].end();
}

word* hmm::lattice::wp(int i) {
	if (i < 0 || i >= (int)_sequence->size())
		return &w_eos;
	return &(*_sequence)[i].first;
}

vector<unsigned int>* hmm::lattice::rp(int i) {
	if (i < 0 || i >= (int)_sequence->size())
		return &p_eos;
	return &(*_sequence)[i].second;
}

double hmm::lattice::lpr(word& w, vector<unsigned int>& r, hpyp& wd, hpyp& phn) {
	double lp = wd.lp(w, wd.h());
	int len = r.size();
	double lp_phone = 0;
	for (auto i = 0; i <= len; ++i) {
		context *c = phn.h();
		int n = phn.n();
		for (auto m = 1; m < n; ++m) {
			int p = (i-m >= 0)? r[i-m]:0;
			context *d = c->find(p);
			if (!d)
				break;
			c = d;
		}
		int p = (i < len)? r[i]:0;
		lp_phone += phn.lp(p, c);
	}
	lp_phone /= (len+1);
	return lp+lp_phone;
}

hmm::hmm(int n, int m, int k):_n(n),_m(m),_k(k),_word(new vector<shared_ptr<hpyp> >), _letter(new vector<shared_ptr<vpyp> >), _phonetic(new vector<shared_ptr<hpyp> >), _pos(new hpyp(2)) {
	_pos->set_v(K);
	for (auto i = 0; i < _k; ++i) {
		_word->emplace_back(shared_ptr<hpyp>(new hpyp(1)));
		_letter->emplace_back(shared_ptr<vpyp>(new vpyp(_n)));
		(*_letter)[i]->set_v(C);
		(*_word)[i]->set_base((*_letter)[i].get());
		_phonetic->emplace_back(shared_ptr<hpyp>(new hpyp(_m)));
		(*_phonetic)[i]->set_v(P);
	}
}

hmm::~hmm() {
}

void hmm::sample(vector<pair<word, vector<unsigned int> > >& s) {
	hmm::lattice l(_k, s, *_word, *_phonetic, *_pos);
	vt dp;
	int len = s.size();
	for (auto t = 0; t < len; ++t) {
		word *w = l.wp(t);
		for (auto it = l.begin(t); it != l.end(t); ++it) {
			int k = *it;
			context *h = _pos->h();
			for (auto jt = l.begin(t-1); jt != l.end(t-1); ++jt) {
				context *g = h->find(*jt);
				context *c = (g)? g:h;
				double lp_tr = _pos->lp(k, c);
				double lp_em = (*_word)[k]->lp(*w, (*_word)[k]->h());
				dp[t][k].v = math::lse(dp[t][k].v, dp[t-1][*jt].v+lp_tr+lp_em, !dp[t][k].is_init());
				if (!dp[t][k].is_init())
					dp[t][k].set(true);
			}
		}
	}
	word *w = l.wp(len);
	auto t = len-1;
	while (t >= 0) {
		context *h = _pos->h();
		int k = w->pos;
		vector<double> table;
		vector<int> pos;
		for (auto it = l.begin(t); it != l.end(t); ++it) {
			context *g = h->find(*it);
			context *c = (g)?g:h;
			double lp_tr = _pos->lp(k, c);
			table.emplace_back(dp[t][*it].v+lp_tr);
			pos.emplace_back(*it);
		}
		int id = rd::ln_draw(table);
		w = l.wp(t--);
		w->pos = pos[id];
	}
}

void hmm::add(vector<pair<word, vector<unsigned int> > >& s) {
	shared_ptr<wid> dic = wid::create();
	int rd[s.size()] = {0};
	rd::shuffle(rd, s.size());
	for (auto i = 0; i <= (int)s.size(); ++i) {
		if (i < (int)s.size()) {
			word& w = s[rd[i]].first;
			w.id = dic->index(w);
			(*_word)[w.pos]->add(w, (*_word)[w.pos]->h());
			int prev_pos = (rd[i]-1 >= 0)? s[rd[i]-1].first.pos:0;
			context *c = _pos->h();
			context *d = c->make(prev_pos);
			if (d)
				c = d;
			_pos->add(w.pos, c);
			for (auto j = 0; j <= (int)s[rd[i]].second.size(); ++j) {
				context *c = (*_phonetic)[w.pos]->h();
				for (auto t = 1; t < _m; ++t) {
					int p = (j-t >= 0)?s[rd[i]].second[j-t]:0;
					context *d = c->make(p);
					if (!d)
						break;
					c = d;
				}
				int p = (j < (int)s[rd[i]].second.size())?s[rd[i]].second[j]:0;
				(*_phonetic)[w.pos]->add(p, c);
			}
		} else {
			int prev_pos = s[i-1].first.pos;
			context *c = _pos->h();
			context *d = c->make(prev_pos);
			if (d)
				c = d;
			_pos->add(0, c); // eos
		}
	}
}

void hmm::remove(vector<pair<word, vector<unsigned int> > >& s) {
	for (auto i = 0; i <= (int)s.size(); ++i) {
		if (i < (int)s.size()) {
			word& w = s[i].first;
			(*_word)[w.pos]->remove(w, (*_word)[w.pos]->h());
			int prev_pos = (i-1 >= 0)?s[i-1].first.pos:0;
			context *c = _pos->h();
			context *d = c->find(prev_pos);
			if (d)
				c = d;
			_pos->remove(w.pos, c);
			for (auto j = 0; j <= (int)s[i].second.size(); ++j) {
				context *c = (*_phonetic)[w.pos]->h();
				for (auto t = 1; t < _m; ++t) {
					int p = (j-t >= 0)?s[i].second[j-t]:0;
					context *d = c->find(p);
					if (!d)
						break;
					c = d;
				}
				int p = (j < (int)s[i].second.size())?s[i].second[j]:0;
				(*_phonetic)[w.pos]->remove(p, c);
			}
		} else {
			int prev_pos = s[i-1].first.pos;
			context *c = _pos->h();
			context *d = c->find(prev_pos);
			if (d)
				c = d;
			_pos->remove(0, c);
		}
	}
}

hsmm::hsmm(const char *dic, const char *unit):_n(10),_m(3),_k(50),_word(new vector<shared_ptr<hpyp> >),_letter(new vector<shared_ptr<vpyp> >),_phonetic(new vector<shared_ptr<hpyp> >), _pos(new hpyp(2)),_dic(new trie(3)),_unit(nullptr) {
	_dic->load(dic, hsmm::vv_reader, hsmm::uint_reader);
	_pos->set_v(K);
	for (auto k = 0; k < _k; ++k) {
		_word->emplace_back(shared_ptr<hpyp>(new hpyp(1)));
		_letter->emplace_back(shared_ptr<vpyp>(new vpyp(_n)));
		(*_letter)[k]->set_v(C);
		(*_word)[k]->set_base((*_letter)[k].get());
		_phonetic->emplace_back(shared_ptr<hpyp>(new hpyp(_m)));
		(*_phonetic)[k]->set_v(P);
	}
	if (unit) {
		_unit = shared_ptr<trie>(new trie(3));
		_unit->load(unit, hsmm::vv_reader, hsmm::uint_reader);
	}
}

hsmm::hsmm(int n, int m, int k, const char *dic, const char *unit):_n(n),_m(m),_k(k),_word(new vector<shared_ptr<hpyp> >),_letter(new vector<shared_ptr<vpyp> >),_phonetic(new vector<shared_ptr<hpyp> >), _pos(new hpyp(2)),_dic(new trie(3)),_unit(nullptr) {
	_dic->load(dic, hsmm::vv_reader, hsmm::uint_reader);
	_pos->set_v(K);
	for (auto k = 0; k < _k; ++k) {
		_word->emplace_back(shared_ptr<hpyp>(new hpyp(1)));
		_letter->emplace_back(shared_ptr<vpyp>(new vpyp(_n)));
		(*_letter)[k]->set_v(C);
		(*_word)[k]->set_base((*_letter)[k].get());
		_phonetic->emplace_back(shared_ptr<hpyp>(new hpyp(_m)));
		(*_phonetic)[k]->set_v(P);
	}
	if (unit) {
		_unit = shared_ptr<trie>(new trie(3));
		_unit->load(unit, hsmm::vv_reader, hsmm::uint_reader);
	}
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
	for (auto k = 1; k < _k; ++k)
		(*_letter)[k]->set_v(_v);
}

void hsmm::estimate(int iter) {
	for (auto k = 1; k < _k; ++k) {
		(*_word)[k]->gibbs(iter);
		(*_word)[k]->estimate(iter);
		(*_letter)[k]->estimate(iter);
		(*_phonetic)[k]->estimate(iter);
	}
	_pos->estimate(iter);
}

void hsmm::poisson_correction(int n) {
	for (auto k = 1; k < _k; ++k) {
		(*_word)[k]->poisson_correction(n);
	}
}

void hsmm::add(vector<pair<word, vector<unsigned int> > >& s) {
	shared_ptr<wid> dic = wid::create();
	int rd[s.size()] = {0};
	rd::shuffle(rd, s.size());
	for (auto i = 0; i <= (int)s.size(); ++i) {
		if (i < (int)s.size()) {
			word& w = s[rd[i]].first;
			w.id = dic->index(w);
			(*_word)[w.pos]->add(w, (*_word)[w.pos]->h());
			int prev_pos = (rd[i]-1 >= 0)? s[rd[i]-1].first.pos:0;
			context *c = _pos->h();
			context *d = c->make(prev_pos);
			if (d)
				c = d;
			_pos->add(w.pos, c);
			for (auto j = 0; j <= (int)s[rd[i]].second.size(); ++j) {
				context *c = (*_phonetic)[w.pos]->h();
				for (auto t = 1; t < _m; ++t) {
					int p = (j-t >= 0)?s[rd[i]].second[j-t]:0;
					context *d = c->make(p);
					if (!d)
						break;
					c = d;
				}
				int p = (j < (int)s[rd[i]].second.size())?s[rd[i]].second[j]:0;
				(*_phonetic)[w.pos]->add(p, c);
			}
		} else {
			int prev_pos = s[i-1].first.pos;
			context *c = _pos->h();
			context *d = c->make(prev_pos);
			if (d)
				c = d;
			_pos->add(0, c); // eos
		}
	}
}

void hsmm::remove(vector<pair<word, vector<unsigned int> > >& s) {
	for (auto i = 0; i <= (int)s.size(); ++i) {
		if (i < (int)s.size()) {
			word& w = s[i].first;
			(*_word)[w.pos]->remove(w, (*_word)[w.pos]->h());
			int prev_pos = (i-1 >= 0)?s[i-1].first.pos:0;
			context *c = _pos->h();
			context *d = c->find(prev_pos);
			if (d)
				c = d;
			_pos->remove(w.pos, c);
			for (auto j = 0; j <= (int)s[i].second.size(); ++j) {
				context *c = (*_phonetic)[w.pos]->h();
				for (auto t = 1; t < _m; ++t) {
					int p = (j-t >= 0)?s[i].second[j-t]:0;
					context *d = c->find(p);
					if (!d)
						break;
					c = d;
				}
				int p = (j < (int)s[i].second.size())?s[i].second[j]:0;
				(*_phonetic)[w.pos]->remove(p, c);
			}
		} else {
			int prev_pos = s[i-1].first.pos;
			context *c = _pos->h();
			context *d = c->find(prev_pos);
			if (d)
				c = d;
			_pos->remove(0, c);
		}
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
		if (fwrite(&_k, sizeof(int), 1, fp) != 1)
			throw "failed to write _k in hsmm::save";
		if (fwrite(&_v, sizeof(int), 1, fp) != 1)
			throw "failed to write _v in hsmm::save";
		_pos->save(fp);
		for (auto i = 0; i < _k; ++i) {
			(*_word)[i]->save(fp);
			(*_letter)[i]->save(fp);
			(*_phonetic)[i]->save(fp);
		}
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
		if (fread(&_k, sizeof(int), 1, fp) != 1)
			throw "failed to write _k in hsmm::load";
		if (fread(&_v, sizeof(int), 1, fp) != 1)
			throw "failed to write _v in hsmm::load";
		_pos->load(fp);
		_pos->set_v(K);
		for (auto i = 0; i < _k; ++i) {
			if ((int)_word->size() <= i) {
				_word->emplace_back(shared_ptr<hpyp>(new hpyp(1)));
				_letter->emplace_back(shared_ptr<vpyp>(new vpyp(_n)));
				_phonetic->emplace_back(shared_ptr<hpyp>(new hpyp(_m)));
			}
			(*_word)[i]->load(fp);
			(*_letter)[i]->load(fp);
			(*_phonetic)[i]->load(fp);
			(*_letter)[i]->set_v(_v);
			(*_word)[i]->set_base((*_letter)[i].get());
		}
	} catch (const char *ex) {
		throw ex;
	}
	fclose(fp);
}

void hsmm::pretrain(int iter, vector<vector<pair<word, vector<unsigned int> > > >& c) {
	hmm lm(_n,_m,_k);
	for (auto i = 0; i < iter; ++i) {
		for (auto& s : c) {
			if (i > 0)
				lm.remove(s);
			lm.sample(s);
			lm.add(s);
		}
	}
	_word = lm._word;
	_letter = lm._letter;
	_phonetic = lm._phonetic;
	_pos = lm._pos;
}

void hsmm::init(io& f, vector<vector<pair<word, vector<unsigned int> > > >& c) {
	/*
	   int n = f.head.size()-1;
	   shared_ptr<generator> r = generator::create();
	   for (auto i = 0; i < n; ++i) {
	   ylattice l(f, i, *_dic, _unit.get());
	   vector<pair<word, vector<unsigned int> > > s;
	   ynode *node = l.getp(l.size(), 0);
	   s.emplace_back(make_pair(node->w, node->phonetic));
	   auto t = (int)l.size()-node->w.len;
	   while (t >= 0) {
	   int j = (*r)()()%(l.size(t));
	   node = l.getp(t, j);
	   s.push_back(make_pair(node->w, node->phonetic));
	   t -= node->w.len;
	   }
	   reverse(s.begin(), s.end());
	   add(s);
	   c.emplace_back(s);
	   }
	   */
}

vector<pair<word, vector<unsigned int> > > hsmm::sample(io& f, int i) {
	ylattice l(f, i, *_dic, *_word, *_phonetic, *_pos, _unit.get());
	vt dp;
	for (auto t = 0; t < (int)l.size(); ++t) {
		for (auto j = 0; j < l.size(t); ++j) {
			ynode& cur = l.get(t, j);
			for (auto cur_pos = 1; cur_pos < _k; ++cur_pos) {
				double lp_e = _emission(cur, cur_pos);
				for (auto k = 0; k < l.size(t-cur.w.len); ++k) {
					//ynode& prev = l.get(t-cur.w.len, k);
					if (t-cur.w.len < 0) { // bos
						double lp_t = _transition(0, cur_pos);
						dp[t][j][cur_pos].v = math::lse(dp[t][j][cur_pos].v, dp[t-cur.w.len][k][0].v+lp_e+lp_t, !dp[t][j][cur_pos].is_init());
						if (!dp[t][j][cur_pos].is_init())
							dp[t][j][cur_pos].set(true);
						continue;
					}
					for (auto pre_pos = 1; pre_pos < _k; ++pre_pos) {
						double lp_t = _transition(pre_pos, cur_pos);
						dp[t][j][cur_pos].v = math::lse(dp[t][j][cur_pos].v, dp[t-cur.w.len][k][pre_pos].v+lp_e+lp_t, !dp[t][j][cur_pos].is_init());
						if (!dp[t][j][cur_pos].is_init())
							dp[t][j][cur_pos].set(true);
					}
				}
			}
		}
	}
	vector<pair<word, vector<unsigned int> > > s;
	ynode *n = l.getp(l.size(), 0); // eos
	int cur_pos = 0;
	//s.emplace_back(make_pair(n->w, n->phonetic));
	auto t = (int)l.size()-n->w.len;
	while (t >= 0) {
		vector<double> table;
		vector<int> pos;
		vector<int> idx;
		for (auto j = 0; j < l.size(t); ++j) {
			//ynode& prev = l.get(t, j);
			for (auto pre_pos = 1; pre_pos < _k; ++pre_pos) {
				double lp_t = _transition(pre_pos, cur_pos);
				table.emplace_back(dp[t][j][pre_pos].v+lp_t);
				pos.emplace_back(pre_pos);
				idx.emplace_back(j);
			}
		}
		int id = rd::ln_draw(table);
		n = l.getp(t, idx[id]);
		n->w.pos = pos[id];
		cur_pos = pos[id];
		s.emplace_back(make_pair(n->w, n->phonetic));
		t -= n->w.len;
	}
	reverse(s.begin(), s.end());
	return s;
}

vector<pair<word, vector<unsigned int> > > hsmm::parse(io& f, int i) {
	ylattice l(f, i, *_dic, *_word, *_phonetic, *_pos, _unit.get());
	vt dp;
	for (auto t = 0; t < (int)l.size(); ++t) {
		for (auto j = 0; j < l.size(t); ++j) {
			ynode& cur = l.get(t, j);
			for (auto cur_pos = 1; cur_pos < _k; ++cur_pos) {
				double lp_e = _emission(cur, cur_pos);
				for (auto k = 0; k < l.size(t-cur.w.len); ++k) {
					//ynode& prev = l.get(t-cur.w.len, k);
					if (t-cur.w.len < 0) { // bos
						double lp_t = _transition(0, cur_pos);
						dp[t][j][cur_pos].v = math::lse(dp[t][j][cur_pos].v, dp[t-cur.w.len][k][0].v+lp_e+lp_t, !dp[t][j][cur_pos].is_init());
						if (!dp[t][j][cur_pos].is_init())
							dp[t][j][cur_pos].set(true);
						continue;
					}
					for (auto pre_pos = 1; pre_pos < _k; ++pre_pos) {
						double lp_t = _transition(pre_pos, cur_pos);
						dp[t][j][cur_pos].v = math::lse(dp[t][j][cur_pos].v, dp[t-cur.w.len][k][pre_pos].v+lp_e+lp_t, !dp[t][j][cur_pos].is_init());
						if (!dp[t][j][cur_pos].is_init())
							dp[t][j][cur_pos].set(true);
					}
				}
			}
		}
	}
	vector<pair<word, vector<unsigned int> > > s;
	ynode *n = l.getp(l.size(), 0); // eos
	int cur_pos = 0;
	//s.emplace_back(make_pair(n->w, n->phonetic));
	auto t = (int)l.size()-n->w.len;
	while (t >= 0) {
		vector<double> table;
		vector<int> pos;
		vector<int> idx;
		for (auto j = 0; j < l.size(t); ++j) {
			//ynode& prev = l.get(t, j);
			for (auto pre_pos = 1; pre_pos < _k; ++pre_pos) {
				double lp_t = _transition(pre_pos, cur_pos);
				table.emplace_back(dp[t][j][pre_pos].v+lp_t);
				pos.emplace_back(pre_pos);
				idx.emplace_back(j);
			}
		}
		int id = rd::best(table);
		n = l.getp(t, idx[id]);
		n->w.pos = pos[id];
		cur_pos = pos[id];
		s.emplace_back(make_pair(n->w, n->phonetic));
		t -= n->w.len;
	}
	reverse(s.begin(), s.end());
	return s;
}

double hsmm::_emission(ynode& node, int pos) {
	double lp = (*_word)[pos]->lp(node.w, (*_word)[pos]->h());
	int len = node.phonetic.size();
	double lp_phone = 0;
	for (auto i = 0; i <= len; ++i) {
		context *c = (*_phonetic)[pos]->h();
		for (auto m = 1; m < _m; ++m) {
			int p = (i-m >= 0)? node.phonetic[i-m]:0;
			context *d = c->find(p);
			if (!d)
				break;
			c = d;
		}
		int p = (i < len)? node.phonetic[i]:0;
		lp_phone += (*_phonetic)[pos]->lp(p, c);
	}
	lp_phone/=(len+1);
	return lp+lp_phone;
}

double hsmm::_transition(int prev, int cur) {
	context *c = _pos->h();
	context *d = c->find(prev);
	if (d)
		c = d;
	return _pos->lp(cur, c);
}


/*
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
*/
