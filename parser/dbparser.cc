#include <cstdio>
#include <cassert>
#include <cctype>

#include <fstream>
#include <iostream>
#include <list>
#include <string>
#include <sstream>
#include <stdexcept>
#include <unordered_map>
#include <utility>
#include <vector>
#include <queue>

#include <boost/foreach.hpp>
#include <boost/iostreams/device/file_descriptor.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/algorithm/string/trim.hpp>

#include "err.h"
#include "exec.h"
#include "dbparser.h"

using namespace std;
using namespace boost;
using namespace boost::iostreams;
using namespace boost::algorithm;


int Target::idcounter = 0;

void Target::AddDependency(Target* target) {
	dependencies_.push_back(target);
	target->dependent_targets_.push_back(this);
	++inord_;
}

Target::Target(const string& name) : kId_(Target::idcounter++), kName_(name), inord_(0), is_target_(0) {
}

Target::~Target() {
}

bool Target::EmptyRules() {
	return commands_.empty();
}

std::string Target::BuildBashScript(const std::string& working_dir) {
	stringstream ss;

	ss << "cd " << working_dir;
	BOOST_FOREACH(const string& c, commands_) {
		ss << " && " << c;
	}
	return ss.str();
}

void Target::MarkRealized(std::vector<Target*> &ready_targets) {
	BOOST_FOREACH(Target * i, dependent_targets_) {
		--(i->inord_);
		if (i->inord_==0)
			ready_targets.push_back(i);
	}
}

void Target::MarkSubtreeIfTarget(bool if_target) {
	if (is_target_ == if_target)
		return;
	is_target_ = if_target;
	BOOST_FOREACH(Target * i, dependencies_) {
		i->MarkSubtreeIfTarget(if_target);
	}
}

DependencyGraph::DependencyGraph(istream& is) {
	Init(is);
}

DependencyGraph::DependencyGraph(int fd) {
	file_descriptor_source fdsource(fd, close_handle);
	stream_buffer<file_descriptor_source> fdstream(fdsource);
	istream is(&fdstream);

	Init(is);
}

DependencyGraph::DependencyGraph(const string& str) {
	istringstream is(str);

	Init(is);
}

DependencyGraph::~DependencyGraph() {
	BOOST_FOREACH(Target*& t, all_targets_) {
		if (t)
			delete t;
	}
}

void DependencyGraph::ReinitInord() const {
	BOOST_FOREACH(Target* t, all_targets_) {
		if (t) {
			t->inord_ = t->dependencies_.size();
		}
	}
}

void DependencyGraph::TrimToTargets(vector<string> targets) {
	BOOST_FOREACH(Target * i, all_targets_) {
		BOOST_FOREACH(string s, targets) {
			if (s == i->kName_)
				i->MarkSubtreeIfTarget(true);
		}
	}
	BOOST_FOREACH(Target * i, all_targets_) {
		if (!i->is_target_)
			i->commands_.clear();
	}
}

static inline Target*& access(unordered_map<string, Target*>& map,
                              const string& name) {
	Target*& target = map[name];
	if (target == NULL)
		target = new Target(name);
	return target;
}

void DependencyGraph::Init(istream& is) {
	/* parser internal flags */
#define EMPTY_FLAG 1
#define ACQUISITION_FLAG 2

	/* some constants */
	static const string stop_acquisition = "# GNU Make";
	static const string start_acquisition = "# Files";
	static const string not_a_target = "# Not a target:";
	static const string not_proper_beginning = "-@(#%.\t\n";

	/* a map for tracking nodes */
	typedef pair<string, Target*> nodes_type;
	unordered_map<string, Target*> nodes;

	/* parse rules */
	string line = "";
	int skip_flags = 0;

	while (is) {
		getline(is, line);

		trim(line);
		int len = line.length();
		if (len) {
			if (!skip_flags) {

				if (not_proper_beginning.find(line[0]) == string::npos) {
					/* parse line */
					stringstream iss(line);

					string name;
					iss >> name;
					int name_len = name.length();

					/* this is not the best solution for parsing but much
					 * faster than boost regex (captures) */
					if (name[name_len-1] == ':') {
						name.resize(name_len-1);
						--name_len;

						Target* target = access(nodes, name);
						while (iss) {
							string depname;
							iss >> depname;

							trim(depname);
							if (!depname.empty())
								target->AddDependency(access(nodes, depname));
						}
					}
				} else if (line.compare(not_a_target) == 0) {
					/* skip 'Not a target' block */
					skip_flags |= 1 << EMPTY_FLAG;
				} else if (line.find(stop_acquisition) == 0) {
					/* stop acquisition */
					skip_flags |= 1 << ACQUISITION_FLAG;
				}
			} else if (line.find(start_acquisition) == 0) {
				/* start acquisition */
				skip_flags &= ~(1 << ACQUISITION_FLAG);
			}
		} else {
			/* new line after 'Not a target' block */
			skip_flags &= ~(1 << EMPTY_FLAG);
		}
	}

	BOOST_FOREACH(const nodes_type&  p, nodes) {
		all_targets_.push_back(p.second);
	}

	/* perform topological sorting */
	try {
		TopologicalSort();
	} catch (CircularDependency& e) {
		BOOST_FOREACH(const nodes_type&  p, nodes) {
			delete p.second;
		}
		throw e;
		/* return */
	}

}

