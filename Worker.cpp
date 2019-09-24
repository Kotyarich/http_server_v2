#include <cstdlib>
#include <cstdio>
#include <unistd.h>
#include <algorithm>
#include <fstream>
#include <iostream>
#include <fcntl.h>
#include <sys/sendfile.h>
#include "Worker.h"

void Worker::run(int listener) {
    int threads_num = 3;
    std::vector<std::thread> threads;
    for (int i = 0; i < threads_num; i++) {
        std::thread thread([this](int l) {
            this->run_thread(l);
        }, listener);
        threads.push_back(std::move(thread));
    }

    for (auto &thr: threads) {
        if (thr.joinable()) {
            thr.join();
        }
    }
}

void Worker::run_thread(int listener) {
    HttpParser parser;

    while(true) {
//        _mut.lock();
        auto sock = accept(listener, nullptr, nullptr);
//        _mut.unlock();
        if(sock < 0) {
            perror("accept");
            exit(3);
        }

        auto req = read_request(sock);
        auto http_req = parser.parse_header(req);

        try {
            complete_tusk(http_req, sock);
        } catch (std::exception &e) {
            std::cout << e.what() << std::endl;
        }

        close(sock);
    }
}

void Worker::write_start_line(std::string &version, int status, int sock) {
    auto start_str = version + " " + std::to_string(status) + " " + statuses.at(status) + "\r\n";
    send(sock, start_str.c_str(), start_str.length(), 0);
}

std::string Worker::read_request(int sock) {
    const int buf_size = 1024;
    char buf[buf_size]; // TODO move to some config
    std::string req;

    auto n = recv(sock, buf, buf_size, 0);
    if (n > 0) {
        req.append(buf, n);
    } else {
        if (n == -1) {
            throw Exception("read failed"); // TODO add custom exception
        }
    }

    return req;
}

void Worker::complete_tusk(HttpRequest &request, int socket) {
    int status;
    auto method = request.start_line.method;

    if (method == "GET" || method == "HEAD") {
        status = 200;

        bool is_subdir;
        try {
            is_subdir = make_path(request.start_line.uri);
            if (!fs::exists(request.start_line.uri)) {
                if (is_subdir) {
                    status = 403;
                } else {
                    status = 404;
                }
            }
        } catch (Exception &e) {
            status = 403;
            std::cout << "exc" << std::endl;
        }
        // TODO check if its directory
        write_start_line(request.start_line.http_version, status, socket);


        int size = 0;
        if (status == 200) {
            size = fs::file_size(request.start_line.uri);
        }

        if (status == 200) {
            write_headers(socket, true, request.start_line.uri, size);
            if (method == "GET") {
                write_file(socket, request.start_line.uri);
            }
        } else {
            write_headers(socket, false, request.start_line.uri);
        }
    } else {
        status = 405;
        write_start_line(request.start_line.http_version, status, socket);
        write_headers(socket, false, request.start_line.uri);
    }
}

void Worker::write_headers(int sock, bool is_ok, std::string &uri, int length) {
    std::string headers = "Server: Kotyarich Server C++\r\n"
                          "Connection: close\r\n"
                          "Date: " + get_rfc7231_time() + "\r\n";

    if (is_ok) {
        std::string content_type;
        try {
            auto ext = get_extension(uri);
            content_type = contnet_types.at(ext);
        } catch (std::exception &e) {
            content_type = "text/plain";
        }
        headers += "Content-Type: " + content_type + "\r\n"
            + "Content-Length: " + std::to_string(length) + "\r\n\r\n";
    }

    send(sock, headers.c_str(), headers.length(), 0);
}

bool Worker::make_path(std::string &path) {
    if (path.find("/..") != std::string::npos) {
        throw Exception("bad path .."); // TODO add custom exception
    }

    auto arguments_pos = path.find('?');
    if (arguments_pos != std::string::npos) {
        path = path.substr(0, arguments_pos);
    }
    HttpParser parser;
    auto decoded_path = parser.decode_uri(path);

    auto subdir = false;
    if (decoded_path[decoded_path.length() - 1] != '/') {
        path = _docs_root + decoded_path;
    } else {
        subdir = true;
        path = _docs_root + decoded_path + "index.html";
    }

    return subdir;
}

void Worker::write_file(int sock, std::string &uri) {
    auto size = fs::file_size(uri);
    auto fd = open(uri.c_str(), O_RDONLY);
    // TODO add check if file opened

    while (size != 0) {
        auto written = sendfile(sock, fd, nullptr, size);
        if (written != -1) {
            size -= written;
        } else {
            if (errno == EWOULDBLOCK || errno == EAGAIN) {
                // TODO add something useful here mb
            }
            close(fd);
            return;
        }
    }

    close(fd);
}

Worker::Worker(std::string &docs_root): _docs_root(docs_root) {}
