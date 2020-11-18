#ifndef SERVER_WORKER_H
#define SERVER_WORKER_H

#include <string>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unordered_map>
#include "Exception.h"
#include "HttpRequest.h"
#include "NetUtils.h"
#include "db/ExtensionRepository.h"
#include "entities/User.h"

class Worker {
public:
    Worker(std::string &docs_root, std::shared_ptr<ExtensionRepository> &user_rep);
    void run(int listener, int epollfd);

    ~Worker();
private:
    static const size_t MAX_EVENTS = 128;
    std::string _docs_root;
    epoll_event event;
    epoll_event *events;
    int epoll_fd;
    std::shared_ptr<ExtensionRepository> extension_repository;

    void accept_request(int listen_fd, sockaddr_in &client_addr, socklen_t &client_len);
    User get_user_info(const sockaddr_in &client_addr);
    void process_request(HttpRequest *request);
};


#endif //SERVER_WORKER_H
