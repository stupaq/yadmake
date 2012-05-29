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
		void SendJob(Target* target, Worker* worker);
		/**
		 * Gets first job and worker that made it from queue.
		 * @return pointerst to target and worker */
		std::pair<Target*, Worker*> GetJob();
};

/** Worker implementation using SSH
 * Always exists in two copies - in process that invoked constructor
 * and in process spawned during creation of this class instance. */
class SshWorker : public Worker {
	friend void do_kill_worker(int s);
	friend void do_kill_build(int s);
	private:
		/** this variable is set to 0 in spawned process and equal to
		 * pid of spawned process in constructor callers */
		int pid_;
		const std::string hostname_;
		const std::string& working_dir_;
		Messaging* msg_jobs_;
		Messaging* msg_parent_;
		/** this variable is _not_ properly set in dispatcher process */
		ssh::Session* session_;
		void do_run();
		/**
		 * Executes given command on remote host
		 * @param comm command to execute
		 * @return return code of executed command */
		int exec(const std::string& comm);
	public:
		/**
		 * Creates new worker, spawns process for this worker and connects
		 * to given host as defined in config_path ssh config file. This function
		 * returns immediately. Calling process should wait for worker to estabilish
		 * SSH connection by calling GetJob on provided msg_parent Messaging instance.
		 * @param hostname host to be associated with this worker
		 * @param working_dir current working directory for executing commands
		 * @param msg_parent pointer to Messaging instance used by calling process
		 * to wait for job completition
		 * @param config_path  path to SSH configuration file, dafults to ~/.ssh/config */
		SshWorker(const std::string& hostname, const std::string& working_dir,
				Messaging* msg_parent, const std::string& config_path = "");
		virtual ~SshWorker();
		/**
		 * NOT IMPLEMENTED */
		void KillBuild();
		/**
		 * Schedules build of given target on this worker, return immediately
		 * @param target pointer to target to be submitted */
		void BuildTarget(Target* target);
};

#endif  //  _WORKER_H
