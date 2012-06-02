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

std::vector<Worker *> get_ready_workers(const Messaging &m,
    std::vector<Worker *> & broken_workers)
{
  int n;
  std::vector<Worker *> result;
  Report r;

  n = get_workers().size();
  
  for(int i = 0; i < n; ++i)
  {
    r = m->Get();
    switch (r.status)
    {
      case WorkerReady:
        result.push_back(r->worker);
        break;
      case SshError:
        broken_workers.push_back(r->worker)
        fprintf(stderr, "SshError during initialization\n");
        break;
      default:
        fprintf(stderr, "sth unusual happend\n");
    }
  }
  return result;
}

void Dispatcher(const DependencyGraph & dependency_graph,
		bool keep_going){

	std::vector<Worker *> free_workers;
  std::vector<Worker *> broken_workers;
	std::vector<Target *> ready_targets;
  Messaging m = new Messaging();
	int child_count;
  Report report;

  dependency_graph.ReinitInord();

	ready_targets = dependency_graph.leaf_targets_;

  free_workers = get_ready_workers(broken_workers);

	child_count = 0;

	while(!ready_targets.empty() || child_count > 0){
		std::cout << "while outside in" << std::endl;

		/* send targets to workers as long as there are targets and free workers */
		while (!ready_targets.empty() && !free_workers.empty()){
			std::cout << "while inside in" << std::endl;
			Target *t = ready_targets.back();
			ready_targets.pop_back();
			Worker *c = free_workers.back();
			free_workers.pop_back();

			c->BuildTarget(t);
			++child_count;
			std::cout << "while inside out" << std::endl;
		}

    report = m->Get();
		std::cout << "got job" << std::endl;
		--child_count;
    switch (report.status)
    {
      case TargetDone:
        report.target->MarkRealized(ready_targets);
        free_workers.push_back(report.worker);
        break;
      case SshError:
        /* delete worker, repeat target */
        fprintf(stderr, "SshError during build\n");
        ready_targets.push_back(report.target);
        broken_workers.push_back(report.target);
        break;
      case NewJob:
        /* wtf? TODO */
        break;
      case TargetFailed:
        free_workers.push_back(report.worker);
        if (!keep_going)
          syserr("building target error");
        break;
      case WorkerDied:
        fprintf(stderr, "Worker Died much too early\n");
        ready_targets.push_back(report.target);
        broken_workers.push_back(report.target);
        break;
      default:
        fprintf(stderr, "impossible is nothing\n");
    }
    if (free_workers.size() == 0)
      syserr("all workers failed");
		std::cout << "while outside out" << std::endl;
	}

  BOOST_FOREACH(worker *w, free_workers)
    delete w;
  BOOST_FOREACH(worker *w, broken_workers)
    delete w;
  
  /* czy musze odbierac status zniszczenia? TODO */
  int n = free_workers.size() + broken_workers.size();
  for(int i = 0; i < n; ++i)
  {
    report = m->Get();
    if (report.status != WorkerDied)
      syserr("status after worker delete is not WorkerDied");
  }
  delete m;
}
