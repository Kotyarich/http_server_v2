#ifndef SERVER_SERVER_H
#define SERVER_SERVER_H

#include <fcntl.h>
#include <sys/epoll.h>
#include <thread>
#include <memory>
#include "Exception.h"
#include "Worker.h"
#include "db/ExtensionRepository.h"
#include "db/PostgresExtensionRepository.h"

class Server {
public:
    Server();

    void start();
    void set_port(int port) { _port = port; }
    void read_config(std::string path);
private:
    std::string _documents_root;
    size_t _workers_number = 3;
    int _port;

    std::shared_ptr<ExtensionRepository> user_repository;
};


#endif //SERVER_SERVER_H
