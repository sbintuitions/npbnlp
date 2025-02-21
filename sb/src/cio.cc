#include"cio.h"
#include"word.h"

using namespace std;
using namespace npbnlp;

static string err;
cio::cio(const char *file):chunk(new vector<io>) {
	ifstream f(file);
	if (!f) {
		err = "couldn't open file:";
		err += file;
		throw err.c_str();
	}
	io i;
	string buf;
	while (getline(f, buf)) {
		//io::swap_cr2ws(buf);
		io::chomp(buf);
		//cout << buf.size() << " " << buf << endl;
		//if (buf.size() == 0) {
		if (strlen(buf.c_str()) == 0) {
			chunk->push_back(i);
			i = io();
		} else {
			io::s2i(buf.c_str(), *i.raw);
			i.head.push_back(i.raw->size());
		}
	}
	if (!i.raw->empty())
		chunk->push_back(i);
/*
	for (auto it = chunk->begin(); it != chunk->end(); ++it) {
		cout << "head_size:" << it->head.size() << endl;
		for (auto j = it->head.begin(); j != it->head.end()-1; ++j) {
			int h = *j;
			int t = *(j+1);
			cout << "head:" << h << " tail:" << t << " t-h:" << t-h << endl;
			word w(*it->raw, h, t-h);
			for (auto k = 0; k < w.len; ++k) {
				char buf[5] = {0};
				io::i2c(w[k],buf);
				cout << w[k] << ":" << buf << endl;
			}
			cout << w << endl;
		}
	}
*/
}

cio::cio(istream& in):chunk(new vector<io>) {
	if (!in) {
		err = "invalid input stream";
		throw err.c_str();
	}
	io i;
	string buf;
	while (getline(in, buf)) {
		io::swap_cr2ws(buf);
		if (buf.size() == 0) {
			chunk->push_back(i);
			i = io();
		} else {
			io::s2i(buf.c_str(), *i.raw);
			i.head.push_back(i.raw->size());
		}
	}
	if (!i.raw->empty())
		chunk->push_back(i);
}

cio::~cio() {
}
