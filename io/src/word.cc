#include"word.h"
#include<mutex>
#include<fstream>
#include<boost/serialization/vector.hpp>
#include<boost/serialization/unordered_map.hpp>
#include<boost/archive/text_oarchive.hpp>
#include<boost/archive/text_iarchive.hpp>

using namespace std;
using namespace npbnlp;

shared_ptr<wid> wid::_idx;
mutex wid::_mutex;
shared_ptr<vector<unsigned int> > wid::_letter;

shared_ptr<wid> wid::create() {
	lock_guard<mutex> lock(_mutex);
	if (_idx == nullptr) {
		_idx = shared_ptr<wid>(new wid(4/*2*/));
		_letter = make_shared<vector<unsigned int> >(); 
	}
	return _idx;
}

int wid::operator[](word& w) {
	if (_index.find(w) != _index.end())
		return _index[w];
	return 1;
}

int wid::index(word& w) {
	if (_index.find(w) == _index.end()) {
		lock_guard<mutex> m(_mutex);
		if (!_misn.empty()) {
			_index[w] = _misn[_misn.size()-1];
			_misn.pop_back();
		} else {
			_index[w] = _id++;
		}
	} else {
	}
	return _index[w];
}

void wid::remove(word& w) {
	auto it = _index.find(w);
	if (it != _index.end()) {
		lock_guard<mutex> m(_mutex);
		_misn.push_back(it->second);
		_index.erase(w);
	}
}

void wid::save(const char *file) {
	FILE *fp = NULL;
	if ((fp = fopen(file, "wb")) == NULL) {
		throw "failed to open file to save in wid::save";
	}
	if (fwrite(&_id, sizeof(int), 1, fp) != 1)
		throw "failed to write _id in wid::save";
	int size = _misn.size();
	if (fwrite(&size, sizeof(int), 1, fp) != 1)
		throw "failed to write _misn.size() in wid::save";
	if (fwrite(&_misn[0], sizeof(int), size, fp) != (size_t)size)
		throw "failed to write _misn in wid::save";
	_store(fp);
	fclose(fp);
	/*
	ofstream s(file);
	boost::archive::text_oarchive oa(s);
	oa << *this;
	*/
}

bool wid::load(const char *file) {
	FILE *fp = NULL;
	if ((fp = fopen(file, "rb")) == NULL)
		return false;
		//throw "failed to open file stored word indices in wid::load";
	if (fread(&_id, sizeof(int), 1, fp) != 1)
		throw "failed to read _id in wid::load";
	int size = 0;
	if (fread(&size, sizeof(int), 1, fp) != 1)
		throw "failed to read _misn.size() in wid::load";
	_misn.resize(size);
	if (fread(&_misn[0], sizeof(int), size, fp) != (size_t)size)
		throw "failed to read _misn in wid::load";
	int rawsize = 0;
	if (fread(&rawsize, sizeof(int), 1, fp) != 1)
		throw "failed to read _raw size in wid::load";
	_letter->resize(rawsize);
	if (fread(&(*_letter)[0], sizeof(unsigned int), (size_t)rawsize, fp) != (size_t)rawsize)
		throw "failed to read _raw in wid::load";
	int dicsize = 0;
	if (fread(&dicsize, sizeof(unsigned int), 1, fp) != 1)
		throw "failed to read size of indices in wid::load";
	for (int i = 0; i < dicsize; ++i) {
		word w;
		w.load(fp, *_letter);
		int id = 0;
		if (fread(&id, sizeof(int), 1, fp) != 1)
			throw "failed to read word id in wid::load";
		w.id = id;
		_index[w] = id;
	}
	fclose(fp);
	return true;
	/*
	if (access(file, F_OK) != -1) {
		ifstream l(file);
		boost::archive::text_iarchive ia(l);
		ia >> *_idx;
	}
	*/
}

void wid::_store(FILE *fp) {
	vector<word> d;
	vector<int> id;
	for (auto it = _index.begin(); it != _index.end(); ++it) {
		int head = _letter->size();
		word c(it->first);
		for (auto i = 0; i < c.len; ++i)
			_letter->push_back(c[i]);
		c.head = head;
		d.push_back(c);
		id.push_back(it->second);
	}
	// write raw data to fp
	int size = _letter->size();
	if (fwrite(&size, sizeof(int), 1, fp) != 1)
		throw "failed to write size of raw in wid::_store";
	if (fwrite(&(*_letter)[0], sizeof(unsigned int), size, fp) != (size_t)size)
		throw "failed to write raw data in wid::_store";
	int wsize = d.size();
	if (fwrite(&wsize, sizeof(int), 1, fp) != 1)
		throw "failed to write size of indices in wid::_store";
	for (auto i = 0; i < (int)d.size(); ++i) {
		d[i].save(fp);
		if (fwrite(&id[i], sizeof(int), 1, fp) != 1)
			throw "failed to write word id in wid::_store";
	}
}

