#ifndef SERVER_SERVER_H
#define SERVER_SERVER_H

#include <fcntl.h>
#include <sys/epoll.h>
#include <thread>
#include "Exception.h"
#include "Worker.h"

class Server {
public:
    void start();
    void set_port(int port) { _port = port; }
    void read_config(std::string path);
private:
    std::string _documents_root;
    size_t _workers_number = 3;
    int _port;
};


#endif //SERVER_SERVER_H
