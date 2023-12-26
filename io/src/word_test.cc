#include"io.h"
#include"word.h"
#include"util.h"
//#define DEBUG

using namespace npbnlp;
using namespace std;

int main(int argc, char **argv) {
	try {
		io f(*(argv+1));
#ifdef DEBUG
		auto it = f.raw->begin();
		for (; it != f.raw->end(); ++it) {
			char buf[5] = {0};
			io::i2c(*it, buf);
			cout << *it << ":" << buf << endl;
		}
#endif
		shared_ptr<wid> dic = wid::create();
		vector<word> s;
		int i = 0;
		while (i < f.raw->size()) {
			word w(*f.raw);
			i = util::store_word(w, *f.raw, i);
			w.id = dic->index(w);
			s.push_back(w);
		}
		for (auto it = s.begin(); it != s.end(); ++it)
			cout << *it << ":" << it->pos << endl;

	} catch (const char *ex) {
		cerr << ex << endl;
	}
	return 0;
}
