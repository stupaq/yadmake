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

// XXX
int main() {
	SshWorker* w = new SshWorker("students", "", 0);
	w->buildTarget(NULL);

	sleep(10);
	delete w;
}

/* ugly: find workaround */
static SshWorker* currentWorker = NULL;

void do_kill_worker(int s) {
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
	if (currentWorker == NULL)
		throw runtime_error("empty worker");

	/* executes worker */

	// TODO interrupt current job
}

SshWorker::SshWorker(const string& hostname, const string& config_path,
		const int msg_parent) : pid_(-1), hostname_(hostname), msg_dispatcher_(msg_parent) {
	/* executes dispatcher */

	msg_jobs_ = msgget(IPC_PRIVATE, 0777);
	if (msg_jobs_ < 0)
		throw runtime_error("msgget() failure");

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

		if (msgctl(msg_jobs_, IPC_RMID, NULL) < 0)
			throw runtime_error("msgctl() failure");

		currentWorker = NULL;
	}
}

void SshWorker::do_run() {
	static const char exit[] = "exit";

	char buffer[8092];
	struct msg_job job;
	int r;

	// TODO
	// notify that you're ready

	// TODO
	// wait for message with target address
	get_msg(msg_jobs_, &job);

	/* run remote shell */
	commch_->requestShell();
	/* skip welcome message */
	commch_->read(buffer, sizeof(buffer));

	/* write commands one by one */
	char command[] = "ls\n";
	commch_->write(command, sizeof(command));

	/* close shell */
	commch_->write(exit, sizeof(exit));

	do {
		r = commch_->readNonblocking(buffer, sizeof(buffer));
		if (r > 0)
			cout << buffer << endl;
	} while(commch_->isOpen() && ! commch_->isEof());

	// TODO
	// send message with notification
}

void SshWorker::buildTarget(Target* target) {
	/* executes dispatcher */
	if(pid_ > 0) {

		struct msg_job job;
		job.target = target;

		send_msg(msg_jobs_, &job);
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

void get_msg(int msgqid, msg_job* job) {
	if (msg_size(*job) != msgrcv(msgqid, job, msg_size(*job), 0, 0))
		throw runtime_error("msgrcv() failure");
}

void send_msg(int msgqid, msg_job* job) {
	job->type = 1;
	if (msgsnd(msgqid, job, msg_size(*job), 0) < 0)
		throw runtime_error("msgsnd() failure");
}
