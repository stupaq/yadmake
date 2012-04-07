#include <cstdio>

#include <vector>
#include <string>
#include <iostream>
#include <queue>

#include "../dbparser.h"
#include "../commands.h"

using namespace std;

int main() {

	DependencyGraph dg(cin);

	vector<string> b;
	b.push_back("make");
	string d = "blah";
	count_commands(&dg, b, d);

	cout << "# Generated makefile:" << endl;
	dg.DumpMakefile(cout);

	return 0;
}



