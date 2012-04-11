#include "../dispatcher.h"
#include "../commands.h"
#include <vector>

int main(){
   std::vector< RemoteWorker *> free_workers;
   free_workers.push_back(0);
   DependencyGraph dep_graph(0);
   //count_commands(&dep_graph);
   Dispatcher(dep_graph, free_workers);
}
