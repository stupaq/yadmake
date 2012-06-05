#include <iostream>
#include <fstream>

#include "options.h"
#include "dispatcher.h"
#include "dbparser.h"
#include "err.h"
#include "exec.h"



using namespace std;

int main(int argc, char** argv) {
  
  options r = prepare_options(argc, argv);
  if (r.err)
    abort();
  else {
    system(r.exec.c_str());
    if (r.dist_make) {
      const string make_command = "make";
      vector<string> args;
      args.push_back("-pq");
      if (argc > 1) {
      args.push_back("-C");
      args.push_back(argv[1]);
      }
      pair<string, string> out = exec(make_command, args);

      /* create graph */
      DependencyGraph graph(out.first);

      /* get commands */
      const string delimiter = "3344543508980989031231";
      graph.CountCommands(r.forward, delimiter);
      graph.DumpMakefile(cout);
      Dispatcher(graph, r.keep_going);

    }
  }
}
