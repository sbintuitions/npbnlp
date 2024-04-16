#ifndef NPBNLP_UTIL_H
#define NPBNLP_UTIL_H

#include<algorithm>
#include<cstring>
#include"io.h"
#include"word.h"
#include"chunk.h"
#include"sentence.h"
#include"nsentence.h"
namespace npbnlp {
	class util {
		public:
		// delimiter
		// ':' = 58
		// ' ' = 32
		static int find(unsigned int delimiter, std::vector<unsigned int>& r, int offset, int end) {
			if (offset >= (int)r.size())
				return r.size();
			auto it = std::find(r.begin()+offset, r.begin()+end, delimiter);
			if (it != r.end())
				return it - r.begin();
			else
				return end;
				//return r.size();
		}
		static int store_word(word& w, std::vector<unsigned int>& r, int head, int tail) {
			w.head = head;
			//int p = util::find(32, r, head);
			int p = util::find(32, r, head, tail);
			w.len = p - head;
			//int pos = 0;
			/*
			if ((pos = util::find(58, r, head)) < w.head+w.len) {
				std::string num;
				for (int i = pos+1; i < w.head+w.len; ++i) {
					char buf[5] = {0};
					io::i2c(r[i], buf);
					num += buf;
				}
				w.len = pos - head;
				w.pos = std::atoi(num.c_str());
			}
			*/
			w.m.resize(w.len+1, 0);
			return p+1;
		}
		static void store_sentences(io& f, std::vector<sentence>& corpus) {
			if (f.head.empty())
				return;
			for (int i = 0; i < (int)f.head.size()-1; ++i) {
				int s_head = f.head[i];
				int s_tail = f.head[i+1];
				sentence s(*f.raw, s_head, s_tail);
				corpus.push_back(s);
			}
		}

	};
}
#endif
