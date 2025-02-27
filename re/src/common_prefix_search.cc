#include<iostream>
#include"da2.h"
#include"util.h"

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

void dump(const vector<vector<unsigned int> >& r) {
	for (auto& i : r) {
		for (auto& j: i) {
			char buf[5] = {0};
			io::i2c(j, buf);
			cout << buf;
		}
		cout << " ";
	}
	cout << endl;
}

int main(int argc, char **argv) {
	string index("trie.idx");
	if (argc > 1)
		index = *(argv+1);

	da<unsigned int, vector<vector<unsigned int> > > trie(3);
	trie.load(index.c_str(), vv_reader, uint_reader);
	string buf;
	while (getline(cin, buf)) {
		io::chomp(buf);
		vector<unsigned int> query;
		io::s2i(buf.c_str(), query);
		query.emplace_back(3); // 3:terminal code
		auto r = trie.cp_search(query);
		for (auto& i : r) {
			cout << "len:" << i.first << endl;
			auto v = trie.getval(i.second);
			if (v) {
				dump(v.value());
			}
		}

	}
	return 0;
}
