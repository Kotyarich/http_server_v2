#ifndef SERVER_WORKER_H
#define SERVER_WORKER_H

#include <thread>
#include <mutex>
#include <queue>

#include <string>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unordered_map>
#include "Exception.h"
#include "HttpParser.h"
#include "HttpUtils.h"

namespace fs = std::experimental::filesystem;

class Worker {
public:
    explicit Worker(std::string &docs_root);
    void run(int listener);
private:
    std::string _docs_root;
    std::mutex _mut;

    void write_start_line(std::string &version, int status, int sock);
    void write_headers(int sock, bool is_ok, std::string &uri, int length=0);
    void write_file(int sock, std::string &uri);
    void complete_tusk(HttpRequest &request, int sock);
    bool make_path(std::string &path);
    void run_thread(int listener);

    std::string read_request(int sock);
};


#endif //SERVER_WORKER_H
