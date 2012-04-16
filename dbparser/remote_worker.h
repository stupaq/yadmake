#include <string>
#include <istream>
#include <libssh/libssh.h>
#include <stdlib.h>
#include <stdio.h> 

#ifndef _REMOTEWORKER_
#define _REMOTEWORKER_

class NoConnection : public std::runtime_error {
public:
NoConnection(std::string name) : std::runtime_error("connection to ", name, " failed") {}
};


class RemoteWorker{
  public:
    std::string name;
    
    RemoteWorker(std::string name);

//realize commands to crate a target
    void realize(std::vector<std::string> commands)

//creates ssh connection
    void connect_to();

  private:
    const char * username;
    const char * password;
    ssh_session my_session;
    int port;
    ssh_channel channel;
    
    void close_connection();
    int open_channel();

//run a single remote command    
    int run_command(std::string command);
    
// read data displayed by remote command   
    int read_results();
  
};

#endif

