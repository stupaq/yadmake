#ifndef _DISPATCHER_
#define _DISPATCHER_

#include <vector>

#include "dbparser.h"
#include "worker.h"

/**
 * Manages compilation by sending targets to workers,
 * @param dependency_graph - constructed dep. graph with, all targets
 * with commands set.
 * @param free_workres - vector of workers that will be used to bulid targets
 * @param keep_going - is -k option set - if yes, continue after error
 * requires:
 * dependency_graph all targets with commands
 * workers ready to build
 * (calls ReinitInord)
 * state after:
 * inord NOT correct (TODO ?)
 * workers are free
 * [hopefully targets are built]
 */
void Dispatcher(const DependencyGraph & dependency_graph,
    std::vector<Worker *> created_not_ready_workers,
    Messaging * m,
    bool keep_going=false);

#endif
