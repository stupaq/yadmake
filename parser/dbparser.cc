#include <cstdio>
#include <cassert>

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

Target::Target(const string& name) : kId_(Target::idcounter++), kName_(name), inord_(0) {
}

Target::~Target() {
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
	BOOST_FOREACH(Target * i, dependent_targets_){
		--(i->inord_);
		if (i->inord_==0)
			ready_targets.push_back(i);
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

	/* some constants */
	static const string not_a_target = "# Not a target:";
	static const string not_proper_beginning = "#%.\t\n";

	/* a map for tracking nodes */
	typedef pair<string, Target*> nodes_type;
	unordered_map<string, Target*> nodes;

	string line = "";
	/* skip until files tag */
	while (is && line.find("# Files") != 0)
		getline(is, line);

	/* parse rules */
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
				} 
			}
		} else {
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
		if (it->kName_ != "blah")
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

	std::pair<string, string> exec_result = exec("make", options);
	string commands = exec_result.first;
	string commands_err = exec_result.second;

	if (commands_err !=  "")
		throw MakeError(commands_err);

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

		if (delim_pos == string::npos)
			throw std::length_error("Lack of commands in a level\n");

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
			string to_push = command.substr(c_pos, delim_pos + delim.length() - c_pos);
			if (to_push != "") {
				it->commands_.push_back(to_push);
				c_pos = delim_pos + delim.length();
			}
			else c_pos++;
		}

		/* check */
		std::cout << "Target: " << it->kName_ << std::endl;
		BOOST_FOREACH(string str, it->commands_)
			std::cout << str;
		std::cout << std::endl;
		/* end of check */
	}
}


void DependencyGraph::CountCommands(const vector<string>& basics,
		const string& delimiter) {

	vector<string> n_basics = basics;
	n_basics.push_back("-n");

	vector<vector<Target*> > levels = GetLevels();
	if (levels.empty()) return;

	CountOneLevel(n_basics, delimiter, *levels.begin(), vector<Target*>());
	for (vector<vector<Target*> >::iterator it = levels.begin() + 1;
			it != levels.end(); ++it)
		CountOneLevel(n_basics, delimiter, *it, *(it - 1));

	ReinitInord();
}


