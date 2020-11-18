#include <cstdlib>
#include <cstdio>
#include <unistd.h>
#include <algorithm>
#include <fstream>
#include <iostream>
#include <fcntl.h>
#include <signal.h>
#include <sys/sendfile.h>
#include <sys/epoll.h>
#include "Worker.h"


void Worker::run(int listener, int epollfd) {
    this->epoll_fd = epollfd;

    sockaddr_in client_addr{};
    socklen_t client_addr_len = sizeof(client_addr);

    while (true) {
        auto nfds = epoll_wait(epollfd, events, MAX_EVENTS, -1);
        if (nfds < 0) {
            perror("epoll_wait");
            continue;
        }

        for (auto i = 0; i < nfds; i++) {
            if (events[i].data.fd == listener) {
                accept_request(listener, client_addr, client_addr_len);
            } else {
                auto request = static_cast<HttpRequest *>(events[i].data.ptr);
                process_request(request);

                auto state = request->get_state();
                switch (state) {
                    case reading:
                        event.events = EPOLLIN | EPOLLET | EPOLLONESHOT;
                        event.data.ptr = request;
                        if (epoll_ctl(epollfd, EPOLL_CTL_MOD, request->get_conn_fd(), &event) < 0) {
                            perror("epoll_ctl");
                            continue;
                        }
                        break;
                    case writing:
                        event.events = EPOLLOUT | EPOLLET | EPOLLONESHOT;
                        event.data.ptr = request;
                        if (epoll_ctl(epollfd, EPOLL_CTL_MOD, request->get_conn_fd(), &event) < 0) {
                            perror("epoll_ctl");
                            continue;
                        }
                        break;
                    case done:
                        if (epoll_ctl(epollfd, EPOLL_CTL_DEL, request->get_conn_fd(), &event) < 0) {
                            perror("epoll_ctl");
                            continue;
                        }
                        delete request;
                    case read_done:
                        break;
                }
            }
        }
    }
}

Worker::Worker(std::string &docs_root, std::shared_ptr<ExtensionRepository> &user_rep)
    : _docs_root(docs_root), extension_repository(user_rep) {
    events = new epoll_event[MAX_EVENTS];
}

void Worker::accept_request(int listen_fd, sockaddr_in &client_addr, socklen_t &client_len) {
    while (true) {
        int conn_fd = accept(listen_fd, (sockaddr *) &client_addr, &client_len);
        if (conn_fd < 0) {
            if (errno == EAGAIN | errno == EWOULDBLOCK) {
                break;
            } else {
                perror("accept");
                break;
            }
        }

        set_non_blocking(conn_fd);

        event.events = EPOLLIN | EPOLLET | EPOLLONESHOT;
        auto request = new HttpRequest(conn_fd, client_addr);
        event.data.ptr = request;

        if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, conn_fd, &event) < 0) {
            perror("epoll_ctl");
            continue;
        }
    }
}

User Worker::get_user_info(const sockaddr_in &client_addr) {
    std::string ip(inet_ntoa(client_addr.sin_addr));
    return ip;
}


Worker::~Worker() {
    delete[] events;
}

void Worker::process_request(HttpRequest *request) {
    std::cout << request << std::endl;
    switch (request->get_state()) {
        case reading:
            request->read_data();
            if (request->get_state() == read_done) {
                request->parse();
                request->handle(_docs_root);
                
                auto user = get_user_info(request->get_addr());
                auto ext = request->get_ext();
                extension_repository->add_extension(user, ext);
            }
            break;
        case writing:
            request->write_file();
            break;
        case read_done:
            break;
        case done:
            break;
    }
}
