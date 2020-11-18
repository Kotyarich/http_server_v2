#include <errno.h>
#include <unistd.h>
#include <sys/wait.h>
#include <iostream>
#include <fstream>
#include "Server.h"

void Server::start() {
    struct sockaddr_in addr{};

    auto listener = socket(AF_INET, SOCK_STREAM, 0);
    if (listener < 0) {
        perror("socket");
        exit(1);
    }

    addr.sin_family = AF_INET;
    addr.sin_port = htons(static_cast<uint16_t>(_port));
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(listener, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
        perror("bind");
        exit(2);
    }

    if(listen(listener, 30) < 0) {
        perror("listen");
        exit(3);
    }

    set_non_blocking(listener);

    int epollfd = epoll_create1(0);
    if (epollfd < 0) {
        perror("epoll_create1");
        exit(3);
    }

    struct epoll_event epoll_event{};
    epoll_event.events = EPOLLIN | EPOLLET;
    epoll_event.data.fd = listener;

    if (epoll_ctl(epollfd, EPOLL_CTL_ADD, listener, &epoll_event) < 0) {
        perror("epoll_ctl");
        exit(4);
    }

    std::vector<std::thread> work_threads(_workers_number);
    for (int i = 0; i < _workers_number; i++) {
        work_threads[i] = std::thread([&]() {
            auto worker = std::make_unique<Worker>(_documents_root, user_repository);
            worker->run(listener, epollfd);
        });
    }

    auto worker = std::make_unique<Worker>(_documents_root, user_repository);
    worker->run(listener, epollfd);

    for (auto &thr: work_threads) {
        if (thr.joinable()) {
            thr.join();
        }
    }
}

void Server::read_config(std::string path) {
    std::ifstream f(path);
    if (!f.is_open()) {
        throw std::exception();
    }

    std::string line;
    std::cout << "Config:" << std::endl;
    while (std::getline(f, line)) {
        std::cout << line << std::endl;

        std::istringstream line_stream{line};
        std::string arg_name, arg_val;
        line_stream >> arg_name >> arg_val;

        std::cout << arg_val << std::endl;
        if (arg_name == "document_root") {
            _documents_root = arg_val;
        } else if (arg_name == "cpu_limit") {
            char *end;
            _workers_number = strtoul(arg_val.c_str(), &end, 10);
        }
    }

    f.close();
}

Server::Server() {
    user_repository.reset(new PostgresExtensionRepository);
}
