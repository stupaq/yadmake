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
	SshWorker* w1 = new SshWorker("students", "~/Downloads/", m, "./tests/remote/ssh_config");
	SshWorker* w2 = new SshWorker("students", "~/Downloads/", m, "./tests/remote/ssh_config");

	BOOST_CHECK(WorkerReady == m->Get().status);
	BOOST_CHECK(WorkerReady == m->Get().status);

	w1->BuildTarget(t);
	w2->BuildTarget(t);
	BOOST_CHECK(TargetDone == m->Get().status);
	BOOST_CHECK(TargetDone == m->Get().status);

	w1->BuildTarget(t);
	w2->BuildTarget(t);
	BOOST_CHECK(TargetDone == m->Get().status);
	BOOST_CHECK(TargetDone == m->Get().status);

	delete w1;
	delete w2;

	BOOST_CHECK(WorkerDied == m->Get().status);
	BOOST_CHECK(WorkerDied == m->Get().status);

	delete m;
	delete t;
}
