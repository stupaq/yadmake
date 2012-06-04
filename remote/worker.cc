#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/wait.h>
#include <signal.h>

#include <string>
#include <stdexcept>
#include <iostream>
#include <utility>
#include <vector>
#include <fstream>

#include "libsshpp.hpp"

#include "worker.h"

using namespace std;
using namespace ssh;

#define UNUSED(x) ((void) (x))

#define EMPTY_READS_NB 50
#define EMPTY_READS_US 5000

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

		currentWorker->msg_parent_->Send(WorkerDied, currentWorker);

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

		try {
			/* connect and authenticate */
			session_ = new Session;
			session_->setOption(SSH_OPTIONS_HOST, hostname_.c_str());
			session_->optionsParseConfig(config_path.empty() ? NULL : config_path.c_str());

			session_->connect();
			session_->userauthAutopubkey();
		} catch (SshException e) {
			msg_parent_->Send(SshError);
			throw e;
		}

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

		/* wait unitl child process died */
		waitpid(pid_, NULL, 0);

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

#ifdef WORKERS_DEBUG
	cerr << "EXECUTING:\n\t" << comm << endl;
#endif // WORKERS_DEBUG
	commch.requestExec(comm.c_str());

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
#ifdef WORKERS_DEBUG
	cerr << "RETURN VALUE: " << ret_val << endl << endl;
#endif // WORKERS_DEBUG

	/* close chanell */
	commch.close();

	return ret_val;
}

void SshWorker::do_run() {
	int ret_val;

	/* ready to start */
	msg_parent_->Send(WorkerReady, this);

	for(;;) {
		Report r = msg_jobs_->Get();

		try {
			/* prepare, send commands and read response */
			string commands = r.target->BuildBashScript(working_dir_);
			ret_val = exec(commands);
		} catch(SshException e) {
			msg_parent_->Send(SshError, this, r.target);
			throw e;
		}

		/* notify that we've done here */
		if (ret_val == 0)
			msg_parent_->Send(TargetDone, this, r.target);
		else
			msg_parent_->Send(TargetFailed, this, r.target);
	}
}

void SshWorker::BuildTarget(Target* target) {
	/* EXEC: DISPATCHER */
	if(pid_ > 0) {

		msg_jobs_->Send(NewJob, this, target);
	}
}

void SshWorker::KillBuild() {
	/* EXEC: DISPATCHER */
	if(pid_ > 0) {

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
	/* any worker should never destroy message queue,
	 * this must be done by dispatching process */
	if (currentWorker == NULL) {
		msgctl(msgqid_, IPC_RMID, NULL);
	} else {
		throw runtime_error("Messaging destructor invoked in worker");
	}
}

#define msg_size(msg) (sizeof((msg))-sizeof((msg).type))

void Messaging::Send(enum Status status, Worker* worker, Target* target) {
	Report r = {1, status, worker, target};

	if (msgsnd(msgqid_, &r, msg_size(r), 0) < 0)
		throw runtime_error("msgsnd() failure");
}

Report Messaging::Get() {
	Report r;

	if (msg_size(r) != msgrcv(msgqid_, &r, msg_size(r), 0, 0))
		throw runtime_error("msgrcv() failure");

	return r;
}

std::vector<Worker *> get_workers(Messaging* msg_parent) {

	std::vector<Worker *> workers;

	std::string home_path = getenv("HOME");
	std::string hosts_path = home_path + "/.yadmake/hosts";
	std::string config_path = home_path + "/.ssh/config";

	ifstream conf;
	conf.open(hosts_path);

	if (!conf.is_open())
		throw runtime_error("Could not open file with available hosts. Please, chceck configuration.");

	while (conf.good()) {

		std::string host;
		std::string cwd;

		conf >> host;
		if (!conf.good()) break;
		conf >> cwd;

		SshWorker * w = new SshWorker(host, cwd, msg_parent, config_path);
		workers.push_back(w);
	}

	conf.close();

	return workers;
}
