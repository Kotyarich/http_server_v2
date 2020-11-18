#include <utility>

#ifndef SERVER_EXCEPTION_H
#define SERVER_EXCEPTION_H

#include <string>
#include <exception>

class Exception: public std::exception {
public:
    explicit Exception(std::string str): _mess(std::move(str)){}
    const char* what() const noexcept override;
private:
    std::string _mess;
};


#endif //SERVER_EXCEPTION_H
