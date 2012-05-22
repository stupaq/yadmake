#include <string>
#include <stdexcept>
#include <iostream>

#include <sys/ipc.h>
#include <sys/msg.h>
#include <signal.h>

#include "libsshpp.hpp"

#include "worker.h"

using namespace std;
using namespace ssh;

#define UNUSED(x) ((void) (x))

// XXX
int main() {
	Target* tg = new Target("abcd");
	Messaging* m = new Messaging();
	SshWorker* w = new SshWorker("students", "~/", "", m);

	m->wait();
	w->buildTarget(tg);

	Target* t = m->getJob();
	cerr << tg << " " << t << endl;

	delete w;
	delete m;
	delete tg;
}

/* ugly: find workaround */
static SshWorker* currentWorker = NULL;

void do_kill_worker(int s) {
	UNUSED(s);

	if (currentWorker == NULL)
		throw runtime_error("empty worker");

	/* executes worker */

	do_kill_build(0);

	/* close connection */
	currentWorker->commch_->sendEof();
	currentWorker->commch_->close();
	delete currentWorker->commch_;

	currentWorker->session_->disconnect();
	delete currentWorker->session_;

	exit(0);
}

void do_kill_build(int s) {
	UNUSED(s);

	if (currentWorker == NULL)
		throw runtime_error("empty worker");

	/* executes worker */
	throw runtime_error("NOT IMPLEMENTED");
}

SshWorker::SshWorker(const string& hostname, const string& working_dir, const string& config_path,
		Messaging* msg_parent) : pid_(-1), hostname_(hostname), working_dir_(working_dir),
		msg_parent_(msg_parent) {
	/* executes dispatcher */

	msg_jobs_ = new Messaging();

	pid_ = fork();
	if (pid_ < 0)
		throw runtime_error("fork() failure");
	else if (pid_ > 0)
		return;

	/* executes worker */
	currentWorker = this;

	/* setup */
	signal(SIGUSR1, &do_kill_build);
	signal(SIGTERM, &do_kill_worker);

	/* connect and authenticate */
	session_ = new Session;
	session_->setOption(SSH_OPTIONS_HOST, hostname_.c_str());
	session_->optionsParseConfig(config_path.empty() ? NULL : config_path.c_str());

	session_->connect();
	session_->userauthAutopubkey();

	commch_ = new Channel(*session_);
	commch_->openSession();

	/* run main dispatcher */
	do_run();

	/* this will dispose all data */
	exit(0);
}

SshWorker::~SshWorker() {
	/* executes dispatcher */
	if (pid_ > 0) {

		/* kill worker */
		if (kill(pid_, SIGTERM) < 0)
			throw runtime_error("can't kill worker process");

		delete msg_jobs_;

		currentWorker = NULL;
	}
}

void SshWorker::do_run() {
	static const char exit[] = "exit";

	char buffer[8092];
	int r;

	/* ready to start */
	msg_parent_->signal();

	Target* target = msg_jobs_->getJob();

	/* run remote shell and skip message */
	commch_->requestShell();
	commch_->read(buffer, sizeof(buffer));

	/* change working dir */
	// TODO

	// TODO
	/* write commands one by one */
	char command[] = "ls\n";
	commch_->write(command, sizeof(command));

	/* close shell */
	commch_->write(exit, sizeof(exit));

	cerr << "exited" << endl;

	do {
		r = commch_->readNonblocking(buffer, sizeof(buffer));
		if (r > 0)
			cout << buffer << endl;
	} while(commch_->isOpen() && ! commch_->isEof());

	msg_parent_->sendJob(target);
}

void SshWorker::buildTarget(Target* target) {
	/* executes dispatcher */
	if(pid_ > 0) {

		msg_jobs_->sendJob(target);
	}
}

void SshWorker::killBuild() {
	/* executes dispatcher */
	if(pid_ > 0) {

		if (pid_ < 0)
			throw runtime_error("worker not running");

		if (kill(pid_, SIGUSR1) < 0)
			throw runtime_error("failed to kill worker");

		pid_ = -1;
	}
}

Messaging::Messaging() {
	msgqid_ = msgget(IPC_PRIVATE, 0777);
	if (msgqid_ < 0)
		throw runtime_error("msgget() failure");
}

Messaging::~Messaging() {
	msgctl(msgqid_, IPC_RMID, NULL);
}

struct msg_job {
	long type;
	Target* target;
};

#define msg_size(msg) (sizeof((msg))-sizeof((msg).type))

void Messaging::sendJob(Target* target) {
	struct msg_job job;

	job.type = 1;
	job.target = target;

	if (target)
		cerr << "sending job " << job.target << "to " << msgqid_ << endl;
	else
		cerr << "signaling " << msgqid_ << endl;

	if (msgsnd(msgqid_, &job, msg_size(job), 0) < 0)
		throw runtime_error("msgsnd() failure");
}

Target* Messaging::getJob() {
	struct msg_job job;

	cerr << "waiting for job " << msgqid_ << endl;

	if (msg_size(job) != msgrcv(msgqid_, &job, msg_size(job), 0, 0))
		throw runtime_error("msgrcv() failure");

	if (job.target)
		cerr << "got job " << job.target << endl;
	else
		cerr << "signaled" << endl;

	return job.target;
}

void Messaging::signal() {
	sendJob(NULL);
}

void Messaging::wait() {
	getJob();
}
