#include <sys/sendfile.h>
#include <sys/socket.h>
#include "HttpRequest.h"
#include "Exception.h"
#include "NetUtils.h"

void HttpRequest::parse() {
    std::istringstream request{raw_request};

    std::string line;
    std::getline(request, line);
    std::cout << line << std::endl;

    auto header_end = line.find(' ');
    auto uri_end = line.find(' ', header_end + 1);

    start_line = {
        line.substr(0, header_end),
        line.substr(header_end + 1, uri_end - 1 - header_end),
        line.substr(uri_end + 1, line.length() - uri_end - 2)
    };

    if (start_line.http_version != "HTTP/1.0" and start_line.http_version != "HTTP/1.1") {
        throw std::exception();
    }

    while (std::getline(request, line) && line != "\r") {
        auto name_end = line.find(':');

        auto name = line.substr(0, name_end);
        to_lower_case(name);
        auto value = line.substr(name_end + 2, line.length() - name_end - 3);

        headers.emplace(name, value);
    }
}

void HttpRequest::to_lower_case(std::string &str) {
    std::transform(str.begin(), str.end(), str.begin(), ::tolower);
}

std::string HttpRequest::decode_uri(std::string &uri) {
    std::string res;
    char rune = 0;

    for (auto i = 0ul; i < uri.length(); i++) {
        if (uri[i] != '%') {
            if (uri[i] == '+') {
                res += ' ';
            } else {
                res += uri[i];
            }
        } else {
            char *end;
            auto coded = strtoul(uri.substr(i + 1, 2).c_str(), &end, 16);
            rune = static_cast<char>(coded);
            res += rune;
            i = i + 2;
        }
    }

    return res;
}

void HttpRequest::read_data() {
    char buf;
    while (state != read_done) {
        auto ret = read(conn_fd, &buf, 1);
        if (ret < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                break;
            } else {
                perror("read");
                break;
            }
        } else if (ret == 0) {
            break;
        } else {
            raw_request.append(&buf, 1);
            auto len = raw_request.length();
            if (len >= 4 && raw_request.substr(len - 4, 4) == "\r\n\r\n") {
                state = read_done;
            }
        }
    }
}

RequestState HttpRequest::get_state() const {
    return state;
}

void HttpRequest::handle(const std::string &root) {
    int status;
    auto method = start_line.method;
    auto uri = start_line.uri;
    auto http_version = start_line.http_version;

    if (method == "GET" || method == "HEAD") {
        status = 200;

        bool is_subdir;
        try {
            is_subdir = make_path(root);
            if (!fs::exists(start_line.uri)) {
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
        write_start_line(http_version, status);

        if (status == 200) {
            left = fs::file_size(start_line.uri);
            write_headers(true);
            if (method == "GET") {
                file_fd = open(start_line.uri.c_str(), O_RDONLY);
                // TODO add check if file opened
                state = writing;
            } else {
                state = done;
            }
        } else {
            write_headers(false);
            state = done;
        }
    } else {
        status = 405;
        write_start_line(http_version, status);
        write_headers(false);
        status = done;
    }
}

void HttpRequest::write_start_line(std::string &version, int status) {
    auto start_str = version + " " + std::to_string(status) + " " + statuses.at(status) + "\r\n";

    if (rio_writen(conn_fd, start_str.c_str(), start_str.length()) < start_str.length()) {
        perror("roi_written headers");
    }
}

void HttpRequest::write_headers(bool is_ok) {
    std::string headers = "Server: Kotyarich Server C++\r\n"
                          "Connection: close\r\n"
                          "Date: " + get_rfc7231_time() + "\r\n";

    if (is_ok) {
        std::string content_type;
        try {
            extension = get_extension(start_line.uri);
            content_type = content_types.at(extension);
        }
        catch (std::exception &e) {
            content_type = "text/plain";
        }
        headers += "Content-Type: " + content_type + "\r\n"
            + "Content-Length: " + std::to_string(left) + "\r\n\r\n";
    }

    if (rio_writen(conn_fd, headers.c_str(), headers.length()) < headers.length()) {
        perror("roi_written headers");
    }
}

bool HttpRequest::make_path(const std::string &root) {
    auto path = start_line.uri;

    if (path.find("/..") != std::string::npos) {
        throw Exception("bad path .."); // TODO add custom exception
    }

    auto arguments_pos = path.find('?');
    if (arguments_pos != std::string::npos) {
        path = path.substr(0, arguments_pos);
    }

    auto decoded_path = decode_uri(path);

    auto subdir = false;
    if (decoded_path[decoded_path.length() - 1] != '/') {
        path = root + decoded_path;
    } else {
        subdir = true;
        path = root + decoded_path + "index.html";
    }
    std::cout << "path: " << path << std::endl;
    start_line.uri = path;

    return subdir;
}

void HttpRequest::write_file() {
    while (left > 0) {
        auto writen = sendfile(conn_fd, file_fd, nullptr, left);
        if (writen < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                break;
            } else {
                perror("sendfile");
                state = done;
                return;
            }
        } else if (writen == 0) {
            state = done;
            return;
        } else {
            left -= writen;
        }
    }

    if (left == 0) {
        state = done;
    }
}

int HttpRequest::get_conn_fd() const {
    return conn_fd;
}

HttpRequest::~HttpRequest() {
    if (file_fd != -1) {
        close(file_fd);
    }
    shutdown(conn_fd, SHUT_RDWR);
}

std::string HttpRequest::get_ext() const {
    return extension;
}

sockaddr_in HttpRequest::get_addr() const {
    return addr;
}
