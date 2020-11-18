#ifndef SERVER_HTTP_PARSER_H
#define SERVER_HTTP_PARSER_H

#include <sstream>
#include <unordered_map>
#include <algorithm>
#include <iostream>
#include <unistd.h>
#include <experimental/filesystem>
#include <string>
#include <unordered_map>
#include <netinet/in.h>

namespace fs = std::experimental::filesystem;
using Headers = std::unordered_map<std::string, std::string>;

struct HttpRequestHeader {
    std::string method;
    std::string uri;
    std::string http_version;
};

enum RequestState {
    reading,
    read_done,
    writing,
    done,
};

struct HttpRequest {
    void write_file();
public:
    HttpRequest(int conn_fd, sockaddr_in client_addr) {
        this->conn_fd = conn_fd;
        left = 0;
        state = reading;
        file_fd = -1;
        addr = client_addr;
    }

    RequestState get_state() const;
    int get_conn_fd() const;
    std::string get_ext() const;
    sockaddr_in get_addr() const;

    void read_data();
    void parse();
    void handle(const std::string &root);
    std::string decode_uri(std::string &uri);

    void log() {
        std::cout << start_line.uri << std::endl;
        std::cout << "state: " << state << std::endl;
        std::cout << raw_request << std:: endl;
        std::cout << left << std::endl;
    }

    ~HttpRequest();
private:
    HttpRequestHeader start_line;
    std::string raw_request;
    Headers headers;
    int conn_fd;
    size_t left;
    int file_fd;
    RequestState state;
    std::string extension;
    sockaddr_in addr;

    inline void to_lower_case(std::string &str);
    bool make_path(const std::string &root);
    void write_headers(bool is_ok);
    void write_start_line(std::string &version, int status);
};


#endif //SERVER_HTTP_PARSER_H
