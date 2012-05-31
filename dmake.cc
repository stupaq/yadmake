
#include <string>
#include <stdexcept>
#include <iostream>
#include <utility>
#include <vector>
#include <fstream>
#include <boost/foreach.hpp>
#include <iostream>

#include "dbparser.h"
#include "worker.h"
#include "dispatcher.h"


int main()
{
	Messaging* m = new Messaging();
	std::vector<Worker*> workers = get_workers(m);


	/* Here do the whole job */


	BOOST_FOREACH(Worker* w, workers)
		delete w;
	delete m;

	return 0;
}

