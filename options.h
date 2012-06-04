#ifndef OPTIONS_H_
#define OPTIONS_H_

#include <vector>
#include <string>

struct options {
  std::vector<std::string> forward;
  std::string exec;
  int dist_make;
  int err;
};

options prepare_options(int argc, char** argv);



#endif //OPTIONS_H_