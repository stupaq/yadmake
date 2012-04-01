#include "exec.h"
#include "dbparser.hpp"
#include <vector>
#include <list>
#include <string>
#include <stdexcept>


using std::vector;
using std::list;
using std::string;

/* Shares targets into levels. Leafs have level 0 and so on. */
vector<vector<Target*> > get_levels(DependencyGraph* graph) {
   vector<vector<Target*> > levels;
   vector<Target*> level = graph->leaf_targets;

   while (!level.empty()) {
      levels.push_back(level);
      vector<Target*> next_level;
      
      for (vector<Target*>::iterator it = level.begin();
            it != level.end(); ++it)
         for(list<Target*>::iterator parent = (*it)->dependent_targets.begin();
               parent != (*it)->dependent_targets.end(); ++parent) {
            --(*parent)->inord;
            if ((*parent)->inord == 0)
               next_level.push_back(*parent);
         }

      level = next_level;
   }

   return levels;
}

/* Adds commands to targets at one level (to_make).
   Targets at previous level should be in not_to_make.
   Delimiter is the additional target printing its name.
   Basics contains make, -n and options from command line.
   */
void count_one_level(const vector<string>& basics, const string& delimiter,
      const vector<Target*>& to_make, const vector<Target*>& not_to_make) {
   vector<string> options = basics;
   
   for (vector<Target*>::const_iterator it = to_make.begin(); it != to_make.end(); ++it) {
      options.push_back((*it)->name);
      options.push_back(delimiter);
   }

   for (vector<Target*>::const_iterator it = not_to_make.begin(); it != not_to_make.end(); ++it) {
      options.push_back("-o");
      options.push_back((*it)->name);
   }

   string commands = exec("make", options);

   size_t pos = 0;
   for (vector<Target*>::const_iterator it = to_make.begin(); it != to_make.end(); ++it) {
      size_t delimiter_pos = commands.find(delimiter, pos);
      if (delimiter_pos == string::npos)
		 throw std::length_error("Lack of commands in a level\n");
      string command = commands.substr(pos, delimiter_pos - pos);
      pos = delimiter_pos + delimiter.length() + 1;
      (*it)->command = command;
   }
}

/* Adds commands to targets */ 
void count_commands(DependencyGraph* graph, const vector<string>& basics, const string& delimiter) {
   vector<vector<Target*> > levels = get_levels(graph);

   if (levels.empty()) return;
   count_one_level(basics, delimiter, *levels.begin(), vector<Target*>());
   for (vector<vector<Target*> >::iterator it = levels.begin() + 1;
         it != levels.end(); ++it)
      count_one_level(basics, delimiter, *it, *(it - 1));
}
   

