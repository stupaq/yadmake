#include "dispatcher.h"
#include <vector>

int main(){
   std::vector< RemoteWorker *> free_workers;
   free_workers.push_back(0);
   DependencyGraph dep_graph(0);
   Dispatcher(dep_graph, free_workers);
}
