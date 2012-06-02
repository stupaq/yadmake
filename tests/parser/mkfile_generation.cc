#include <iostream>

#include "dbparser.h"
#include "exec.h"

using namespace std;

int main(int argc, char* argv[]) {
	/* read MakefileDB */
	const string make_command = "make";
	vector<string> args;
	//TODO buggy exec
	args.push_back(make_command);
	args.push_back("-pq");
	if (argc > 1) {
		args.push_back("-C");
		args.push_back(argv[1]);
	}
	pair<string, string> out = exec(make_command, args);

	cerr << out.first << endl;

	/* create graph */
	DependencyGraph graph(out.first);

	graph.DumpMakefile(cout);
	return 0;

	/* get commands */
	const string delimiter = "16932741623941281427349182364";
	vector<string> targets;
	graph.CountCommands(targets, delimiter);

	/* dump makefile */
	graph.DumpMakefile(cout);

	return 0;
}