void DependencyGraph::TopologicalSort() {
	ReinitInord();

	queue<Target*> lvqueue;
	BOOST_FOREACH(Target*& t, all_targets_) {
		assert(t != NULL);
		if (t->inord_ == 0) {
			lvqueue.push(t);
			leaf_targets_.push_back(t);
		}
	}

	while (!lvqueue.empty()) {
		Target* currt = lvqueue.front();
		lvqueue.pop();

		assert(currt->inord_ == 0);
		if (currt->dependent_targets_.empty()) {
			/* we're top-tier */
			main_targets_.push_back(currt);
		} else {
			BOOST_FOREACH(Target*& t, currt->dependent_targets_) {
				if (--t->inord_ == 0)
					lvqueue.push(t);
			}
		}
	}

	/* if something left undone, throw */
	BOOST_FOREACH(Target*& t, all_targets_) {
		assert(t != NULL);
		if (t->inord_ > 0)
			throw CircularDependency();
		else
			t->inord_ = t->dependencies_.size();
	}
}

void DependencyGraph::DumpMakefile(ostream& os) {
	ReinitInord();

	queue<Target*> lvqueue;
	BOOST_FOREACH(Target*& t, leaf_targets_) {
		lvqueue.push(t);
	}

	while (!lvqueue.empty()) {
		Target* currt = lvqueue.front();
		lvqueue.pop();

		os << currt->kName_ << ":";
		BOOST_FOREACH(Target*& t, currt->dependencies_)
		os << " " << t->kName_;
		os << '\n';
		BOOST_FOREACH(string& s, currt->commands_)
		os << '\t' << s << '\n';
		os << endl;

		BOOST_FOREACH(Target*& t, currt->dependent_targets_) {
			if (--t->inord_ == 0)
				lvqueue.push(t);
		}
	}

}

vector<vector<Target*> > DependencyGraph::GetLevels() {
	vector<vector<Target*> > levels;
	vector<Target*> level = leaf_targets_;

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

void DependencyGraph::CountOneLevel(const vector<string>& basics, const string& delimiter,
                                    const vector<Target*>& to_make_temp, const vector<Target*>& not_to_make) {
	vector<Target*> to_make;
	BOOST_FOREACH(Target* it, to_make_temp)
	if (it->kName_ != delimiter)
		to_make.push_back(it);

	vector<string> options = basics;

	BOOST_FOREACH(Target* it, to_make) {
		options.push_back(it->kName_);
		options.push_back(delimiter);
	}

	BOOST_FOREACH(Target* it, not_to_make) {
		options.push_back("-o");
		options.push_back(it->kName_);
	}

	std::pair<string, string> exec_result;
	try {
		exec_result = exec("make", options);
	} catch (SystemError& e) {
		remove("Makefile");
		rename("DMakefile", "Makefile");
		throw e;
	}
	string commands = exec_result.first;
	string commands_err = exec_result.second;

	if (commands_err !=  "") {
		remove("Makefile");
		rename("DMakefile", "Makefile");
		throw MakeError(commands_err);
	}

	size_t pos = 0;
	string delima = "make: `" + delimiter + "' is up to date.";
	string delimb = "echo " + delimiter;
	BOOST_FOREACH(Target* it, to_make) {
		size_t delima_pos = commands.find(delima, pos);
		size_t delimb_pos = commands.find(delimb, pos);

		size_t delim_pos = delima_pos;
		if (delima_pos != string::npos && delimb_pos != string::npos
		        && delimb_pos < delima_pos)
			delim_pos = delimb_pos;
		if (delima_pos == string::npos)
			delim_pos = delimb_pos;

		if (delim_pos == string::npos) {
			remove("Makefile");
			rename("DMakefile", "Makefile");
			throw MakeError("Lack of commands in a level\n");
		}

		string command = commands.substr(pos, delim_pos - pos);

		if (delim_pos == delima_pos)
			pos = delima_pos + delima.length() + 1;
		else
			pos = delimb_pos + delimb.length() + 1;

		if (command.substr(0, 6) == "make: ")
			command = "";

		string delim = "\n";
		size_t c_pos = 0;
		while ((delim_pos = command.find(delim, c_pos)) != string::npos) {
			string to_push_temp = command.substr(c_pos, delim_pos - c_pos);
			size_t begin = 0;
			size_t end = to_push_temp.size();
			while (end > 0 && isspace(to_push_temp[end - 1]))
				--end;
			while (begin < end && isspace(to_push_temp[begin]))
				++begin;
			string to_push = to_push_temp.substr(begin, end - begin);
			if (to_push != "") {
				if (to_push.substr(0, 6) != "make: ")
					it->commands_.push_back(to_push);
				c_pos = delim_pos + delim.length();
			} else c_pos++;
		}
	}
}


void DependencyGraph::CountCommands(const vector<string>& basics,
                                    const string& delimiter, vector<string> targets) {

	system("cp Makefile DMakefile");

	FILE *makefile;
	makefile = fopen("Makefile", "a");
	string code = "\n" + delimiter + ":\n\t@echo " + delimiter + "\n";
	fputs(code.c_str(), makefile);
	fclose(makefile);

	vector<string> n_basics = basics;
	n_basics.push_back("-n");

	vector<vector<Target*> > levels = GetLevels();
	if (levels.empty()) {
		remove("Makefile");
		rename("DMakefile", "Makefile");
		return;
	}

	CountOneLevel(n_basics, delimiter, *levels.begin(), vector<Target*>());
	for (vector<vector<Target*> >::iterator it = levels.begin() + 1;
	        it != levels.end(); ++it)
		CountOneLevel(n_basics, delimiter, *it, *(it - 1));

	ReinitInord();
	if (targets.empty())
		targets.push_back("all");
	TrimToTargets(targets);

	remove("Makefile");
	rename("DMakefile", "Makefile");
}


