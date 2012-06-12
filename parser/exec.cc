#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>

#include <string>
#include <vector>
#include <utility>

#include <boost/iostreams/device/file_descriptor.hpp>
#include <boost/iostreams/stream.hpp>

#include "err.h"
#include "exec.h"

using namespace std;
using namespace boost;
using namespace boost::iostreams;

/* Creates arguments suitable for command line from a vector */
char** get_args(const string& programme, const vector<string>& argv) {

	vector<string> v_args;
	v_args.push_back(programme);
	v_args.resize(argv.size() + 1);
	copy(argv.begin(), argv.end(), v_args.begin() + 1);

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

std::pair<string, string> exec(const string& programme, const vector<string>& arguments) {

	char** args = get_args(programme, arguments);
	const char* prog = programme.c_str();
	string result = "";
	string result_err = "";
	string line = "";
	pid_t child;
	int status;

	int conn[2];
	int conn_err[2];
	if (pipe(conn) == -1)			syserr("pipe error");
	if (pipe(conn_err) == -1)		syserr("pipe error");

	switch (child = fork()) {
		case -1:
			syserr("fork error");

		case 0:
			/* set conn[1] as stdout and conn_err[1] as stderr */
			if (close(conn[0]) == -1)		syserr("close error");
			if (close(conn_err[0]) == -1)	syserr("close error");
			if (close(1) == -1)				syserr("close error");
			if (close(2) == -1)				syserr("close error");
			if (dup(conn[1]) != 1)			syserr("dup error");
			if (dup(conn_err[1]) != 2)		syserr("dup error");
			/* execute the programme */
			execvp(prog, args);
			syserr("execvp error");

		default:
			if (close(conn[1]) == -1)		syserr("close error");
			if (close(conn_err[1]) == -1)	syserr("close error");

			/* read stdout of the programme line by line
			 * @and save it in result
			 */
			file_descriptor_source fdsource(conn[0], close_handle);
			stream_buffer<file_descriptor_source> fdstream(fdsource);
			istream ins(&fdstream);
			while (getline(ins, line)) {
				result += line;
				result += "\n";
			}

			/* the same with stderr */
			file_descriptor_source fdsource_err(conn_err[0], close_handle);
			stream_buffer<file_descriptor_source> fdstream_err(fdsource_err);
			istream ins_err(&fdstream_err);
			while (getline(ins_err, line)) {
				result_err += line;
				result_err += "\n";
			}

			/* wait for child process */
			if (waitpid(child, &status, 0) == -1)
				syserr("wait error in exec.cc");

			return std::make_pair(result, result_err);
	}
}

