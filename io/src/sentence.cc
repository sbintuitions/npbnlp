#include"sentence.h"
#include"util.h"

using namespace std;
using namespace npbnlp;

static word eos;
sentence::sentence() {
}

sentence::sentence(vector<unsigned int>& d, int head, int tail) {
	shared_ptr<wid> dic = wid::create();
	int i = head;
	while (i < tail) {
		word wd(d);
		i = util::store_word(wd, d, i);
		wd.id = dic->index(wd);
		w.push_back(wd);
	}
	n.resize(w.size(), 0);
}

sentence::~sentence() {
}

int sentence::operator[](int i) {
	if (i < 0 || i >= w.size())
		return 0; // bos/eos
	return w[i].id;
}


word& sentence::wd(int i) {
	if (i < 0 || i >= w.size()) {
		return eos;
	}
	return w[i];
}

sentence::sentence(const sentence& s) {
	for (auto it = s.w.begin(); it != s.w.end(); ++it)
		w.push_back(*it);
	for (auto it = s.n.begin(); it != s.n.end(); ++it)
		n.push_back(*it);
}

sentence::sentence(sentence&& s) {
	w = move(s.w);
	n = move(s.n);
	s.w.clear();
	s.n.clear();
}

sentence& sentence::operator=(const sentence& s) {
	w.clear();
	n.clear();
	for (auto it = s.w.begin(); it != s.w.end(); ++it)
		w.push_back(*it);
	for (auto it = s.n.begin(); it != s.n.end(); ++it)
		n.push_back(*it);
	return *this;
}

sentence& sentence::operator=(sentence&& s) noexcept {
	if (this != &s) {
		w = move(s.w);
		n = move(s.n);
		s.w.clear();
		s.n.clear();
		//*this = move(s);
	}
	return *this;
}

int sentence::size() {
	return w.size();
}
