#ifndef _WORKER_H
#define _WORKER_H

#include <sys/types.h>

#include "libsshpp.hpp"

#include "dbparser.h"

class Worker {
	public:
		virtual void killBuild() = 0;
		virtual void buildTarget(Target* target) = 0;
};

void do_kill_worker(int s);
void do_kill_build(int s);

class SshWorker : public Worker {
	friend void do_kill_worker(int s);
	friend void do_kill_build(int s);
	private:
		int pid_;
		const std::string hostname_;
		int msg_jobs_;
		int msg_dispatcher_;
		/* next two variables are _not_ properly set in dispatcher process */
		ssh::Session* session_;
		ssh::Channel* commch_;
		void do_run();
	public:
		SshWorker(const std::string& hostname, const std::string& config_path,
				const int msg_parent);
		virtual ~SshWorker();
		void killBuild();
		void buildTarget(Target* target);
};

struct msg_job {
	long type;
	Target* target;
};

#define msg_size(msg) (sizeof((msg))-sizeof((msg).type))

void get_msg(int msgqid, msg_job* job);

void send_msg(int msgqid, msg_job* job);

#endif  //  _WORKER_H
