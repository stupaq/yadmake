#include <iostream>
#include <fstream>

#include "../options.h"
#include "../dispatcher.h"
#include "../commands.h"
#include "../remote_worker.h"
#include "../exec.h"



using namespace std;

int main(int argc, char** argv) {
  
  options r = prepare_options(argc, argv);
  if (r.err)
    abort();
  else {
    system(r.exec.c_str());
    if (r.dist_make) {
      string s = "make -pq > base";
      system(s.c_str());
      filebuf fb;
      fb.open ("base",ios::in);
      istream is(&fb);
      DependencyGraph dep_graph(is);  

      //TODO wywołać Dispatcher 

    
    }
  }
}
