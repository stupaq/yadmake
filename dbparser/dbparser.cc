#include <cstdio>
#include <cassert>

#include <string>
#include <sstream>
#include <unordered_map>
#include <utility>
#include <queue>

#include <boost/foreach.hpp>
#include <boost/iostreams/device/file_descriptor.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/algorithm/string/trim.hpp>

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


DependencyGraph::DependencyGraph(istream& ins) {
	Init(ins);
}

DependencyGraph::DependencyGraph(int fd) {
	file_descriptor_source fdsource(fd, close_handle);
	stream_buffer<file_descriptor_source> fdstream(fdsource);
	istream ins(&fdstream);

	Init(ins);
}

DependencyGraph::~DependencyGraph() {
	BOOST_FOREACH(Target*& t, all_targets_) {
		if (t)
			delete t;
	}
}

void DependencyGraph::ReinitInord() {
	BOOST_FOREACH(Target*& t, all_targets_) {
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

void DependencyGraph::Init(istream& ins) {
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
	while (ins && line.find("# Files") != 0)
		getline(ins, line);

	/* parse rules */
	int skip_flags = 0;
	while (ins) {
		getline(ins, line);

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

void DependencyGraph::DumpMakefile() {
	queue<Target*> lvqueue;
	BOOST_FOREACH(Target*& t, leaf_targets_) {
		lvqueue.push(t);
	}

	while (!lvqueue.empty()) {
		Target* currt = lvqueue.front();
		lvqueue.pop();

		cerr << currt->kName_ << ":";
		BOOST_FOREACH(Target*& t, currt->dependencies_) {
			cerr << " " << t->kName_;
		}
		cerr << endl;

		BOOST_FOREACH(Target*& t, currt->dependent_targets_) {
			if (--t->inord_ == 0)
				lvqueue.push(t);
		}
	}

}

