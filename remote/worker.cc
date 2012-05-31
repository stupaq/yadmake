#include <string>
#include <stdexcept>
#include <iostream>
#include <utility>
#include <vector>
#include <fstream>

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

		try {
			/* connect and authenticate */
			session_ = new Session;
			session_->setOption(SSH_OPTIONS_HOST, hostname_.c_str());
			session_->optionsParseConfig(config_path.empty() ? NULL : config_path.c_str());

			session_->connect();
			session_->userauthAutopubkey();
		} catch (SshException e) {
			msg_parent_->SendJob(NULL, NULL);
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
	int ret_val;

	/* ready to start */
	msg_parent_->SendJob(NULL, this);

	pair<Target*, Worker*> p = msg_jobs_->GetJob();
	Target* target = p.first;

	try {
		/* prepare, send commands and read response */
		string commands = target->BuildBashScript(working_dir_);
		ret_val = exec(commands);
	} catch(SshException e) {
		msg_parent_->SendJob(NULL, this);
		throw e;
	}

	/* notify that we've done here */
	if (ret_val == 0)
		msg_parent_->SendJob(target, this);
	else
		msg_parent_->SendJob(NULL, this);
}

void SshWorker::BuildTarget(Target* target) {
	/* EXEC: DISPATCHER */
	if(pid_ > 0) {

		msg_jobs_->SendJob(target, NULL);
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
	// FIXME reference counting
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
