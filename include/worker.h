/**
 * @defgroup remote_worker Remote execution
 * Remote execution abstracts and remote worker implementation */

#ifndef _WORKER_H
#define _WORKER_H

#include <sys/types.h>

#include "libsshpp.hpp"

#include "dbparser.h"
#include "err.h"

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
	virtual ~Worker() {};
};

/** Enum representing Report status */
enum Status {WorkerReady, SshError, NewJob, TargetDone, TargetFailed, WorkerDied};

/** Report that can be send through Messaging */
struct Report {
	long type;
	enum Status status;
	Worker* worker;
	Target* target;
};

/** Messaging primitive between dispatcher and worker.
 * Note that no worker process should invoke destructor of Messaging object. */
class Messaging {
private:
	int msgqid_;
public:
	Messaging();
	Messaging(const Messaging& that) = delete;
	/**
	 * Messaging destructor removes underlying message queue.
	 * Should never be invoked by worker process. */
	~Messaging();

	/**
	 * Creates and submits Report to given messaging queue.
	 * @param status Status of Report
	 * @param worker worker that issued a Report
	 * @param target target that was processed when issuing a Report */
	void Send(enum Status status, Worker* worker = NULL, Target* target = NULL);

	/**
	 * Gets first Report from given messaging queue.
	 * @return first Report from queue */
	Report Get();
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
	const std::string working_dir_;
	Messaging* msg_jobs_;
	Messaging* msg_parent_;
	/** this variable is _not_ properly set in dispatcher process */
	ssh::Session* session_;
	/** Worker's main loop */
	void do_run();
	/**
	 * Executes given command on remote host
	 * @param comm command to execute
	 * @return return code of executed command */
	int exec(const std::string& comm);
	/**
	 * Reads output of command executed on this channel
	 * @param comch channel to read
	 * @param is_stderr determines whether read stdout or stderr */
	void pipeOutput(ssh::Channel& commch, bool is_stderr = false);
public:
	/**
	 * Creates new worker, spawns process for this worker and connects
	 * to given host as defined in config_path ssh config file. This function
	 * returns immediately. Calling process should wait for worker to estabilish
	 * SSH connection by calling Get on provided msg_parent Messaging instance.
	 * @param hostname host to be associated with this worker
	 * @param working_dir current working directory for executing commands
	 * @param msg_parent pointer to Messaging instance used by calling process
	 * to wait for job completition
	 * @param config_path  path to SSH configuration file, dafults to ~/.ssh/config */
	SshWorker(const std::string& hostname, const std::string& working_dir,
	          Messaging* msg_parent, const std::string& config_path = "");

	/**
	 * Kills given worker irreversibly.
	 * */
	~SshWorker();

	/**
	 * NOT IMPLEMENTED */
	void KillBuild();

	/**
	 * Schedules build of given target on this worker, return immediately
	 * @param target pointer to target to be submitted */
	void BuildTarget(Target* target);
};

/** Returns vector of workers, based on file ".yadmake/hosts"
 * in home directiory. The file should consist of names of hosts
 * and working directories */
std::vector<Worker *> get_workers(Messaging* msg_parent);

#endif  //  _WORKER_H
