#ifndef NPBNLP_CTYPE_H
#define NPBNLP_CTYPE_H

#include<unicode/utypes.h>
#include<unicode/uchar.h>
#include<unicode/unistr.h>
#include<unicode/uscript.h>

namespace npbnlp {
	enum type {
		U_HIRAGANA,
		U_KATAKANA,
		U_KATA_OR_HIRA,
		U_HANJI,
		U_HIRA_KATA,
		U_HIRA_HANJI,
		U_KATA_HANJI,
		U_HIRA_KATA_HANJI,
		U_MISC,
		U_ARABIC,
		U_GREEK,
		U_HANGUL,
		U_HEBREW,
		U_LATIN,
		U_MYANMAR,
		U_THAI,
		U_DIGIT,
		U_SYNBOL
	};
	class chartype {
		public:
			const static int n = 18;
			static type get(unsigned int c);
		protected:
	};
}

#endif
