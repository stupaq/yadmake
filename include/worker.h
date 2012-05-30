#ifndef _WORKER_H
#define _WORKER_H

#include <sys/types.h>

#include "libsshpp.hpp"

#include "dbparser.h"

/** Abstract class representing worker. */
class Worker {
	public:
		/**
		 * Kills current build, freeing worker. */
		virtual void KillBuild() = 0;
		/**
		 * Submits given target to this worker.
		 * @param target pointer to target to build */
		virtual void BuildTarget(Target* target) = 0;
};

/** Messaging primitive between dispatcher and worker. */
class Messaging {
	private:
		int msgqid_;
	public:
		Messaging();
		~Messaging();
		/**
		 * Submits job to given messaging queue.
		 * @param target pointer to target to build */
		void SendJob(Target* target, Worker* worker = NULL);
		/**
		 * Gets first job and worker that made it from queue.
		 * @return pointerst to target and worker */
		std::pair<Target*, Worker*> GetJob();
};

/** Worker implementation using SSH */
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

/** Returns vector of workers, based on file ".yadmake/hosts"
 * in home directiory. The file should consist of names of hosts
 * and working directories */
std::vector<Worker *> get_workers(Messaging* msg_parent);

#endif  //  _WORKER_H
