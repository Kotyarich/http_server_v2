#ifndef SERVER_SERVER_H
#define SERVER_SERVER_H

#include "Exception.h"
#include "Worker.h"

class Server {
public:
    void start();
    void set_port(int port) { _port = port; }
    void read_config(std::string path);
private:
    std::string _documents_root;
    int _workers_number = 3;
    int _port;
};


#endif //SERVER_SERVER_H
