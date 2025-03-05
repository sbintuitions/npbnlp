#ifndef NPBNLP_YOMI_LATTICE_H
#define NPBNLP_YOMI_LATTICE_H

#include"io.h"
#include"da2.h"
#include"word.h"
#include<vector>

namespace npbnlp {
	class ynode {
		public:
			ynode(word& w, std::vector<unsigned int>& p);
			virtual ~ynode();
			ynode(const ynode& n);
			ynode& operator=(const ynode& n);
			word w;
			std::vector<unsigned int> phonetic;
			double prod;
			double bos;
			double eos;
	};

	class ylattice {
		using trie = da<unsigned int, std::vector<std::vector<unsigned int> > >;
		public:
			ylattice(io& f, int i, trie& t, trie* u);
			virtual ~ylattice();
			std::vector<unsigned int> query;
			std::vector<std::vector<ynode> > y;
			int size();
			int size(int i);
			int len(int i, int j);
			ynode& get(int i, int j);
			ynode* getp(int i, int j);
	};
}

#endif
