#include"chunk.h"

using namespace std;
using namespace npbnlp;

static word eos;
shared_ptr<cid> cid::_idx;
mutex cid::_mutex;
shared_ptr<vector<unsigned int> > cid::_letter;
shared_ptr<vector<word> > cid::_word;

shared_ptr<cid> cid::create() {
	lock_guard<mutex> lock(_mutex);
	if (_idx == nullptr) {
		_idx = shared_ptr<cid>(new cid(2));
		_word = make_shared<vector<word> >();
		_letter = make_shared<vector<unsigned int> >();
	}
	return _idx;
}

int cid::operator[](chunk& c) {
	if (_index.find(c) != _index.end())
		return _index[c];
	return 1;
}

int cid::index(chunk& c) {
	if (_index.find(c) == _index.end()) {
		lock_guard<mutex> m(_mutex);
		if (!_misn.empty()) {
			_index[c] = _misn[_misn.size()-1];
			_misn.pop_back();
		} else {
			_index[c] = _id++;
		}
	}
	return _index[c];
}

void cid::remove(chunk& c) {
	auto it = _index.find(c);
	if (it != _index.end()) {
		lock_guard<mutex> m(_mutex);
		_misn.push_back(it->second);
		_index.erase(c);
	}
}

void cid::save(const char *file) {
	FILE *fp = NULL;
	if ((fp = fopen(file, "wb")) == NULL) {
		throw "failed to open file to save in cid::save";
	}
	if (fwrite(&_id, sizeof(int), 1, fp) != 1)
		throw "failed to write _id in cid::save";
	int size = _misn.size();
	if (fwrite(&size, sizeof(int), 1, fp) != 1)
		throw "failed to write _misn.size() in cid::save";
	if (fwrite(&_misn[0], sizeof(int), size , fp) != size)
		throw "failed to write _misn in cid::save";
	_store(fp);
	fclose(fp);
}

bool cid::load(const char *file) {
	FILE *fp = NULL;
	if ((fp = fopen(file, "rb")) == NULL) {
		return false;
	}
	if (fread(&_id, sizeof(int), 1, fp) != 1)
		throw "failed to read _id in cid::load";
	int size = 0;
	if (fread(&size, sizeof(int), 1, fp) != 1)
		throw "failed to read _misn.size() in cid::load";
	_misn.resize(size);
	if (fread(&_misn[0], sizeof(int), size, fp) != size)
		throw "failed to read _misn in cid::load";
	int rawsize = 0;
	if (fread(&rawsize, sizeof(int), 1, fp) != 1)
		throw "failed to read _letter->size() in cid::load";
	_letter->resize(rawsize);
	if (fread(&(*_letter)[0], sizeof(unsigned int), rawsize, fp) != rawsize)
		throw "failed to read _letter in cid::load";
	int wsize = 0;
	if (fread(&wsize, sizeof(int), 1, fp) != 1)
		throw "failed to read size of words in cid::load";
	for (int i = 0; i < wsize; ++i) {
		word w;
		w.load(fp, *_letter);
		_word->push_back(w);
	}
	int csize = 0;
	if (fread(&csize, sizeof(int), 1, fp) != 1)
		throw "failed to read size of chunk indices in cid::load";
	for (int i = 0; i < csize; ++i) {
		chunk c;
		c.load(fp, *_word);
		int id = 0;
		if (fread(&id, sizeof(int), 1, fp) != 1)
			throw "failed to read chunk id in cid::load";
		c.id = id;
		_index[c] = id;
	}
	fclose(fp);
	return true;
}

