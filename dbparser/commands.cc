#include "exec.h"
#include "dbparser.h"
#include <vector>
#include <list>
#include <string>
#include <stdexcept>
#include <boost/foreach.hpp>

using std::vector;
using std::list;
using std::string;

// TODO: usunac
#include <iostream>

/* Shares targets into levels. Leafs have level 0 and so on. */
vector<vector<Target*> > get_levels(DependencyGraph* graph) {
   vector<vector<Target*> > levels;
   vector<Target*> level = graph->leaf_targets_;

   while (!level.empty()) {
      levels.push_back(level);
      vector<Target*> next_level;
      
	  BOOST_FOREACH(Target* it, level)
		 BOOST_FOREACH(Target* parent, it->dependent_targets_) {
            --parent->inord_;
            if (parent->inord_ == 0)
               next_level.push_back(parent);
         }

      level = next_level;
   }

   return levels;
}

/* Adds commands to targets at one level (to_make).
   Targets at previous level should be in not_to_make.
   Delimiter is the additional target printing its kName_.
   Basics contains make, -n and options from command line.
   */
void count_one_level(const vector<string>& basics, const string& delimiter,
      const vector<Target*>& to_make, const vector<Target*>& not_to_make) {
   vector<string> options = basics;
   
   BOOST_FOREACH(Target* it, to_make) {
      options.push_back(it->kName_);
      options.push_back(delimiter);
   }

   BOOST_FOREACH(Target* it, not_to_make) {
      options.push_back("-o");
      options.push_back(it->kName_);
   }

   /* TODO: usunac */
   std::cout << "level options: ";
   for (vector<string>::iterator it = options.begin(); it != options.end(); ++it)
	   std::cout << *it << " ";
   std::cout << std::endl;
   /* dotad */

   string commands = exec("make", options);

   /*TODO: usunac */
   std::cout << "level commands: " << commands << std::endl;
   /* dotad */

   size_t pos = 0;
   string delima = "make: `blah' is up to date.";
   string delimb = "echo blah";
   BOOST_FOREACH(Target* it, to_make) {
      size_t delima_pos = commands.find(delima, pos);
      size_t delimb_pos = commands.find(delimb, pos);

	  size_t delim_pos = delima_pos;
	  if (delima_pos != string::npos && delimb_pos != string::npos
			  && delimb_pos < delima_pos)
		  delim_pos = delimb_pos;
	  if (delima_pos == string::npos)
		  delim_pos = delimb_pos;

      /*TODO: usunac */
	  std::cout << "target: " << it->kName_ << std::endl;
      /* dotad */

      if (delim_pos == string::npos)
		 throw std::length_error("Lack of commands in a level\n");

      string command = commands.substr(pos, delim_pos - pos);

   	  /*TODO: usunac */
	  std::cout << "command: " << command << std::endl;
      /* dotad */

	  if (delim_pos == delima_pos)
      	pos = delima_pos + delima.length() + 1;
	  else
      	pos = delimb_pos + delimb.length() + 1;
      it->command_ = command;
   }
}

/* Adds commands to targets */ 
void count_commands(DependencyGraph* graph, const vector<string>& basics, const string& delimiter) {
   graph->ReinitInord();
   vector<vector<Target*> > levels = get_levels(graph);

   vector<string> n_basics = basics;
   n_basics.push_back("-n");

   if (levels.empty()) return;
   count_one_level(n_basics, delimiter, *levels.begin(), vector<Target*>());
   for (vector<vector<Target*> >::iterator it = levels.begin() + 1;
         it != levels.end(); ++it)
      count_one_level(n_basics, delimiter, *it, *(it - 1));

   graph->ReinitInord();
}
   

