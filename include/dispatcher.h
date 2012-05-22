#ifndef _DISPATCHER_
#define _DISPATCHER_

#include <vector>
#include "dbparser.h"
#include "worker.h"

void Dispatcher(const DependencyGraph & dependency_graph, std::vector<Worker *> free_workers, Messaging *messaging);

#endif
