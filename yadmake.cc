#include <iostream>
#include <boost/foreach.hpp>
#include <signal.h>

#include "dbparser.h"
#include "exec.h"
#include "worker.h"
#include "dispatcher.h"

using namespace std;

vector<Worker *> workers;
Messaging *m = NULL;

void clean_all() {
	rename("DMakefile", "Makefile");
	BOOST_FOREACH(Worker *w, workers)
		delete w;
	if (m != NULL)
		delete m;
}

void handler(int sig_number) {
	clean_all();
	exit(sig_number);
}

void catch_signals() {
	struct sigaction action;
	action.sa_handler = handler;
	action.sa_flags = 0;
	sigaction(SIGINT, &action, NULL);
	sigaction(SIGTERM, &action, NULL);
}

int main(int argc, char* argv[]) {

	/* clean everything after a signal */
	catch_signals();

	const string make_command = "make";
	vector<string> args;

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
