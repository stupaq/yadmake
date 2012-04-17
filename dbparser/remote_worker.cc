
#include <sys/types.h>
#include <stdlib.h>
#include <vector>
#include <map>
#include <string>
#include <boost/foreach.hpp>
#include <stdio.h>
#include <libssh/libssh.h>
#include "err.h"
#include "remote_worker.h"


void RemoteWorker::close_connection() {
  ssh_disconnect(my_ssh_session);
  ssh_free(my_ssh_session);
}

void RemoteWorker::connect_to() {
  int rc;
  
  my_ssh_session = ssh_new();
  if (my_ssh_session == NULL)
    syserr("ssh_new");

  ssh_options_set(my_ssh_session, SSH_OPTIONS_HOST, name);
  ssh_options_set(my_ssh_session, SSH_OPTIONS_PORT, &port);
  ssh_options_set(my_ssh_session, SSH_OPTIONS_USER, username);

  
  rc = ssh_connect(my_ssh_session);
  if (rc != SSH_OK)
  {
    ssh_free(my_ssh_session);
    throw NoConnection(name);
  }
  
  rc = ssh_userauth_password(my_ssh_session, NULL, password);
  if (rc != SSH_AUTH_SUCCESS)
  {
    syserr("Error authenticating with password");
    close_connection();
  }

}

RemoteWorker::RemoteWorker(std::string name) {
    this.name = name;
    port = 22;
}


int RemoteWorker::open_channel() {
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


int RemoteWorker::run_command(std::string command) {
  rc = ssh_channel_request_exec(channel, command);
  if (rc != SSH_OK)
  {
    ssh_channel_close(channel);
    ssh_channel_free(channel);
    return rc;
  }
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

void RemoteWorker::realize(std::vector<std::string> commands) {
  open_channel();
  
  BOOST_FOREACH(std::string s, commands_) {
    (void) run_command(s);
    (void) read_results();
  }
  
  ssh_channel_send_eof(channel);
  ssh_channel_close(channel);
  ssh_channel_free(channel);
  
}

