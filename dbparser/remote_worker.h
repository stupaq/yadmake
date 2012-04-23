#include <string>
#include <istream>
#include <stdlib.h>
#include <stdio.h> 
#include <stdexcept>
#include <libssh/libssh.h>

#ifndef _REMOTEWORKER_
#define _REMOTEWORKER_

class NoConnection : public std::runtime_error {
public:
NoConnection() : std::runtime_error("connection  failed") {}
};


class RemoteWorker{
  public:
    const char * name;
    
    RemoteWorker(const char * name, char * password, char * username);

    RemoteWorker(const char * name, char * password);

//run commands to create a target
    void realize(std::vector<std::string> commands, std::string path);

//creates ssh connection
    void connect_to();

    void close_connection();

  private:
    char * password;

    char * username;

    ssh_session session;
    int port;
    ssh_channel channel;
    
    int open_channel();
//run a single remote command    
    int run_command(char * command);
    
// read data displayed by remote command   
    int read_results();

  
};

#endif
