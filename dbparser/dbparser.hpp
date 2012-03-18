#ifndef _DBPARSER_
#define _DBPARSER_

#include <string>
#include <istream>
#include <list>
#include <stdexcept>
#include <unordered_map>


class Target;
class DependencyGraph;


class CircularDependency : public std::runtime_error {
	public:
		CircularDependency() : std::runtime_error("cycle in dependencies") {}
};

class DependencyGraph {
	public:
		std::vector<Target*> main_targets;
		std::vector<Target*> leaf_targets;

		DependencyGraph(std::istream& ins);
		DependencyGraph(int fd);
		virtual ~DependencyGraph();
	protected:
		void init(std::istream& ins);
		void topological_sort(const std::unordered_map<std::string, Target*>& targets);
			/* throws CircularDependency */
};

class Target {
	friend class DependencyGraph;
   friend std::vector<std::vector<Target*> > get_levels(DependencyGraph* graph);
   friend void count_one_level(const std::vector<std::string>& basics, const std::string& delimiter,
      const std::vector<Target*>& to_make, const std::vector<Target*>& not_to_make);
	public:
		const int id;
		const std::string name;

		void addDependency(Target* target);
      std::string getCommand();

		Target(const std::string& _name);
		virtual ~Target();
	protected:
		std::list<Target*> dependent_targets;
		std::list<Target*> dependencies;
		int inord;
      int outord;
      std::string command;
		static int idcounter;
};

int Target::idcounter = 0;

#endif /* _DBPARSER_ */
