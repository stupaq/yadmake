#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <wait.h>
#include <vector>
#include <map>

using namespace std;

class Target{};
class Computer{};

void error(){}

int exec(Target * t, Computer * c){
	char **argv = 0;
	// build argv	TODO

	return execvp("./single_target", argv);
}

bool done;
vector<Target *> targets;			// ready to do 
map<pid_t, Target *> targ;
vector<Computer *> free_comp;
map<pid_t, Computer *> comp;

int main(){
	// read data about available computers

	// connect with them (check connection or sth)

	// get graph

	// init free_comp
	
	if (free_comp.empty())
		error();

	// init targets

	// (proces dla każdego targetu/joba)
	
	done = false;

	while(not done){
		while (!targets.empty() && !free_comp.empty()){
			Target *t = targets.back();
			targets.pop_back();
			Computer *c = free_comp.back();
			free_comp.pop_back();

			pid_t who;

			switch (who = fork()){
				case	-1:
					error();
					break;
				case 	 0: 
					if (exec(t, c) != 0){
						error();
					}
					break;
				default:	
					comp[who] = c;
					targ[who] = t;
					break;
			}
		}
		int status;
		pid_t who;
		who = wait(&status);
		if (status != 0){
			error();
		}

		free_comp.push_back(comp[who]);
		comp.erase(who);

		// na grafie cos cfanego 
		// zaznacz ze target wykonany
		// 'usun' zaleznosci od wykonanego
		// dodaj cos do targets

		// czy done?
	}
}
