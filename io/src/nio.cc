#include"nio.h"
#include<memory>
#include<string>

using namespace std;
using namespace npbnlp;

static string err;

nio::nio(vector<sentence>& s):raw(new vector<word>) {
	head.push_back(0);
	for (auto it = s.begin(); it != s.end(); ++it) {
		for (auto i = 0; i < it->size(); ++i) {
			raw->push_back(it->wd(i));
		}
		head.push_back(raw->size());
	}
}

nio::~nio() {
}
