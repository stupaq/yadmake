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
#include "../include/dispatcher.h"
#include "../include/dbparser.h"
#include "../include/err.h"
#include "../include/worker.h"

void error(){
  fprintf(stderr,"error\n");
  exit(1);
}

/* mark target as realized, 
 * check whether dependent targets are ready to realize,
 * add them to ready_targets */
inline void MarkRealized(Target * t, std::vector<Target*> & ready_targets){
  
  BOOST_FOREACH(Target * i, t->dependent_targets_){
    --(i->inord_);
    if (i->inord_==0)
      ready_targets.push_back(i);
  }
}

void Dispatcher(const DependencyGraph & dependency_graph, std::vector<Worker *> free_workers, Messaging *messaging){

  std::vector<Target *> ready_targets;	// only these ready to make
  std::map<Target *, Worker *> target_worker;
  int child_count;

  // init ready_targets
  ready_targets = dependency_graph.leaf_targets_;

  child_count = 0;

  // make sure inord is properly initialized TODO is it?
  // any waits on messaging needed? TODO

  while(!ready_targets.empty() || child_count > 0){

    // send targets to workers as long as there are targets and free workers
    while (!ready_targets.empty() && !free_workers.empty()){
      Target *t = ready_targets.back();
      ready_targets.pop_back();
      Worker *c = free_workers.back();
      free_workers.pop_back();
      
      target_worker[t] = c;
      c->BuildTarget(t);
      ++child_count;
    }
    
    Target *completed_target;

    completed_target = messaging->getJob();
    --child_count;
    
    free_workers.push_back(target_worker[completed_target]);
    target_worker.erase(completed_target);
    if (completed_target != NULL)
      MarkRealized(completed_target, ready_targets);
  }
  // recover inord TODO
}
