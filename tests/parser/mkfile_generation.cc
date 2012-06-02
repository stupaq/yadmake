#include <iostream>

#include "dbparser.h"
#include "exec.h"

using namespace std;

int main(int argc, char* argv[]) {
	/* read MakefileDB */
	const string make_command = "make";
	vector<string> args;
	//TODO buggy exec
	args.push_back("-pq");
	if (argc > 1) {
		args.push_back("-C");
		args.push_back(argv[1]);
	}
	pair<string, string> out = exec(make_command, args);

	/* create graph */
	DependencyGraph graph(out.first);

	/* get commands */
	const string delimiter = "3344543508980989031231";
	vector<string> basics;
	graph.CountCommands(basics, delimiter);

	/* dump makefile */
	graph.DumpMakefile(cout);

	return 0;
}
