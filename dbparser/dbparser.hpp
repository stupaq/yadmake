#ifndef _DBPARSER_
#define _DBPARSER_

#include <string>
#include <istream>
#include <list>
#include <stdexcept>
#include <vector>
#include <unordered_map>


class Target;
class DependencyGraph;

/**
 * An exception thrown by DependencyGraph if compilation order
 * is not a complete partial order on set of targets.
 */
class CircularDependency : public std::runtime_error {
	public:
		CircularDependency() : std::runtime_error("cycle in dependencies") {}
};

/** Holds DAG representing all targets and dependencies between them. */
class DependencyGraph {
	public:
		/** Holds pointers to Target, which are not dependency of any other Target. */
		std::vector<Target*> main_targets;
		/** Holds pointers to Target, which are not dependant of any other Target. */
		std::vector<Target*> leaf_targets;

		/**
		 * Builds DependencyGraph from MakefileDB.
		 * @param ins reference to std::istream used to read MakefileDB
		 * @throws CircularDependency
		 */
		DependencyGraph(std::istream& ins);
		/**
		 * Builds DependencyGraph from MakefileDB.
		 * @param fd file descriptor used to read MakefileDB
		 * @throws CircularDependency
		 */
		DependencyGraph(int fd);
		virtual ~DependencyGraph();
	protected:
		void init(std::istream& ins);
		void topological_sort(const std::unordered_map<std::string, Target*>& targets);
};

/** Holds single target together with receipes to build. */
class Target {
	friend class DependencyGraph;
	public:
		/** Integer value unique to each Target. */
		const int id;
		/** Name of a Target as it occurs in MakefileDB. */
		const std::string name;

		/**
		 * Registers another target as a dependency of this one.
		 * @param target pointer to newly added dependency Target */
		void addDependency(Target* target);

		/**
		 * Target constructor.
		 * @param _name reference to constant string holding new Target name
		 */
		Target(const std::string& _name);
		virtual ~Target();
	protected:
		std::list<Target*> dependent_targets;
		std::list<Target*> dependencies;
		int inord;
		static int idcounter;
};

#endif /* _DBPARSER_ */
