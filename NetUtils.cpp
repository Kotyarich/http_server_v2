#include <iostream>
#include <unistd.h>
#include "NetUtils.h"

std::string get_rfc7231_time() {
    char buf[80];

    auto now = time(nullptr);
    auto time_info = gmtime(&now);

    strftime(buf, 80, "%a, %d %b %Y %T GMT", time_info);

    return buf;
}

std::string get_extension(std::string &p) {
    if (p[p.length() - 1] == '/') {
        return ".html";
    }
    std::experimental::filesystem::path file_path(p);
    return file_path.extension().string();
}

void set_non_blocking(int fd) {
    int old_option = fcntl(fd, F_GETFL);
    int new_option = old_option | O_NONBLOCK;

    fcntl(fd, F_SETFL, new_option);
}

size_t rio_writen(int fd, const char *user_buf, size_t n) {
    size_t left = n;
    ssize_t written;
    const char *buf = user_buf;

    while (left > 0) {
        if ((written = write(fd, buf, left)) <= 0) {
            return 0;
        }
        left -= written;
        buf += written;
    }

    return n;
}
