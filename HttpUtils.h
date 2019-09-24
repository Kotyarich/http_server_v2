#ifndef SERVER_HTTPUTILS_H
#define SERVER_HTTPUTILS_H

#include <unordered_map>
#include <experimental/filesystem>
#include <string>
#include <ctime>

const std::unordered_map<int, std::string> statuses {
    std::pair<int, std::string>{200, "OK"},
    std::pair<int, std::string>{403, "Forbidden"},
    std::pair<int, std::string>{404, "Not Found"},
    std::pair<int, std::string>{405, "Method Not Allowed"},
};

const std::unordered_map<std::string, std::string> contnet_types {
    std::pair<std::string, std::string>{".html", "text/html"},
    std::pair<std::string, std::string>{".css", "text/css"},
    std::pair<std::string, std::string>{".js", "application/javascript"},
    std::pair<std::string, std::string>{".jpg", "image/jpeg"},
    std::pair<std::string, std::string>{".jpeg", "image/jpeg"},
    std::pair<std::string, std::string>{".png", "image/png"},
    std::pair<std::string, std::string>{".gif", "image/gif"},
    std::pair<std::string, std::string>{".swf", "application/x-shockwave-flash"},
};

std::string get_rfc7231_time();
std::string get_extension(std::string &p);

#endif //SERVER_HTTPUTILS_H
