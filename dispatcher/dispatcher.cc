#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <wait.h>
#include <vector>
#include <map>
#include <boost/tokenizer.hpp>
#include <string>
#include <boost/foreach.hpp>
#include <string>
#include <stdio.h>
#include <utility>
#include "dispatcher.h"
#include "err.h"
void error(){
  fprintf(stderr,"error\n");
  exit(1);
}

/* make sure inord is properly initialized TODO is it? */
void Dispatcher(const DependencyGraph & dependency_graph,
    std::vector<Worker *> free_workers, Messaging *messaging,
    bool keep_going){

  std::vector<Target *> ready_targets;
  //  std::map<Target *, Worker *> target_worker;
  int child_count;

  ready_targets = dependency_graph.leaf_targets_;

  child_count = 0;


  while(!ready_targets.empty() || child_count > 0){

    /* send targets to workers as long as there are targets and free workers */
    while (!ready_targets.empty() && !free_workers.empty()){
      Target *t = ready_targets.back();
      ready_targets.pop_back();
      Worker *c = free_workers.back();
      free_workers.pop_back();
      
      c->BuildTarget(t);
      ++child_count;
    }
    
    std::pair<Target *, Worker *> target_worker = messaging->GetJob();

    --child_count;
    
    free_workers.push_back(target_worker.second);
    if (target_worker.first != NULL)
      target_worker.first->MarkRealized(ready_targets);
    else if (!keep_going)
      syserr("building target error");
  }
}