wid::wid(int id):_id(id) {
}

wid::~wid() {
}

word::word(): head(0), len(1), id(0), pos(0), n(0), _doc(NULL) {
	m.resize(len+1, 0);
}

word::word(vector<unsigned int>& d): head(0), len(0), id(0), pos(0), n(0) {
	_doc = &d;
	m.resize(len+1,0);
}

word::word(vector<unsigned int>& d, int head, int len) :head(head), len(len), id(1), pos(0), n(0) {
	_doc = &d;
	m.resize(len+1, 0);
}

word::word(const word& w): head(w.head), len(w.len), id(w.id), pos(w.pos), n(w.n) {
	_doc = w._doc;
	for (auto i = w.m.begin(); i < w.m.end(); ++i)
		m.push_back(*i);
}

word::word(word&& w): head(w.head), len(w.len), id(w.id), pos(w.pos), n(w.n) {
	_doc = w._doc;
	m = move(w.m);
	/*
	for (auto i = w.m.begin(); i < w.m.end(); ++i)
		m.push_back(*i);
		*/
	w._doc = NULL;
	w.head = 0;
	w.len = 1;
	w.id = 0;
	w.pos = 0;
	w.n = 0;
	w.m.clear();
}


word& word::operator=(const word& w) {
	head = w.head;
	len = w.len;
	id = w.id;
	pos = w.pos;
	n = w.n;
	_doc = w._doc;
	m.resize(w.m.size());
	for (auto i = 0; i < (int)w.m.size(); ++i)
		m[i] = w.m[i];

	return *this;
}

word& word::operator=(word&& w) noexcept {
	if (this == &w) {
		return *this;
	}
	head = w.head;
	len = w.len;
	id = w.id;
	pos = w.pos;
	n = w.n;
	_doc = w._doc;
	m = move(w.m);
	/*
	m.resize(w.m.size());
	for (auto i = 0; i < w.m.size(); ++i)
		m[i] = w.m[i];
		*/
	w._doc = NULL;
	w.head = 0;
	w.len = 1;
	w.id = 0;
	w.pos = 0;
	w.n = 0;
	w.m.clear();

	return *this;
}

word::~word() {
}

void word::save(FILE *fp) {
	if (!fp)
		throw "found nullptr in word::save";
	if (fwrite(&head, sizeof(int), 1, fp) != 1)
		throw "failed to write word.len";
	if (fwrite(&len, sizeof(int), 1, fp) != 1)
		throw "failed to write word.len";
	if (fwrite(&id, sizeof(int), 1, fp) != 1)
		throw "failed to write word.id";
	if (fwrite(&pos, sizeof(int), 1, fp) != 1)
		throw "failed to write word.pos";
	if (fwrite(&n, sizeof(int), 1, fp) != 1)
		throw "failed to write word.n";
	int msize = m.size();
	if (fwrite(&msize, sizeof(int), 1, fp) != 1)
		throw "failed to write word.m.size()";
	if (fwrite(&m[0], sizeof(int), msize, fp) != (size_t)msize)
		throw "failed to write word.m";
}

void word::load(FILE *fp, vector<unsigned int>& r) {
	if (!fp)
		throw "found nullptr in word::load";
	if (fread(&head, sizeof(int), 1, fp) != 1)
		throw "failed to read word.len";
	if (fread(&len, sizeof(int), 1, fp) != 1)
		throw "failed to read word.len";
	if (fread(&id, sizeof(int), 1, fp) != 1)
		throw "failed to read word.id";
	if (fread(&pos, sizeof(int), 1, fp) != 1)
		throw "failed to read word.pos";
	if (fread(&n, sizeof(int), 1, fp) != 1)
		throw "failed to read word.n";
	int msize = 0;
	if (fread(&msize, sizeof(int), 1, fp) != 1)
		throw "failed to read word.m.size()";
	m.resize(msize);
	if (fread(&m[0], sizeof(int), msize, fp) != (size_t)msize)
		throw "failed to read word.m";
	_doc = &r;
}

unsigned int word::operator[](int i) const {
	if (!_doc || i < 0 || i >= len)
		return 0; // bow/eow
	return (*_doc)[head+i];
}