void cid::_store(FILE *fp) {
	vector<chunk> d;
	vector<int> id;
	for (auto it = _index.begin(); it != _index.end(); ++it) {
		chunk c(it->first);
		id.push_back(it->second);
		c.head = _word->size();
		for (auto i = 0; i < c.len; ++i) {
			word& w = c.wd(i);
			w.head = _letter->size();
			for (auto j = 0; j < w.len; ++j) {
				_letter->push_back(w[j]);
			}
			_word->push_back(w);
		}
		d.push_back(c);
	}
	int lsize = _letter->size();
	if (fwrite(&lsize, sizeof(int), 1, fp) != 1)
		throw "failed to write size of raw in cid::_store";
	if (fwrite(&(*_letter)[0], sizeof(unsigned int), lsize, fp) != lsize)
		throw "failed to write raw data in cid::_store";
	int wsize = _word->size();
	if (fwrite(&wsize, sizeof(int), 1, fp) != 1)
		throw "failed to write size of words in cid::_store";
	for (auto it = _word->begin(); it != _word->end(); ++it) {
		it->save(fp);
	}
	int csize = d.size();
	if (fwrite(&csize, sizeof(int), 1, fp) != 1)
		throw "failed to write size of chunks in cid::_store";
	for (auto i = 0; i < d.size(); ++i) {
		d[i].save(fp);
		if (fwrite(&id[i], sizeof(int), 1, fp) != 1)
			throw "failed to write chunk id in cid::_store";
	}
}

cid::cid(int id):_id(id) {
}

chunk::chunk():k(0), head(0), len(1), id(0), n(len+1,0), _doc(NULL)  {
}

chunk::chunk(vector<word>& d): k(0), head(0), len(0), id(0), n(len+1,0), _doc(&d) {
}

chunk::chunk(vector<word>& d, int head, int len): k(0), head(head), len(len), id(1), n(len+1,0), _doc(&d) {
}

chunk::chunk(const chunk& c): k(c.k), head(c.head), len(c.len), id(c.id), _doc(c._doc)  {
	for (auto i = c.n.begin(); i < c.n.end(); ++i)
		n.push_back(*i);
}

chunk::chunk(chunk&& c): k(c.k), head(c.head), len(c.len), id(c.id), _doc(c._doc) {
	n = move(c.n);
	c.k = 0;
	c.head = 0;
	c.len = 1;
	c.id = 0;
	c._doc = nullptr;
	/*
	for (auto i = c.n.begin(); i < c.n.end(); ++i)
		n.push_back(*i);
		*/
}

chunk& chunk::operator=(const chunk& c) {
	k = c.k;
	head = c.head;
	len = c.len;
	id = c.id;
	_doc = c._doc;
	for (auto i = c.n.begin(); i < c.n.end(); ++i)
		n.push_back(*i);
	return *this;
}

chunk& chunk::operator=(chunk&& c) noexcept {
	if (this == &c)
		return *this;
	k = c.k;
	head = c.head;
	len = c.len;
	id = c.id;
	_doc = c._doc;
	n = move(c.n);
	c.k = 0;
	c.head = 0;
	c.len = 1;
	c.id = 0;
	c._doc = nullptr;
	/*
	for (auto i = c.n.begin(); i < c.n.end(); ++i)
		n.push_back(*i);
		*/
	return *this;
}

chunk::~chunk() {
}

int chunk::operator[](int i) const {
	if (!_doc || i < 0 || i >= len)
		return 0;
	return (*_doc)[head+i].id;
}

word& chunk::wd(int i) const {
	if (!_doc || i < 0 || i >= len)
		return eos; // bos/eos
	return (*_doc)[head+i];
}

void chunk::save(FILE *fp) {
	if (!fp)
		throw "found nullptr in chunk::save";
	if (fwrite(&k, sizeof(int), 1, fp) != 1)
		throw "failed to write chunk.k";
	if (fwrite(&head, sizeof(int), 1, fp) != 1)
		throw "failed to write chunk.head";
	if (fwrite(&len, sizeof(int), 1, fp) != 1)
		throw "failed to write chunk.len";
	if (fwrite(&id, sizeof(int), 1, fp) != 1)
		throw "failed to write chunk.id";
}

void chunk::load(FILE *fp, vector<word>& w) {
	if (!fp)
		throw "found nullptr in chunk::load";
	if (fread(&k, sizeof(int), 1, fp) != 1)
		throw "failed to read chunk.k";
	if (fread(&head, sizeof(int), 1, fp) != 1)
		throw "failed to read chunk.head";
	if (fread(&len, sizeof(int), 1, fp) != 1)
		throw "failed to read chunk.len";
	if (fread(&id, sizeof(int), 1, fp) != 1)
		throw "failed to read chunk.id";
	_doc = &w;
}
