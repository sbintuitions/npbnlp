#include"io.h"
#include<iostream>

using namespace std;
using namespace npbnlp;

int main(int argc, char **argv) {
	try {
		io f(*(argv+1));
	} catch (const char *ex) {
		cerr << ex << endl;
		return 1;
	}
	return 0;
}
