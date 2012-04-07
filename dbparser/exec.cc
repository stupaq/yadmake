#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>
#include <string>
#include <vector>
#include "err.h"
#include "exec.h"

using std::string;
using std::vector;

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

	int conn[2];
	if (pipe(conn) == -1)				syserr("pipe error");
	
	int buf_size = 10;	// TODO: ile najlepiej?
	// Po tyle znakow czytamy stdout i dodajemy do wynikowego stringa.
	char buf[buf_size + 1];
	int bytes = 0;
	string result = "";
	
	
	switch (fork()) {
		case -1:
			syserr("fork error");

		case 0:
			if (close(conn[0]) == -1)	syserr("close error");
			if (close(1) == -1)			syserr("close error");
			if (dup(conn[1]) != 1)		syserr("dup error");
			execvp(prog, args);
			syserr("execvp error");

		default:
			if (close(conn[1]) == -1)	syserr("close error");
			while ((bytes = read(conn[0], buf, buf_size)) > 1) {
				buf[bytes] = '\0';
				result += buf;
			}
			if (bytes == -1)			syserr("read error");
			if (close(conn[0]) == -1)	syserr("close error");
			return result;
	}
}

