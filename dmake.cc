#include <iostream>
#include <fstream>

#include "../options.h"
#include "../dispatcher.h"
#include "../dbparser.h"
#include "../err.h"



using namespace std;

int main(int argc, char** argv) {
  
  options r = prepare_options(argc, argv);
  if (r.err)
    abort();
  else {
    system(r.exec.c_str());
    if (r.dist_make) {
      system("make -pq > base");
      filebuf fb;
      fb.open ("base",ios::in);
      istream is(&fb);
      DependencyGraph dep_graph(is);  
      
      Dispatcher(dep_graph, r.keep_going);
      fb.close();
      system("rm base");
    }
  }
}
