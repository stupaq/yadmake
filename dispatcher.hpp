#ifndef _DISPATCHER_
#define _DISPATCHER_

#include <vector>
#include "remoteworker.hpp"

void dispatcher(std::vector<RemoteWorker * > & free_workers);

void realize_test();

#endif
