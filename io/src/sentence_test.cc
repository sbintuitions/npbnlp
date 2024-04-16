#include"io.h"
#include"word.h"
#include"sentence.h"
#include"util.h"
//#define DEBUG

using namespace std;
using namespace npbnlp;

int main(int argc, char **argv) {
	try {
		io f(*(argv+1));
		vector<sentence> c;
		util::store_sentences(f, c);
		for (auto it = c.begin(); it != c.end(); ++it) {
			for (auto w = it->w.begin(); w != it->w.end(); ++w)
				cout << *w << " ";
			cout << endl;
		}
		cout << "word ids:" << endl;
		for (auto it = c.begin(); it != c.end(); ++it) {
			for (int i = -1; i <= (int)it->w.size(); ++i) {
				cout << i << ":" << (*it)[i] << " ";
			}
			cout << endl;
		}
		cout << "character ids:" << endl;
		for (auto it = c.begin(); it != c.end(); ++it) {
			for (int i = 0; i < (int)it->w.size(); ++i) {
				word& w = it->wd(i);
				for (int j = -1; j <= (int)w.len; ++j) {
					cout << j << ":" << w[j] << " ";
				}
				cout << endl;
			}
		}
	} catch (const char *ex) {
		cerr << ex << endl;
	}
	return 0;
}
