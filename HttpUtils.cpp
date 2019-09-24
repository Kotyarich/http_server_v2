#include <iostream>
#include "HttpUtils.h"

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
