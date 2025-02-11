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

	ifstream f(*(argv+1));
	string buf;
	int i = 0;
	while(getline(f, buf)) {
		vector<char> k;
		for (auto& c : buf) {
			k.emplace_back(c);
		}
		k.emplace_back('\0');
		/*
		cout << "insert ";
		for (auto& c : k) {
			cout << c;
		}
		cout << " val:" << i << endl;
		*/
		trie.insert(k, i);
		keys.emplace_back(k);
		if (trie.exactmatch(keys[0]) < 0) {
			exit(1);
		}
		++i;
	}
	cout << "keysize:" << i << endl;
	trie.save("sample2.idx", writer, char_writer);

	/*
	da<char, int> check('\0');
	check.load("sample2.idx", reader, char_reader);
	int j = 0;
	for (auto& k : keys) {
		long id = check.exactmatch(k);
		if (id > 0) {
			optional<int> v = check.getval(id);
			//cout << id << " val:" << v.value() << endl;
		} else {
			cout << "id:" << id << " key:";
			for (auto& c : k)
				cout << c;
			cout << " is not found" << endl;
		}
		check.erase(k);
		id = check.exactmatch(k);
		if (id > 0) {
			cout << "failed to erase id:" << id << endl;
			cout << "key:";
			for (auto& c : k)
				cout << c;
			cout << endl;
		} else {
			++j;
		}
		cout << "erase:";
		for (auto& c : k) {
			cout << c;
		}
	       	cout << " id:" << id << endl;
	}
	cout << "erase " << j << " record" << endl;
	*/
	return 0;
}
