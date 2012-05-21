#include <sys/types.h>
#include <stdlib.h>
#include <vector>
#include <map>
#include <string>
#include <boost/foreach.hpp>
#include <stdio.h>
#include "libssh/libssh.h"
#include "remote_worker.h"
#include "err.h"
 
 
void RemoteWorker::close_connection() {
  ssh_disconnect(session);
  ssh_free(session);
}

<<<<<<< HEAD
void RemoteWorker::connect_to() {
  int rc;
  
  session = ssh_new();
   if (session == NULL)
     syserr("ssh_new");

  ssh_options_set(session, SSH_OPTIONS_HOST, name);
  ssh_options_set(session, SSH_OPTIONS_PORT, &port);
  ssh_options_set(session, SSH_OPTIONS_USER, username);

  
  rc = ssh_connect(session);
  if (rc != SSH_OK)
  {
    ssh_free(session);
    throw NoConnection();
  }

  rc = ssh_userauth_password(session, NULL, password);
  if (rc == SSH_AUTH_ERROR)
  {
     fprintf(stderr, "Authentication failed: %s\n",
       ssh_get_error(session));
  }

}

RemoteWorker::RemoteWorker(const char * n, char * p, char * u) {
    username = u;
    password = p;
    name = n;
    port = 22;
}

RemoteWorker::RemoteWorker(const char * n, char * p) {
    username = "md292600";
    password = p;
    name = n;
    port = 22;
}


int RemoteWorker::open_channel() {
  int rc;
  channel = ssh_channel_new(session);
  if (channel == NULL)
    return SSH_ERROR;

  rc = ssh_channel_open_session(channel);
  if (rc != SSH_OK) {
    ssh_channel_free(channel);
    return rc;
  }

  return 0;
}


int RemoteWorker::run_command(char * command) {
  int rc;
int nbytes;
  rc = open_channel();
  if (rc != 0)
    return rc;

  rc = ssh_channel_request_exec(channel, command);
  if (rc != SSH_OK)
  {
    ssh_channel_close(channel);
    ssh_channel_free(channel);
    return rc;
  }
  rc = read_results();

  if (rc != 0)
  {
    ssh_channel_close(channel);
    ssh_channel_free(channel);
    return rc;
  }
  ssh_channel_close(channel);
  ssh_channel_free(channel);

  return 0;
}


int RemoteWorker::read_results() {
  char buffer[256];
  unsigned int nbytes;

  nbytes = ssh_channel_read(channel, buffer, sizeof(buffer), 0);
  while (nbytes > 0)
  {
    if (fwrite(buffer, 1, nbytes, stdout) != nbytes)
    {
      ssh_channel_close(channel);
      ssh_channel_free(channel);
      return SSH_ERROR;
    }
    nbytes = ssh_channel_read(channel, buffer, sizeof(buffer), 0);
  }

  if (nbytes < 0)
  {
    ssh_channel_close(channel);
    ssh_channel_free(channel);
    return SSH_ERROR;
  }
  return 0;
}

 void RemoteWorker::realize(std::vector<std::string> commands, std::string path) {
   std::string temp;
   char* buffer = new char[5000];
   BOOST_FOREACH(std::string s, commands) {
     temp = "cd " + path + "; " + s;
     strcpy(buffer, temp.c_str());
//     run_command(buffer);
     if (run_command(buffer) != 0)
  syserr("run_command");
   }
 }
