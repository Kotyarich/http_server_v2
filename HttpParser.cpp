#include <sstream>
#include <unordered_map>
#include <algorithm>
#include "HttpParser.h"

HttpRequest HttpParser::parse_header(std::string &req) {
    std::istringstream request{req};

    std::string line;
    std::getline(request, line);

    auto header_end = line.find(' ');
    auto uri_end = line.find(' ', header_end + 1);

    HttpReuestHeader http_header = {
        line.substr(0, header_end),
        line.substr(header_end + 1, uri_end - 1 - header_end),
        line.substr(uri_end + 1, line.length() - uri_end - 2)
    };

    Headers headers;
    while (std::getline(request, line) && line != "\r") {
        auto name_end = line.find(':');

        auto name = line.substr(0, name_end);
        to_lower_case(name);
        auto value = line.substr(name_end + 2, line.length() - name_end - 3);

        headers.emplace(name, value);
    }

    return {http_header, headers};
}

void HttpParser::to_lower_case(std::string &str) {
    std::transform(str.begin(), str.end(), str.begin(), ::tolower);
}

std::string HttpParser::decode_uri(std::string &uri) {
    std::string res;
    char rune = 0;
    unsigned coded = 0;

    for (int i = 0; i < uri.length(); i++) {
        if (uri[i] != '%') {
            if (uri[i] == '+') {
                res += ' ';
            } else {
                res += uri[i];
            }
        } else {
            sscanf(uri.substr(i + 1, 2).c_str(), "%x", &coded);
            rune = static_cast<char>(coded);
            res += rune;
            i = i + 2;
        }
    }

    return res;
}
