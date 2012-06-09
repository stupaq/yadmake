#include <iostream>
#include <boost/foreach.hpp>

#include "dbparser.h"
#include "exec.h"
#include "worker.h"
#include "dispatcher.h"

using namespace std;

int main(int argc, char* argv[]) {
	const string make_command = "make";
	vector<string> args;
  vector<Worker *> workers;
  Messaging *m;

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

  /* create messaging, workers, not ready to use */
  m = new Messaging();
  workers = get_workers(m);

	/* run dispatcher */
	Dispatcher(graph, workers, m, false);

  /* delete workers, messaging */
  BOOST_FOREACH(Worker *w, workers)
    delete w;

  delete m;

	return 0;
}
