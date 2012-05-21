#include <string>
#include <iostream>

#include <libsshpp.hpp>

using namespace std;
using namespace ssh;

/**
 * Detached worker. SSH based.
 */
void detachedWorkerSSH(const string& hostname) {
	char buffer[8092];
	int r;

	char command[] = "cd Work && ls\n";
	char exit[] = "exit\n";

	Session session;

	session.setOption(SSH_OPTIONS_HOST, hostname.c_str());
	session.optionsParseConfig(NULL);

	session.connect();
	session.userauthAutopubkey();

	{
		Channel channel(session);
		channel.openSession();
		channel.requestShell();

		r = channel.read(buffer, sizeof(buffer));

		channel.write(command, sizeof(command));

		usleep(500000);

		channel.write(exit, sizeof(exit));

		do {
			r = channel.readNonblocking(buffer, sizeof(buffer));

			if (r > 0) {
				cout << buffer << endl;
			}
		} while(channel.isOpen() && ! channel.isEof());

		channel.sendEof();
		channel.close();
	}

	session.disconnect();
}

int main() {
	try {
		detachedWorkerSSH("students");
	} catch (SshException e) {
		cerr << e.getError() << endl;
	}

	return 0;
}
