#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <wait.h>

#include <vector>
#include <map>
#include <string>
#include <string>
#include <utility>

#include <boost/tokenizer.hpp>
#include <boost/foreach.hpp>

#include "dispatcher.h"
#include "err.h"

std::vector<Worker *> get_ready_workers(
    Messaging *m,
    const std::vector<Worker *> & created_not_ready_workers)
{
	int n;
	Report r;
  std::vector<Worker *> result;

	n = created_not_ready_workers.size();

	for(int i = 0; i < n; ++i)
	{
		r = m->Get();
		switch (r.status)
		{
			case WorkerReady:
				result.push_back(r.worker);
				break;
			case SshError:
				fprintf(stderr, "SshError during initialization\n");
				break;
			default:
				fprintf(stderr, "sth unusual happend\n");
		}
	}
  return result;
}


void Dispatcher(const DependencyGraph & dependency_graph,
    std::vector<Worker *> created_not_ready_workers,
    Messaging * m,
		bool keep_going)
{
	std::vector<Worker *> free_workers;
  std::vector<Target *> ready_targets;
	// Messaging *m = new Messaging();
	int child_count;
	Report report;

	dependency_graph.ReinitInord();

	ready_targets = dependency_graph.leaf_targets_;

	free_workers = get_ready_workers(m, created_not_ready_workers);

	child_count = 0;

	while(!ready_targets.empty() || child_count > 0){

		/* send targets to workers as long as there are targets and free workers */
		while (!ready_targets.empty() && !free_workers.empty()){
			Target *t = ready_targets.back();
			ready_targets.pop_back();
			if (t->EmptyRules())
			{
				t->MarkRealized(ready_targets);
			}
			else
			{
				Worker *c = free_workers.back();
				free_workers.pop_back();

				c->BuildTarget(t);
				++child_count;
			}
		}

		if (child_count) {
			report = m->Get();
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
					break;
				default:
					fprintf(stderr, "impossible is nothing\n");
			}
			if (free_workers.size() == 0)
				syserr("all workers failed");
		}
	}
}
