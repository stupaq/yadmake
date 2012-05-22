#ifndef _WORKER_H
#define _WORKER_H

#include <sys/types.h>

#include "libsshpp.hpp"

#include "dbparser.h"

class Messaging {
	private:
		int msgqid_;
	public:
		Messaging();
		~Messaging();
		void signal();
		void wait();
		void sendJob(Target* target);
		Target* getJob();
};

class Worker {
	public:
		virtual void KillBuild() = 0;
		virtual void BuildTarget(Target* target) = 0;
};

void do_kill_worker(int s);
void do_kill_build(int s);

class SshWorker : public Worker {
	friend void do_kill_worker(int s);
	friend void do_kill_build(int s);
	private:
		int pid_;
		const std::string hostname_;
		const std::string& working_dir_;
		Messaging* msg_jobs_;
		Messaging* msg_parent_;
		/* next two variable is _not_ properly set in dispatcher process */
		ssh::Session* session_;
		void do_run();
		int exec(const std::string& comm);
	public:
		SshWorker(const std::string& hostname, const std::string& working_dir,
				Messaging* msg_parent, const std::string& config_path = "");
		virtual ~SshWorker();
		void KillBuild();
		void BuildTarget(Target* target);
};

#endif  //  _WORKER_H
