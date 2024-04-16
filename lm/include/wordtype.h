#ifndef NPBNLP_WTYPE_H
#define NPBNLP_WTYPE_H

#include"chartype.h"
#include"word.h"

namespace npbnlp {
	class wordtype {
		public:
			static type get(word& w) {
				type t = chartype::get(w[0]);
				for (int i = 1; i < w.len; ++i) {
					npbnlp::type u = chartype::get(w[i]);
					switch (u) {
						case U_HIRAGANA:
							if (t == U_KATAKANA)
								t = U_HIRA_KATA;
							else if (t == U_HANJI)
								t = U_HIRA_HANJI;
							else if (t == U_KATA_HANJI)
								t = U_HIRA_KATA_HANJI;
							else if (t == U_KATA_OR_HIRA)
								t = U_HIRAGANA;
							else if (t == U_HIRA_HANJI || t == U_HIRA_KATA || t == U_HIRA_KATA_HANJI)
								;
							else if (u != t)
								return U_MISC;
							break;
						case U_KATAKANA:
							if (t == U_HIRAGANA)
								t = U_HIRA_KATA;
							else if (t == U_HANJI)
								t = U_KATA_HANJI;
							else if (t == U_HIRA_HANJI)
								t = U_HIRA_KATA_HANJI;
							else if (t == U_KATA_OR_HIRA)
								t = U_KATAKANA;
							else if (t == U_HIRA_KATA || t == U_KATA_HANJI || t == U_HIRA_KATA_HANJI) 
								;
							else if (u != t)
								return U_MISC;
							break;
						case U_KATA_OR_HIRA:
							if (t != U_HIRAGANA && t != U_KATAKANA && t != U_HIRA_KATA && t != U_KATA_HANJI && t != U_HIRA_HANJI && t != U_HIRA_KATA_HANJI)
								return U_MISC;
							break;
						case U_HANJI:
							if (t == U_HIRAGANA)
								t = U_HIRA_HANJI;
							else if (t == U_KATAKANA)
								t = U_KATA_HANJI;
							else if (t == U_HIRA_KATA)
								t = U_HIRA_KATA_HANJI;
							else if (t == U_KATA_OR_HIRA)
								t = U_HIRA_KATA_HANJI;
							else if (t == U_HIRA_HANJI || t == U_KATA_HANJI || t == U_HIRA_KATA_HANJI) 
								;
							else if (u != t)
								return U_MISC;
							break;
						default:
							if (t != u)
								return U_MISC;
					}
				}
				return t;
			}
			static type get(std::vector<unsigned int>& w) {
				int i = 0;
				for (; w[i] == 0; ++i);
				type t = chartype::get(w[i]);
				for (; i < (int)w.size(); ++i) {
					if (w[i] == 0)
						break;
					type u = chartype::get(w[i]);
					switch (u) {
						case U_HIRAGANA:
							if (t == U_KATAKANA)
								t = U_HIRA_KATA;
							else if (t == U_HANJI)
								t = U_HIRA_HANJI;
							else if (t == U_KATA_HANJI)
								t = U_HIRA_KATA_HANJI;
							else if (t == U_KATA_OR_HIRA)
								t = U_HIRAGANA;
							else if (t == U_HIRA_HANJI || t == U_HIRA_KATA || t == U_HIRA_KATA_HANJI)
								;
							else if (u != t)
								return U_MISC;
							break;
						case U_KATAKANA:
							if (t == U_HIRAGANA)
								t = U_HIRA_KATA;
							else if (t == U_HANJI)
								t = U_KATA_HANJI;
							else if (t == U_HIRA_HANJI)
								t = U_HIRA_KATA_HANJI;
							else if (t == U_KATA_OR_HIRA)
								t = U_KATAKANA;
							else if (t == U_HIRA_KATA || t == U_KATA_HANJI || t == U_HIRA_KATA_HANJI) 
								;
							else if (u != t)
								return U_MISC;
							break;
						case U_KATA_OR_HIRA:
							if (t != U_HIRAGANA && t != U_KATAKANA && t != U_HIRA_KATA && t != U_KATA_HANJI && t != U_HIRA_HANJI && t != U_HIRA_KATA_HANJI)
								return U_MISC;
							break;
						case U_HANJI:
							if (t == U_HIRAGANA)
								t = U_HIRA_HANJI;
							else if (t == U_KATAKANA)
								t = U_KATA_HANJI;
							else if (t == U_HIRA_KATA)
								t = U_HIRA_KATA_HANJI;
							else if (t == U_KATA_OR_HIRA)
								t = U_HIRA_KATA_HANJI;
							else if (t == U_HIRA_HANJI || t == U_KATA_HANJI || t == U_HIRA_KATA_HANJI) 
								;
							else if (u != t)
								return U_MISC;
							break;
						default:
							if (t != u)
								return U_MISC;
					}
				}
				return t;
			}
		protected:
	};
}

#endif
