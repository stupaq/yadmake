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
#include "dbparser.h"
#include "err.h"

void error(){
  fprintf(stderr,"error\n");
  exit(1);
}

/* temporary,
 * will be replaced,
 * executes all Targets commands */
inline int Realize(Target * t, RemoteWorker * c){

  BOOST_FOREACH(std::string s, t->commands_)
    system(s.c_str());

  return 0;
}

/** mark target as realized, 
 *  check whether dependent targets are ready to realize,
 *  add them to ready_targets */
inline void MarkRealized(Target * t, std::vector<Target*> & ready_targets){
  
  if (t == NULL)
    syserr("MarkRealized: target is NULL");
  BOOST_FOREACH(Target * i, t->dependent_targets_){
    --(i->inord_);
    if (i->inord_==0)
      ready_targets.push_back(i);
  }
}

void Dispatcher(const DependencyGraph & dependency_graph, std::vector<RemoteWorker *> free_workers){

  std::vector<Target *> ready_targets;	// only these ready to make
  std::map<pid_t, Target *> pid_target;
  std::map<pid_t, RemoteWorker *> pid_worker;
  int child_count;

  // init ready_targets
  ready_targets = dependency_graph.leaf_targets_;

  // (proces dla każdego targetu)

  child_count = 0;

  // make sure inord is properly initialized
  dependency_graph.ReinitInord();

  while(!ready_targets.empty() || child_count > 0){

    // send targets to workers as long as there are targets and free workers
    while (!ready_targets.empty() && !free_workers.empty()){
      Target *t = ready_targets.back();
      ready_targets.pop_back();
      RemoteWorker *c = free_workers.back();
      free_workers.pop_back();

      pid_t who;

      switch (who = fork()){
        case	-1:
          syserr("fork erorr");
          error();
          break;
        case 	 0: 
          Realize(t, c);
          return ;
          break;
        default:	
          ++child_count;
          pid_worker[who] = c;
          pid_target[who] = t;
          break;
      }
    }

    // wait for children
    int status;
    pid_t who = wait(&status);

    if (status!=0){
      syserr("wait error");
    }
    --child_count;

    free_workers.push_back(pid_worker[who]);
    pid_worker.erase(who);

    Target *t = pid_target[who];
    pid_target.erase(who);

    MarkRealized(t, ready_targets);
  }
}

/* tests */

/* realize */
/*
   void realize_test(){
   Target t("t1");
   RemoteWorker c;
   t.set_command("echo ala\necho alala\necho ololo");
   realize(&t, &c);

   }
   */

