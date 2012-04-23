#include <vector>
#include <boost/foreach.hpp>
#include "dispatcher.h"
#include "remote_worker.h"

int main(){
  char* user = "md292600";
  char* password = getpass("");
  std::vector< RemoteWorker *> free_workers;
  free_workers.push_back(new RemoteWorker("yellow01", password, user));
  free_workers.push_back(new RemoteWorker("yellow02", password, user));
  BOOST_FOREACH(RemoteWorker* rw, free_workers)
    rw->connect_to();
  DependencyGraph dep_graph(0);
  Dispatcher(dep_graph, free_workers);
  BOOST_FOREACH(RemoteWorker* rw, free_workers)
    rw->close_connection();
}