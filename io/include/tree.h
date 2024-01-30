#ifndef NPBNLP_TREE_H
#define NPBNLP_TREE_H

#include"io.h"
#include"word.h"
namespace npbnlp {
	class tree {
		public:
			tree();
			tree(int k, tree *l, tree *r);
			tree(const tree& t);
			tree& operator=(const tree& t);
			tree(tree&& t) = default;
			tree& operator=(tree&& t) noexcept;
			virtual ~tree();
			int k;
			tree* left;
			tree* right;
		private:
			int _head;
			int _len;
	};
}

#endif
