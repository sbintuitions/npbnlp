//#include"da.h"
#include"da2.h"
#include<fstream>
#include<string>
#include<iostream>

using namespace std;
using namespace npbnlp;

void char_writer(char& c, FILE *fp) {
	fwrite(&c, sizeof(char), 1, fp);
}

void char_reader(char& c, FILE *fp) {
	fread(&c, sizeof(char), 1, fp);
}

void writer(int& i, FILE *fp) {
	fwrite(&i, sizeof(int), 1, fp);
}

void reader(int& i, FILE *fp) {
	fread(&i, sizeof(int), 1, fp);
}

int main(int argc, char **argv) {
	da<char, int> trie('\0');
	vector<vector<char> > keys;
	vector<int> vals;

	ifstream f(*(argv+1));
	string buf;
	int i = 0;
	while (getline(f, buf)) {
		vector<char> k;
		for (auto& c : buf) {
			k.emplace_back(c);
		}
		k.emplace_back('\0');
		keys.emplace_back(k);
		vals.emplace_back(i++);
	}
	cout << "keysize:" << i << endl;
	trie.build(keys, vals);
	trie.save("trie.idx", writer, char_writer);

	da<char, int> check('\0');
	check.load("trie.idx", reader, char_reader);
	for (auto& k : keys) {
		long id = check.exactmatch(k);
		if (id > 0) {
			optional<int> v = check.getval(id);
			//cout << id << " val:" << v.value() << endl;;
		} else {
			cout << "id:" << id << " key:";
			for (auto& c : k) {
				cout << c;
			}
			cout << " is not found" << endl;
		}
	}
	return 0;
}
