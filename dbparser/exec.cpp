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
	if (pipe(conn) == -1)				syserr("Error in pipe\n");
	
	int buf_size = 10;	// TODO: ile najlepiej?
	// Po tyle znakow czytamy stdout i dodajemy do wynikowego stringa.
	char buf[buf_size];
	int bytes = 0;
	string result = "";
	
	switch (fork()) {
		case -1:
			syserr("Error in fork\n");

		case 0:
			if (close(conn[0]) == -1)	syserr("Error in close\n");
			if (close(1) == -1)			syserr("Error in close\n");
			if (dup(conn[1]) != 1)		syserr("Error in dup\n");
			execvp(prog, args);
			syserr("Error in execvp\n");

		default:
			if (close(conn[1]) == -1)	syserr("Error in close\n");
			while ((bytes = read(conn[0], buf, buf_size)) > 1) {
				buf[bytes] = '\0';
				result += buf;
			}
			if (bytes == -1)			syserr("Error in read\n");
			if (close(conn[0]) == -1)	syserr("Error in close\n");
			return result;
	}
}

