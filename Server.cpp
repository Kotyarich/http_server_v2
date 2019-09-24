#include <errno.h>
#include <unistd.h>
#include <sys/wait.h>
#include <iostream>
#include <fstream>
#include "Server.h"

void Server::start() {
    struct sockaddr_in addr{};

    auto listener = socket(AF_INET, SOCK_STREAM, 0);
    if(listener < 0) {
        perror("socket");
        exit(1);
    }

    addr.sin_family = AF_INET;
    addr.sin_port = htons(_port);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    if(bind(listener, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("bind");
        exit(2);
    }

    listen(listener, 30);
    int pid = 0;
    for (int i = 0; i < _workers_number; i++) {
        pid = fork();
        if (pid == -1) {
            perror("fork");
            exit(3);
        }
        if (pid == 0) {
            auto worker = std::make_unique<Worker>(_documents_root);
            worker->run(listener);
        }
    }

    if (pid > 0) {
        wait(nullptr);
    }
}

void Server::read_config(std::string path) {
    std::ifstream f(path);
    if (!f.is_open()) {
        throw std::exception();
    }

    std::string tmp_s;
    int tmp_i;
    char buf[100];
    f >> tmp_s >> tmp_i;
    f.getline(buf, 100);
    _workers_number = tmp_i;

    f >> tmp_s >> tmp_i;
    f.getline(buf, 100);

    f >> tmp_s;
    f >> _documents_root;
}
