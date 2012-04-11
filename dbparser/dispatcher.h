#ifndef _DISPATCHER_
#define _DISPATCHER_

#include <vector>
#include "dbparser.h"

void Dispatcher(const DependencyGraph & dependency_graph, std::vector<RemoteWorker * > free_workers);

#endif
