#include <iostream>

#include "worker.h"
#include "dbparser.h"

using namespace std;

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE remote/execute_students
#include <boost/test/unit_test.hpp>

/* Necessary for auto Makefile to build this file
int main( */

BOOST_AUTO_TEST_CASE(execute_students) {
	Target* t = new Target("test.o");
	Messaging* m = new Messaging();
	SshWorker* w = new SshWorker("students", "~/Downloads/", m, "./tests/remote/ssh_config");

	m->GetJob();
	w->BuildTarget(t);

	pair<Target*, Worker*> p = m->GetJob();

	BOOST_CHECK(t == p.first);
	BOOST_CHECK(w == p.second);

	delete w;
	delete m;
	delete t;
}
