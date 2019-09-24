#include <iostream>
#include <bits/unique_ptr.h>
#include "Server.h"

int main(int argc, char *argv[]) {
    std::unique_ptr<Server> server_p{new Server};
    int port = 80;
    std::string path("/etc/httpd.conf");
    for (int i = 1; i < argc; i += 2) {
        std::string arg(argv[i]);
        if (arg == "-p") {
            port = atoi(argv[i + 1]);
        } else {
            path = argv[i + 1];
        }
    }

    server_p->set_port(port);
    server_p->read_config(path);
    server_p->start();

    return 0;
}