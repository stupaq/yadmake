#ifndef DBPARSER_DBPARSER_H_
#define DBPARSER_DBPARSER_H_

#include <string>
#include <istream>
#include <list>
#include <stdexcept>
#include <vector>
#include <unordered_map>


class Target;
class DependencyGraph;
class Computer;

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
		std::vector<Target*> main_targets_;
		/** Holds pointers to Target, which are not dependant of any other Target. */
		std::vector<Target*> leaf_targets_;

		/**
		 * Initiates inord of each Target with number of dependencies.
		 */
		void ReinitInord();

		/**
		 * If targets is not empty leaves in graph only targets
		 * and their subtrees.
		 */
		void TrimToTargets(std::vector<std::string> targets);
		
		/**
		 * Builds DependencyGraph from MakefileDB.
		 * @param is reference to std::istream used to read MakefileDB
		 * @throws CircularDependency
		 */
		DependencyGraph(std::istream& is);
		/**
		 * Builds DependencyGraph from MakefileDB.
		 * @param fd file descriptor used to read MakefileDB
		 * @throws CircularDependency
		 */
		DependencyGraph(int fd);
		/**
		 * Dumps DependencyGraph to given stream in Makefile format.
		 * @param os output stream
		 */
		void DumpMakefile(std::ostream& os);
		virtual ~DependencyGraph();
	protected:
		std::vector<Target*> all_targets_;
		void Init(std::istream& is);
		void TopologicalSort();
		void RemoveNotMarkedTargets();
		void DeleteBlah();
};

/** Holds single target together with receipes to build. */
class Target {
	friend class DependencyGraph;
	friend std::vector<std::vector<Target*> > get_levels(DependencyGraph* graph);
	friend void count_one_level(const std::vector<std::string>& basics,
			const std::string& delimiter, const std::vector<Target*>& to_make,
			const std::vector<Target*>& not_to_make);
	friend void dispatcher();
	friend int realize(Target * t, Computer * c);
	friend void mark_realized(Target * t, std::vector<Target*> & targets);
	public:
		/** Integer value unique to each Target. */
		const int kId_;
		/** Name of a Target as it occurs in MakefileDB. */
		const std::string kName_;

		/**
		 * Registers another target as a dependency of this one.
		 * @param target pointer to newly added dependency Target */
		void AddDependency(Target* target);

		/**
		 * Sets is_target_ to if_target recursively.
		 * @If is_target_ is already correct, returns.
		 */
		void MarkSubtreeIfTarget(bool if_target);

		/**
		 * Target constructor.
		 * @param name reference to constant string holding new Target name
		 */
		Target(const std::string& name);
		virtual ~Target();
	protected:
		std::list<Target*> dependent_targets_;
		std::list<Target*> dependencies_;
		int inord_;
		std::vector<std::string> commands_;
		bool is_target_;
		static int idcounter;
};

#endif  // DBPARSER_DBPARSER_H_
