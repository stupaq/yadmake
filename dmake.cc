
#include <string>
#include <stdexcept>
#include <iostream>
#include <utility>
#include <vector>
#include <fstream>
#include <boost/foreach.hpp>
#include <iostream>
#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>

#include "dbparser.h"
#include "worker.h"
#include "dispatcher.h"


int main()
{
	Messaging* m = new Messaging();
	Messaging* m2 = new Messaging();
	std::vector<Worker*> workers = get_workers(m);


	/* Here do the whole job */
	DependencyGraph dep_graph(dup(0));
	std::vector<std::string> basics;
	basics.push_back("make");
	dep_graph.CountCommands(basics, "blah");
	Dispatcher(dep_graph, workers, m2);


	BOOST_FOREACH(Worker* w, workers)
		delete w;
	delete m;
	delete m2;

	return 0;
}

