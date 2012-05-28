#include <string>
#include <stdexcept>
#include <iostream>
#include <utility>

#include <sys/ipc.h>
#include <sys/msg.h>
#include <signal.h>

#include "libsshpp.hpp"

#include "worker.h"

using namespace std;
using namespace ssh;

#define UNUSED(x) ((void) (x))

#define EMPTY_READS_NB 50
#define EMPTY_READS_US 5000

// XXX
/* TODO zmieniona nazwa ponizej.. 
int mpain() {
	Target* tg = new Target("abcd");
	Messaging* m = new Messaging();
	SshWorker* w = new SshWorker("students", "~/Downloads/", m);

	m->GetJob();
	w->BuildTarget(tg);

	pair<Target*, Worker*> p = m->GetJob();
	cerr << tg << " " << p.first << endl << w << " " << p.second << endl;

	delete w;
	delete m;
	delete tg;
}
*/

/* ugly: find workaround */
static SshWorker* currentWorker = NULL;

void do_kill_build(int sig) {
	if (sig) {
		if (currentWorker == NULL)
			throw runtime_error("empty worker");

		/* EXEC: WORKER */
		throw runtime_error("NOT IMPLEMENTED");
	}
}

void do_kill_worker(int sig) {
	if (sig) {
		if (currentWorker == NULL)
			throw runtime_error("empty worker");

		/* EXEC: WORKER */

		do_kill_build(0);

		/* close connection */
		currentWorker->session_->disconnect();
		delete currentWorker->session_;

		exit(0);
	}
}

SshWorker::SshWorker(const string& hostname, const string& working_dir,
		Messaging* msg_parent, const string& config_path) : pid_(-1),
	hostname_(hostname), working_dir_(working_dir), msg_parent_(msg_parent) {
		/* EXEC: DISPATCHER */

		msg_jobs_ = new Messaging();

		pid_ = fork();
		if (pid_ < 0)
			throw runtime_error("fork() failure");
		else if (pid_ > 0)
			return;

		/* EXEC: WORKER */
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

		/* run main dispatcher */
		do_run();

		/* this will dispose all data */
		exit(0);
	}

SshWorker::~SshWorker() {
	/* EXEC: DISPATCHER */
	if (pid_ > 0) {

		/* kill worker */
		if (kill(pid_, SIGTERM) < 0)
			throw runtime_error("can't kill worker process");

		delete msg_jobs_;

		currentWorker = NULL;
	}
}

int SshWorker::exec(const string& comm) {
	char buffer[8092];
	int buffersz = sizeof(buffer) - 1;
	int r, ret_val = 0;

	/* unfortunately me must request new chanell each time */
	Channel commch(*session_);
	commch.openSession();

	commch.requestExec(comm.c_str());
	cerr << "EXEC: " << comm << endl;

	commch.sendEof();
	/* read output */
	int empty_reads = EMPTY_READS_NB;
	do {
		r = commch.readNonblocking(buffer, buffersz);
		buffer[r] = '\0';
		if (r > 0) {
			cout << buffer << endl;
		} else {
			--empty_reads;
			usleep(EMPTY_READS_US);
		}
	} while(commch.isOpen() && ! commch.isEof() && empty_reads);

	/* get return value */
	ret_val = commch.getExitStatus();
	cerr << "RETVAL: " << ret_val << endl << endl;

	/* close chanell */
	commch.close();

	return ret_val;
}

void SshWorker::do_run() {

	/* ready to start */
	msg_parent_->SendJob(NULL);

	pair<Target*, Worker*> p = msg_jobs_->GetJob();
	Target* target = p.first;

	/* prepare, send commands and read response */
	string commands = target->BuildBashScript(working_dir_);
	int ret_val = exec(commands);

	/* notify that we've done here */
	if (ret_val == 0)
		msg_parent_->SendJob(target, this);
	else
		msg_parent_->SendJob(NULL, this);
}

void SshWorker::BuildTarget(Target* target) {
	/* EXEC: DISPATCHER */
	if(pid_ > 0) {

		msg_jobs_->SendJob(target);
	}
}

void SshWorker::KillBuild() {
	/* EXEC: DISPATCHER */
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
	Worker* worker;
};

#define msg_size(msg) (sizeof((msg))-sizeof((msg).type))

void Messaging::SendJob(Target* target, Worker* worker) {
	struct msg_job job = {1, target, worker};

	if (msgsnd(msgqid_, &job, msg_size(job), 0) < 0)
		throw runtime_error("msgsnd() failure");
}

pair<Target*, Worker*> Messaging::GetJob() {
	struct msg_job job;

	if (msg_size(job) != msgrcv(msgqid_, &job, msg_size(job), 0, 0))
		throw runtime_error("msgrcv() failure");

	return make_pair(job.target, job.worker);
}
