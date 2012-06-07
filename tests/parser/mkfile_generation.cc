#include <iostream>

#include "dbparser.h"
#include "exec.h"

using namespace std;

int main(int argc, char* argv[]) {
	/* read MakefileDB */
	const string make_command = "make";
	vector<string> args;
	args.push_back("-pq");
	pair<string, string> out = exec(make_command, args);

	/* create graph */
	DependencyGraph graph(out.first);

	/* get commands */
	const string delimiter = "3344543508980989031231";
	vector<string> basics;
	vector<string> targets;
	if (argc > 1)
		targets.push_back(argv[1]);
	graph.CountCommands(basics, delimiter, targets);

	/* dump makefile */
	graph.DumpMakefile(cout);

	return 0;
}
