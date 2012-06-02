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

	BOOST_CHECK(WorkerReady == m->Get().status);

	w->BuildTarget(t);

	Report r = m->Get();

	BOOST_CHECK(TargetDone == r.status);
	BOOST_CHECK(t == r.target);
	BOOST_CHECK(w == r.worker);

	delete w;
	delete m;
	delete t;
}
