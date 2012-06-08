#include <iostream>

#include <boost/foreach.hpp>

#include "worker.h"
#include "dbparser.h"

#define UNUSED(x) ((void) (x))

using namespace std;

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE remote/execute_students
#define BOOST_TEST_IGNORE_NON_ZERO_CHILD_CODE
#include <boost/test/unit_test.hpp>

/* Necessary for auto Makefile to build this file
int main( */

BOOST_AUTO_TEST_CASE(ready_benchmark) {

	const string cwd = "~/Downloads/";
	const string ssh_config = "./tests/remote/ssh_config";

	vector<string> ssh_hosts;
	ssh_hosts.push_back("violet04");
	ssh_hosts.push_back("violet05");
	ssh_hosts.push_back("violet06");
	ssh_hosts.push_back("violet07");
	ssh_hosts.push_back("violet08");
	ssh_hosts.push_back("violet09");

	Target* t = new Target("foo");
	Messaging* m = new Messaging();
	vector<Worker*> workers;

	BOOST_FOREACH(const string& s, ssh_hosts) {
		workers.push_back(new SshWorker(s, cwd, m, ssh_config));
	}

	BOOST_FOREACH(Worker* w, workers) {
		UNUSED(w);
		BOOST_CHECK(WorkerReady == m->Get().status);
	}

	BOOST_FOREACH(Worker* w, workers) {
		delete w;
	}

	delete m;
	delete t;
}
