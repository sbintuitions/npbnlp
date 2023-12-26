#include<unicode/utypes.h>
#include<unicode/uchar.h>
#include<unicode/ucnv.h>
#include<unicode/unistr.h>
#include<unicode/uscript.h>
#include<iostream>

using namespace std;
using namespace icu;

int main(int argc, char **argv) {
	if (argc < 2) {
		return 0;
	}
	UnicodeString u(*(argv+1), "utf8");
	UChar32 c = u.char32At(0);

	UErrorCode err = U_ZERO_ERROR;
	UBlockCode b = ublock_getCode(c);
	UScriptCode s = uscript_getScript(c, &err);
	int32_t wb = u_getIntPropertyValue(c, UCHAR_WORD_BREAK);
	int32_t bp = u_getIntPropertyValue(c, UCHAR_BLOCK);
	int8_t t = u_charType(c);

	char name[1024];
	UCharNameChoice nc = U_UNICODE_CHAR_NAME;
	u_charName(c, nc, name, 1024, &err);
	cout << "charname:" << name << endl;
	cout << "uscript:" << s << ":" << uscript_getName(s) << endl;
	cout << "block property:" << u_getPropertyValueName(UCHAR_BLOCK, bp, U_SHORT_PROPERTY_NAME) << endl;
	cout << "word break property:" << u_getPropertyValueName(UCHAR_WORD_BREAK, wb, U_SHORT_PROPERTY_NAME) << endl;
	cout << "unicode 1 name:" << u_getPropertyName(UCHAR_UNICODE_1_NAME, U_SHORT_PROPERTY_NAME) << endl;


	return 0;
}
