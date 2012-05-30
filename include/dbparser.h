#ifndef _DBPARSER_H_
#define _DBPARSER_H_

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

/**
 * An exception thrown by count_commands if make -n
 * exits with error.
 */
class MakeError : public std::runtime_error {
	public:
		MakeError(const std::string& what): std::runtime_error(what) {}
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
		void ReinitInord() const;

		/**
		 * If targets is not empty leaves in graph only targets
		 * and their subtrees.
		 */
		void TrimToTargets(std::vector<std::string> targets);
		
		/**
		 * Adds commands to targets.
		 * @basic can contain for example 'make', 'aim', '-o', 'ready_aim'.
		 * @delimiter seems to be blah always
		 */
		void CountCommands( const std::vector<std::string>& basics,
				const std::string& delimiter);

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
	private:
		/* Shares targets into levels.
		 * Leafs have level 0 and so on. */
		std::vector<std::vector<Target*> > GetLevels();
		/* Adds commands to targets at one level (to_make).
		 * Targets at previous level should be in not_to_make.
		 * @Delimiter is the additional target printing its name.
		 * @Basics contains make, -n and options from command line.
		 */
		void CountOneLevel(const std::vector<std::string>& basics,
				const std::string& delimiter,
				const std::vector<Target*>& to_make,
				const std::vector<Target*>& not_to_make);
};

/** Holds single target together with receipes to build. */
class Target {
	friend class DependencyGraph;

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
		 * Constructs string containing bash script that executes commands for this target
		 * @param working_dir string containing path to desired working directory */
		std::string BuildBashScript(const std::string& working_dir);

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

    /**
     * Mark me as realized - 
     * decrease dependent targets inords
     * check whether dependent targets are ready to realize,
     * add them to ready_targets
     */
	  void MarkRealized(std::vector<Target*> &ready_targets);
	protected:
		std::list<Target*> dependent_targets_;
		std::list<Target*> dependencies_;
		std::vector<std::string> commands_;
		int inord_;
		bool is_target_;
		static int idcounter;
};

#endif  // _DBPARSER_H_
