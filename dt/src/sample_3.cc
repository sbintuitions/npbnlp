#include<algorithm>
#include<vector>
#include<iostream>
#include<bits/stdc++.h>
#include"util.h"
//#include"da.h"
#include"da2.h"

using namespace std;
using namespace npbnlp;

void uint_writer(unsigned int& c, FILE *fp) {
	fwrite(&c, sizeof(unsigned int), 1, fp);
}

void uint_reader(unsigned int& c, FILE *fp) {
	fread(&c, sizeof(unsigned int), 1, fp);
}

void vv_writer(vector<vector<unsigned int> >& v, FILE *fp) {
	size_t size = v.size();
	fwrite(&size, sizeof(size_t), 1, fp);
	for (auto& i : v) {
		size_t s = i.size();
		fwrite(&s, sizeof(size_t), 1, fp);
		fwrite(&i[0], sizeof(unsigned int), s, fp);
	}
}

void vv_reader(vector<vector<unsigned int> >& v, FILE *fp) {
	size_t size = 0;
	fread(&size, sizeof(size_t), 1, fp);
	v.resize(size);
	for (auto i = 0; i < (int)size; ++i) {
		size_t s = 0;
		fread(&s, sizeof(size_t), 1, fp);
		v[i].resize(s);
		fread(&v[i][0], sizeof(unsigned int), s, fp);
	}
}

int main(int argc, char **argv) {
	if (argc < 2)
		return 1;
	io f(*(argv+1));
	const char *delim = "\t";
	unsigned int d = io::c2i(delim, io::u8size(delim));
	vector<vector<unsigned int> > keys;
	vector<vector<vector<unsigned int> > > values;

	da<unsigned int, vector<vector<unsigned int> > > trie(0);
	int id = 0;
	for (auto i = 0; i < (int)f.head.size()-1; ++i) {
		int p = util::find(d, *f.raw, f.head[i], f.head[i+1]);
		int key_len = p-f.head[i];
		vector<unsigned int> k;
		for (auto j = 0; j < key_len; ++j) {
			k.emplace_back((*f.raw)[f.head[i]+j]);
		}
		k.push_back(0);
		keys.push_back(k);
		values.resize(keys.size());
		while (p < f.head[i+1]) {
			int next = util::find(d, *f.raw, p+1, f.head[i+1]);
			int vlen = next-p-1;
			vector<unsigned int> v;
			for (auto j = 0; j < vlen; ++j) {
				v.emplace_back((*f.raw)[p+j+1]);
			}
			values[id].emplace_back(v);
			p = next;
		}
		trie.insert(keys[id], values[id]);
		++id;
	}
	//trie.build(keys, values);
	trie.save("sample3.idx", vv_writer, uint_writer);
	/*
	cout << "finished build\n" << endl;
	// check and common prefix search
	da<unsigned int, vector<vector<unsigned int> > > check(0);
	check.load("sample3.idx", vv_reader, uint_reader);
	cout << "input query" << endl;
	string buf;
	while (getline(cin, buf)) {
		io::chomp(buf);
		vector<unsigned int> query;
		io::s2i(buf.c_str(), query);
		query.push_back(0);
		auto r = check.cp_search(query);
		for (auto& i : r) {
			cout << "len:" << i.first << endl;
			auto v = check.getval(i.second);
			if (v) {
				for (auto& j : v.value()) {
					for (auto& c : j) {
						char buf[5] = {0};
						io::i2c(c, buf);
						cout << buf;
					}
					cout << " ";
				}
				cout << endl;
			}
		}
	}
	*/
	return 0;
}
