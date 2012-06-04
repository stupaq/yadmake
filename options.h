#ifndef OPTIONS_H_
#define OPTIONS_H_

#include <vector>
#include <string>

struct options {
  std::vector<std::string> forward;     /** options which are passed on. !!! QUESTION should -k be passed on? !!! */
  std::string exec;                     /**  the command that should be run immediatly */
  int dist_make;                        /**  0 - if and only if distribute make is unnecessary */
  int err;                              /**  indicates if error occured during options parsing */
  int keep_going;                       /**  indicates if option -k was used */
};



options prepare_options(int argc, char** argv);



#endif //OPTIONS_H_