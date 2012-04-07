#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>
#include <string>
#include <vector>
#include <boost/iostreams/device/file_descriptor.hpp>
#include <boost/iostreams/stream.hpp>
#include "err.h"
#include "exec.h"

using namespace std;
using namespace boost;
using namespace boost::iostreams;

/* Creates arguments suitable for command line from a vector */
char** get_args(const vector<string>& v_args) {
	char** args = new char*[v_args.size() + 1];
	for (size_t i = 0; i < v_args.size(); ++i) {
		args[i] = new char[v_args[i].size() + 1];
		for (size_t j = 0; j < v_args[i].size(); ++j)
			args[i][j] = v_args[i][j];
		args[i][v_args[i].size()] = '\0';
	}
	args[v_args.size()] = (char*) 0;
	return args;
}

string exec(const string& programme, const vector<string>& arguments) {

	char** args = get_args(arguments);
	const char* prog = programme.c_str();
	string result = "";
	string line = "";

	int conn[2];
	if (pipe(conn) == -1)				syserr("pipe error");
	
	switch (fork()) {
		case -1:
			syserr("fork error");

		case 0:
			/* set conn[1] as stdout */
			if (close(conn[0]) == -1)	syserr("close error");
			if (close(1) == -1)			syserr("close error");
			if (dup(conn[1]) != 1)		syserr("dup error");
			/* execute the programme */
			execvp(prog, args);
			syserr("execvp error");

		default:
			if (close(conn[1]) == -1)	syserr("close error");

			/* read stdout of the programme line by line
			 * @and save it in result
			 */
			file_descriptor_source fdsource(conn[0], close_handle);
			stream_buffer<file_descriptor_source> fdstream(fdsource);
			istream ins(&fdstream);
			while (ins) {
				getline(ins, line);
				result += line;
				result += "\n";
			}

			return result;
	}
}

