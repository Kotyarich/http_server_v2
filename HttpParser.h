#ifndef SERVER_HTTPPARSER_H
#define SERVER_HTTPPARSER_H


#include <string>
#include <unordered_map>

typedef std::unordered_map<std::string, std::string> Headers;

struct HttpReuestHeader {
    std::string method;
    std::string uri;
    std::string http_version;
};

struct HttpRequest {
    HttpReuestHeader start_line;
    Headers headers;
};

class HttpParser {
public:
    HttpRequest parse_header(std::string &req);
    std::string decode_uri(std::string &uri);
private:
    inline void to_lower_case(std::string &str);
};


#endif //SERVER_HTTPPARSER_H
