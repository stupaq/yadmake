#ifndef OPTIONS_H_
#define OPTIONS_H_

#include <vector>
#include <string>

struct options {
	std::vector<std::string> forward;     /** options which are passed on. !!! QUESTION should -k be passed on? !!! */
	std::string exec;                     /**  the command that should be run immediatly */
	int dist_make;                        /**  0 - if and only if distribute make is unnecessary */
	int keep_going;                       /**  indicates if option -k was used */
};



options PrepareOptions(int argc, char** argv);

#endif //OPTIONS_H_
