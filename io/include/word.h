#ifndef NPBNLP_WORD_H
#define NPBNLP_WORD_H

#include"io.h"
#include<unordered_map>
#include<iostream>
#include<mutex>
#include<boost/serialization/vector.hpp>
#include<boost/serialization/unordered_map.hpp>
#include<boost/archive/text_oarchive.hpp>
#include<boost/archive/text_iarchive.hpp>
namespace npbnlp {
	class chunk;
	class word {
		public:
			word();
			word(std::vector<unsigned int>& d);
			word(std::vector<unsigned int>& d, int head, int len);
			word(const word& w);
			word(word&& w);
			word& operator=(const word& w);
			word& operator=(word&& w) noexcept;
			void save(FILE *fp);
			void load(FILE *fp, std::vector<unsigned int>& d);
			virtual ~word();
			unsigned int operator[](int i) const;
			int head;
			int len;
			int id;
			int pos;
			int n;
			std::vector<int> m; // for ngram order for letters
			friend std::ostream& operator<<(std::ostream& os, const word& w) {
				for (auto i = 0; i < w.len; ++i) {
					char buf[5] = {0};
					io::i2c(w[i], buf);
					os << buf;
				}
				if (w.id > 0) 
					os << ":" << w.id;
				if (w.pos > 0)
					os << ":" << w.pos;
				if (w.n > 0)
					os << ":" << w.n;
				return os;
			}
		protected:
			std::vector<unsigned int> *_doc;
		friend class boost::serialization::access;
		template<class Archive>
			void serialize(Archive& ar, unsigned int version) {
				std::shared_ptr<std::vector<unsigned int> > cache = std::make_shared<std::vector<unsigned int> >();
				cache->resize(len);
				for (auto i = 0; i < len; ++i)
					(*cache)[i] = (*_doc)[head+i];
				_doc = &(*cache);
				head = 0;
				ar& head;
				ar& len;
				ar& id;
				ar& pos;
				ar& n;
				ar& m;
				ar& *_doc;
			}
	};
	struct whash {
		size_t operator() (const word& w) const {
			size_t seed = w.len;
			for (int i = 0; i < w.len; ++i) {
				auto x = w[i];
				x = ((x >> 16) ^ x) * 0x45d9f3b;
				x = ((x >> 16) ^ x) * 0x45d9f3b;
				x = (x >> 16) ^ x;
				seed ^= x + 0x9e3779b9 + (seed << 6) + (seed >> 2);
			}
			return seed;
		}
		/*
		size_t operator() (const word& w) const {
			size_t id = 19780211;
			const int size = w.len;
			for (int i = 0; i < size; ++i)
				id = id * 37 * w[i];
			return id;
		}
		*/
	};
	struct wcmp {
		bool operator() (const word& a, const word& b) const {
			if (a.len != b.len)
				return false;
			int i = 0;
			for (; a[i] == b[i] && i < a.len; ++i);
			return (i == a.len);
		}
	};
	class wid {
		using wdic = std::unordered_map<word, int, whash, wcmp>;
		private:
			static std::shared_ptr<wid> _idx;
			static std::mutex _mutex;
			static std::shared_ptr<std::vector<unsigned int> > _letter;
			int _id;
			wdic _index;
			std::vector<int> _misn;
			void _store(FILE *fp);
		protected:
			wid(int id);
		friend class cid;
		public:
			wid(const wid& d) = delete;
			wid& operator=(const wid& d) = delete;
			static std::shared_ptr<wid> create();
			int operator[](word& w);
			int index(word& w);
			void remove(word& w);
			void save(const char *file);
			bool load(const char *file);
			virtual ~wid();
		friend class boost::serialization::access;
		template<class Archive>
			void serialize(Archive& ar, unsigned int version) {
				ar& _id;
				ar& _index;
				ar& _misn;
			}
	};
}
#endif
