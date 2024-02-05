#ifndef NPBNLP_TREE_H
#define NPBNLP_TREE_H

#include"io.h"
#include"word.h"
namespace npbnlp {
	class node {
		public:
			node();
			virtual ~node();
			int k;
			int i;
			int j;
	};
	class tree {
		public:
			tree(sentence& s);
			tree(const tree& t);
			tree& operator=(const tree& t);
			tree(tree&& t) = default;
			tree& operator=(tree&& t) noexcept;
			node& operator[](int i);
			word& wd(int i);
			virtual ~tree();
			std::vector<node> c;
			sentence s;
		private:
	};
}

#endif
