/**
 * @defgroup parser_dbparser Parser
 * Dependency graph structure and Makefile parser */

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
 * An exception thrown by CountCommands if make -n
 * exits with error or in case of parsing failure.
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
	 * Adds commands to targets.
	 * @basic can contain make options,
	 * for example ('aim', '-o', 'ready_aim').
	 * @delimiter is a name of target to add.
	 * @targets are targets expected to be built; if left empty
	 * target 'all' is expected (rules for other targets
	 * will be empty).
	 * Expects file 'Makefile' in working directory.
	 * Throws MakeError if make -n exits with error
	 * or in case of parsing failure.
	 * Throws SystemError in case of system error during
	 * executing make -n.
	 */
	void CountCommands( const std::vector<std::string>& basics,
	                    const std::string& delimiter,
	                    std::vector<std::string> targets =
	                        std::vector<std::string>());

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
	 * Builds DependencyGraph from MakefileDB.
	 * @param str reference to string with MakefileDB
	 * @throws CircularDependency
	 */
	DependencyGraph(const std::string& str);

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
	/**
	 * Shares targets into levels.
	 * Leafs have level 0 and so on. */
	std::vector<std::vector<Target*> > GetLevels();
	/**
	 * Leaves commands in graph only for targets
	 * and their subtrees.
	 */
	void TrimToTargets(std::vector<std::string> targets);
	/**
	 * Adds commands to targets at one level (to_make).
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
	 * Determines whether there's something to do within target.
	 * @return true if target should be rebuild */
	bool EmptyRules();

	/**
	 * Constructs string containing bash script that executes commands for this target
	 * @param working_dir string containing path to desired working directory */
	std::string BuildBashScript(const std::string& working_dir);

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
	/**
	 * Sets is_target_ to if_target recursively.
	 * @If is_target_ is already correct, returns.
	 */
	void MarkSubtreeIfTarget(bool if_target);
};

#endif  // _DBPARSER_H_
