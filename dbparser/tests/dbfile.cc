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
	cerr << "# Generated makefile:" << endl;

	dg.DumpMakefile();

	return 0;
}



