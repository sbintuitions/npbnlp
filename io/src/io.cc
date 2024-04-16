#include"io.h"
#include<memory>
#include<fstream>
#include<string>
//#define DEBUG
#ifdef DEBUG
#include<iostream>
#endif

using namespace std;
using namespace npbnlp;

static string err;
io::io():raw(new vector<unsigned int>) {
	head.push_back(0);
}

io::io(const char *file):raw(new vector<unsigned int>) {
	head.push_back(0);
	ifstream f(file);
	if (!f) {
		err = "couldn't open file:";
	        err += file;
		throw err.c_str();
	}
	string buf;
	while (getline(f, buf)) {
		swap_cr2ws(buf);
		/*
		if (!f.eof() && buf[buf.size()-1] != ' ')
			buf += ' ';
			*/
		try {
			s2i(buf.c_str(), *raw);
			head.push_back(raw->size());
		} catch (const char *ex) {
			err = ex;
		        err += " in s2i called by io constructor: argument = ";
			err += buf;
			throw err.c_str();
		}
	}
	// debug
#ifdef DEBUG
	for (auto i = raw->begin(); i != raw->end(); ++i) {
		char buf[5] = {0};
		i2c(*i, buf);
		cout << buf;
	}
	cout << endl;
#endif
}

io::io(istream& in):raw(new vector<unsigned int>) {
	head.push_back(0);
	if (!in) {
		err = "invalid input stream";
		throw err.c_str();
	}
	string buf;
	while (getline(in, buf)) {
		swap_cr2ws(buf);
		/*
		if (!in.eof() && buf[buf.size()-1] != ' ')
			buf += ' ';
			*/
		try {
			s2i(buf.c_str(), *raw);
			head.push_back(raw->size());
		} catch (const char *ex) {
			err = ex;
			err += " in s2i called by io constructor: argument = ";
			err += buf;
			throw err.c_str();
		}
	}
}

io::io(io&& f):raw(nullptr) {
	raw = f.raw;
	head = move(f.head);
	f.raw = nullptr;
	f.head.clear();
}

io::io(const io& f):raw(nullptr) {
	raw = f.raw;
	for (auto i = 0; i < (int)f.head.size(); ++i) {
		head.push_back(f.head[i]);
	}
}

io& io::operator=(io&& f) noexcept {
	if (this == &f)
		return *this;
	raw = f.raw;
	head = move(f.head);
	f.raw = nullptr;
	f.head.clear();
	return *this;
}

io& io::operator=(const io& f) {
	if (this == &f)
		return *this;
	raw = f.raw;
	for (auto i = 0; i < (int)f.head.size(); ++i) {
		head.push_back(f.head[i]);
	}
	return *this;
}

io::~io() {
}
