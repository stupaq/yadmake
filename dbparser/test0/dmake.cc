#include "../dispatcher.h"
#include "../commands.h"
#include <vector>
#include <iostream>
#include <string>

int main(){
   std::vector< RemoteWorker *> free_workers;
   RemoteWorker r1("brown01");
   RemoteWorker r2("brown02");

   free_workers.push_back(new RemoteWorker("brown01"));
   free_workers.push_back(new RemoteWorker("brown02"));
   
   for(int i = 0; i < free_workers.size(); ++i)
     free_workers[i]->connect_to();

   DependencyGraph dep_graph(dup(0));
   std::vector<std::string> basics;
   basics.push_back("make");
   count_commands(&dep_graph,basics, "blah");
   Dispatcher(dep_graph, free_workers);

   for(int i = 0; i < free_workers.size(); ++i)
     free_workers[i]->disconnect();

}
