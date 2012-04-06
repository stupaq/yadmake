#include "dispatcher.hpp"
#include "remoteworker.hpp"
#include <vector>

int main(){
   std::vector< RemoteWorker *> free_workers;
   free_workers.push_back(0);
   dispatcher(free_workers);
}
