#include <iostream>
#include <boost/foreach.hpp>
#include <signal.h>

#include "dbparser.h"
#include "exec.h"
#include "worker.h"
#include "dispatcher.h"
#include "options.h"

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

	if (setenv("LANG", "en_US.UTF-8", 1) == -1) {
		fprintf(stderr, "Cannot change environment");
		exit(1);
	}

	try {
		/*get options */
		options r = prepare_options(argc, argv);

		system(r.exec.c_str());

		if (r.dist_make) {

			/* create database */
			const string make_command = "make";
			vector<string> args;
			args.push_back("-pq");
			pair<string, string> out = exec(make_command, args);

			/* create graph */
			DependencyGraph graph(out.first);

			/* get commands */
			const string delimiter = "3344543508980989031231";
			graph.CountCommands(r.forward, delimiter, r.targets);

			/* dump makefile */
			graph.DumpMakefile(cout);

			/* create messaging, workers, not ready to use */
			m = new Messaging();
			workers = get_workers(m);

			/* run dispatcher */
			Dispatcher(graph, workers, m, r.keep_going);

			/* delete workers, messaging */
			BOOST_FOREACH(Worker *w, workers)
			delete w;
			delete m;
		}
	}

	catch (CircularDependency E) {
		handler(1);
	} catch (MakeError E) {
		handler(1);
	} catch (SystemError E) {
		handler(1);
	} catch (runtime_error E) {
		fprintf(stderr, "%s\n", E.what());
		handler(1);
	}


	return 0;
}
