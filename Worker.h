#ifndef SERVER_WORKER_H
#define SERVER_WORKER_H

#include <string>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unordered_map>
#include "Exception.h"
#include "HttpParser.h"
#include "NetUtils.h"

class Worker {
public:
    explicit Worker(std::string &docs_root);
    void run(int listener, int epollfd);

    ~Worker();
private:
    static const size_t MAX_EVENTS = 128;
    std::string _docs_root;
    epoll_event event;
    epoll_event *events;
    int epoll_fd;

    void accept_request(int listen_fd, sockaddr_in &client_addr, socklen_t &client_len);
    void process_request(HttpRequest *request);
};


#endif //SERVER_WORKER_H
