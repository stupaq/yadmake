#include "../dispatcher.h"
#include "../commands.h"
#include <vector>
#include <iostream>

int main(){
   std::vector< RemoteWorker *> free_workers;
   free_workers.push_back(0);
   DependencyGraph dep_graph(dup(0));
   std::vector<std::string> basics;
   basics.push_back("make");
   count_commands(&dep_graph,basics, "blah");
   Dispatcher(dep_graph, free_workers);
}
