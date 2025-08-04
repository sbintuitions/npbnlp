#include"ylattice2.h"
#include"beta.h"
#include"rd.h"

using namespace std;
using namespace npbnlp;

static word w_eos;
static vector<unsigned int> p_eos(1, 0);
static ynode eos(w_eos, p_eos);

ynode::ynode(word& w, vector<unsigned int>& p):w(w),phonetic(p),prod(0)/*,bos(0),eos(0)*/ {
}

ynode::~ynode() {
}

ynode::ynode(const ynode& n):w(n.w),phonetic(n.phonetic),prod(n.prod)/*,bos(n.bos),eos(n.eos)*/ {
}

ynode& ynode::operator=(const ynode& n) {
	w = n.w;
	if (!phonetic.empty())
		phonetic.clear();
	for (auto& i : n.phonetic)
		phonetic.emplace_back(i);
	//bos = n.bos;
	prod = n.prod;
	//eos = n.eos;
	return *this;
}

ylattice::ylattice(io& f, int i, trie& t, vector<shared_ptr<hpyp> >& wrd, vector<shared_ptr<hpyp> >& phonetic, hpyp& pos, trie *unit) {
	int head = f.head[i];
	int tail = f.head[i+1];
	for (auto j = head; j < tail; ++j) {
		query.emplace_back((*f.raw)[j]);
	}
	y.resize(query.size());
	shared_ptr<wid> d = wid::create();
	beta_distribution be;
	for (auto j = 0; j < (int)query.size(); ++j) {
		vector<unsigned int> key(query.begin()+j, query.end());
		key.emplace_back(3);
		auto r = t.cp_search(key);
		vector<ynode> nodes;
		vector<int> length;
		vector<double> table;
		//double z = 0;
		for (auto& n : r) {
			word w(*f.raw, head+j, n.first);
			w.id = (*d)[w];
			auto v = t.getval(n.second);
			if (v) {
				for (auto& m : v.value()) {
					ynode node(w,m);
					nodes.emplace_back(node);
					length.emplace_back(n.first);
					double lp = prob_node(node, wrd, phonetic, pos);
					//z = math::lse(z, lp, (z==0));
					table.emplace_back(lp);
				}
			}
		}
		if (unit && y[j].empty()) {
			auto u = unit->cp_search(key);
			for (auto& m : u) {
				word w(*f.raw, head+j, m.first);
				w.id = d->index(w);
				auto v = unit->getval(m.second);
				if (v) {
					for (auto& l : v.value()) {
						ynode node(w, l);
						nodes.emplace_back(node);
						length.emplace_back(m.first);
						double lp = prob_node(node, wrd, phonetic, pos);
						//z = math::lse(z, lp, (z==0));
						table.emplace_back(lp);
					}
				}
			}
		}
		if (table.size() > 0) {
			int id = rd::ln_draw(table);
			double mu = log(be(1.,1.)+table[id]);
			for (auto k = 0; k < (int)nodes.size(); ++k) {
				if (table[k] < mu)
					continue;
				y[j+length[k]-1].emplace_back(nodes[k]);
			}
		}

		if (y[j].empty()) {
			word w(*f.raw, head+j, 1);
			w.id = 1; // unk
			vector<unsigned int> n(1, w[0]);
			y[j].emplace_back(ynode(w, n));
		}
	}
}

ylattice::~ylattice() {
}

double ylattice::prob_node(ynode& node, vector<shared_ptr<hpyp> >& wrd, vector<shared_ptr<hpyp> >& phonetic, hpyp& pos) {
	double lp = 0; // p(word, phonetic) = \sum_k p(word|k)p(phonetic|k)p(k)
	int k = wrd.size();
	for (auto i = 0; i < k; ++i) {
		double lp_pos = pos.lp(i, pos.h());
		double lp_wd = wrd[i]->lp(node.w, wrd[i]->h());
		double lp_phone = 0;
		int phonetic_len = node.phonetic.size();
		int n = phonetic[0]->n();
		for (auto j = 0; j <= phonetic_len; ++j) {
			context *c = phonetic[i]->h();
			for (auto m = 1; m < n; ++m) {
				int p = (j-m >= 0)? node.phonetic[j-m]:0;
				context *d = c->find(p);
				if (!d)
					break;
				c = d;
			}
			int p = (j < phonetic_len)? node.phonetic[j]:0;
			lp_phone += phonetic[i]->lp(p, c);
		}
		lp_phone /= phonetic_len;
		lp += lp_pos+lp_wd+lp_phone;
	}
	return lp;
}

int ylattice::len(int i, int j) {
	if (i < 0 || i >= (int)y.size())
		return 1;
	if (j >= (int)y[i].size())
		throw "found invalid node id in len()";
	return y[i][j].w.len;
}

ynode& ylattice::get(int i, int j) {
	if (i < 0 || i >= (int)y.size())
		return eos;
	if (j >= (int)y[i].size())
		throw "found invalid node id in get()";
	return y[i][j];
}

ynode* ylattice::getp(int i, int j) {
	if (i < 0 || i >= (int)y.size())
		return &eos;
	if (j >= (int)y[i].size())
		throw "found invalid node id in getp()";
	return &y[i][j];
}

int ylattice::size() {
	return y.size();
}

int ylattice::size(int i) {
	if (i < 0 || i >= (int)y.size())
		return 1;
	return y[i].size();
}
