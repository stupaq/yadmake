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
#include "remoteworker.hpp"
#include "dbparser.hpp"
#include <stdio.h>

using namespace std;

void error(){
   fprintf(stderr,"error\n");
   exit(1);
}

/* bierze target, wykonuje wszystkie komendy w command */
int realize(Target * t, RemoteWorker * c){

	typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
	
   //   printf("%s\n", t->name.c_str());

	boost::char_separator<char> sep("\n");
   tokenizer tok(t->command, sep);

	BOOST_FOREACH(string s, tok)
		system(s.c_str());
	
	return 0;
}

/* mark target as realized, 
 * check whether dependent targets are ready to realize,
 * add them to targets */
void mark_realized(Target * t, vector<Target*> & targets){

	t->realized = true;

   BOOST_FOREACH(Target * i, t->dependent_targets){
      --(i->inord);
      if (i->inord == 0)
         targets.push_back(i);
   }
}

void dispatcher(vector<RemoteWorker *> & free_workers){

	vector<Target *> targets;	// only these ready to make
	map<pid_t, Target *> targ;
	map<pid_t, RemoteWorker *> workers;
	int child_count;

	// get graph
	DependencyGraph dependency_graph(0);

	// init targets
	targets = dependency_graph.leaf_targets;
   
   // count commands TODO 

	// (proces dla każdego targetu)

	child_count = 0;

   // make sure inord is properly initialized

	while(!targets.empty() || child_count > 0){

      // send targets to workers as long as there are ready targets and free workers
		while (!targets.empty() && !free_workers.empty()){
			Target *t = targets.back();
			targets.pop_back();
			RemoteWorker *c = free_workers.back();
			free_workers.pop_back();

			pid_t who;

			switch (who = fork()){
				case	-1:
					error();
					break;
				case 	 0: 
					if (realize(t, c) != 0){
						error();
					}
               exit(0);
					break;
				default:	
					++child_count;
					workers[who] = c;
					targ[who] = t;
					break;
			}
		}
      
      // wait for children
		int status;
		pid_t who;

		who = wait(&status);
		if (status != 0){
			error();
		}
		--child_count;

		free_workers.push_back(workers[who]);
		workers.erase(who);

		Target *t = targ[who];
		targ.erase(who);
		
		mark_realized(t, targets);
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

